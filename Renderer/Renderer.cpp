#include "Renderer.h"

Renderer::Renderer(Window* win)
	: m_DrawEraseShader("Shader/shaders/shader.comp"),
	m_VFShader("Shader/shaders/shader.vert", "Shader/shaders/shader.frag"),
	m_CanvasShader("Shader/shaders/canvas.comp"),
	m_SobelShader("Shader/shaders/sobelShader.comp"),
	m_BlurShader("Shader/shaders/blurShader.comp"),
	m_RACShader("Shader/shaders/container.vert", "Shader/shaders/container.frag"),
	m_CurrentEditable(win->GetCurrentEditable()),
	m_CurrentShader(NULL),
	m_Window(win)
{
	m_CanvasShader.CreateProgram();
	m_VFShader.CreateProgram();
	m_DrawEraseShader.CreateProgram();
	m_SobelShader.CreateProgram();
	m_BlurShader.CreateProgram();

	m_RenderingQuad = m_CurrentEditable->GetRenderingQuad();

	glGenBuffers(1, &m_ScreenQuadBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_ScreenQuadBuffer);
	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), m_RenderingQuad, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	float cursorPos[2] = {
		win->GetCursor().x,
		win->GetCursor().y
	};
	glGenBuffers(1, &m_CursorBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_CursorBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(cursorPos), cursorPos, GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, CURSOR_BUFFER_LOCATION, m_CursorBuffer);

	if (m_CurrentEditable != NULL)
		m_CurrentEditable->BindImage();

	render_area ra = m_CurrentEditable->GetRenderArea();
	float* rq = m_CurrentEditable->GetRenderingQuad();

	float containerVerts[16] = {
			rq[0], rq[1],
			rq[4], rq[5],
			rq[4], rq[5],
			rq[8], rq[9],
			rq[8], rq[9],
			rq[20], rq[21],
			rq[20], rq[21],
			rq[0], rq[1],
	};

	// create a buffer with the render area vertices as data
	// enable the attribute at location 0, already enabled (the same attribute location is used for the screen quad)
	// do the vertex specification for the attrbute
	glGenBuffers(1, &m_RACBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_RACBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(containerVerts), containerVerts, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	Shader::Use(m_CanvasShader.GetID());
	m_CanvasShader.SetDimensions(ra.width, ra.height);

	// execute the current Editable's canvas shader dispatching the correct number of work groups based on its dimensions
	m_CanvasShader.Execute();
}

void Renderer::SelectEditable()
{
	// send the current Editable's data to the shaders and then execute them
	Editable* currentEditable = m_Window->GetCurrentEditable();

	// if the current editable has changed bind the new image texture to the shader units
	if (currentEditable != m_CurrentEditable)
	{
		m_CurrentEditable = currentEditable;

		if (m_CurrentEditable != NULL)
		{
			m_CurrentEditable->BindImage();
		}
	}
}

void Renderer::SelectShader()
{
	uint8_t mouseState = m_Window->GetBrush()->GetMouseState();
	uint8_t brushState = m_Window->GetBrush()->GetDrawState();

	// otherwise the current shader doesn't get to be NULL if the same draw state goes inactive then active again
	//if (brushState == STATE_INACTIVE)
	//	m_CurrentShader = NULL;

	if (mouseState != STATE_DRAG || m_CurrentEditable == NULL || brushState == STATE_INACTIVE)
	{
		m_CurrentShader = NULL;
		return;
	}

	cursor curs = m_Window->GetCursor();

	unsigned int radius = m_Window->GetBrush()->GetRadius();

	render_area currentRA = m_CurrentEditable->GetRenderArea();
	// get the current editable WindowToRenderArea coordinates
	float cursorPos[2] = {
		currentRA.cursor_x,
		currentRA.cursor_y
	};

	// send to update inputs the cursor position relative to the current editable's reneder area
	// send to the shader that data in the UpdateInputs method
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_CursorBuffer);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(cursorPos), cursorPos);

	switch (brushState)
	{
	case STATE_DRAW: {
		Shader::Use(m_DrawEraseShader.GetID());
		m_DrawEraseShader.UpdateInputs(radius, cursorPos, brushState);
		m_CurrentShader = &m_DrawEraseShader;
	} break;
	case STATE_ERASE: {
		Shader::Use(m_DrawEraseShader.GetID());
		m_DrawEraseShader.UpdateInputs(radius, cursorPos, brushState);
		m_CurrentShader = &m_DrawEraseShader;
	} break;
	case STATE_SOBEL: {
		Shader::Use(m_SobelShader.GetID());
		m_SobelShader.UpdateInputs(radius, currentRA.width, currentRA.height);
		// update only at the frame when the brush state changes to STATE_SOBEL
		if(m_CurrentShader != &m_SobelShader)
			m_SobelShader.UpdateSobelCanvas();
		m_CurrentShader = &m_SobelShader;
	} break;
	case STATE_BLUR:
	{
		Shader::Use(m_BlurShader.GetID());
		m_BlurShader.UpdateInputs(radius, currentRA.width, currentRA.height);
		if (m_CurrentShader != &m_BlurShader)
			m_BlurShader.UpdateBlurCanvas();
		m_CurrentShader = &m_BlurShader;
	} break;
	//case STATE_INACTIVE:
	//	m_CurrentShader = NULL;
	//	break;
	}
}

void Renderer::Draw()
{
	SelectEditable();
	SelectShader();

	if (m_CurrentShader != NULL)
		m_CurrentShader->Execute();

	Shader::Use(m_VFShader.GetID());

	for (Editable* ed : m_Window->GetEditables())
	{
		// bind the m_CurrentEditable's rendering quad buffer, send data to OpenGL and draw the quad with the texture applied to it
		m_RenderingQuad = ed->GetRenderingQuad();
		glBindBuffer(GL_ARRAY_BUFFER, m_ScreenQuadBuffer);
		glBufferSubData(GL_ARRAY_BUFFER, 0, 24 * sizeof(float), m_RenderingQuad);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindTexture(GL_TEXTURE_2D, ed->GetRenderTexture());
		glUniform1i(glGetUniformLocation(m_VFShader.GetID(), "tex"), 0);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// use RenderAreaContainer shader
		// draw 4 pairs of vertices as the container (dotted 1D texture for the look?)
		
		// do the vertex specification for attribute index = 0 according to the render area container data
		Shader::Use(m_RACShader.GetID());

		glBindBuffer(GL_ARRAY_BUFFER, m_RACBuffer);
		glBufferSubData(GL_ARRAY_BUFFER, 0, 16 * sizeof(float), ed->GetRenderingContainer());
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0); // without redefining the vertex specification the attribute at index = 0 will get the data from the previousspecification which is the one done with the m_ScreenQuadBuffer buffer

		glDrawArrays(GL_LINES, 0, 8);

		// return to the attribute specification for the render area
		glBindBuffer(GL_ARRAY_BUFFER, m_ScreenQuadBuffer);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	}
}

Renderer::~Renderer()
{
	//glDeleteTextures(1, &m_RenderTexture);
	//glDeleteTextures(1, &m_Canvas);
	glDeleteBuffers(1, &m_ScreenQuadBuffer);
	glDeleteBuffers(1, &m_CursorBuffer);
}