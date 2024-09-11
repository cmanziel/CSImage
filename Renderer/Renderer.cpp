#include "Renderer.h"

//float quad[] = {
//	// 2 floats for postions coordinates, 2 for texture coordinates
//	-1.0f, 1.0f, 0.0f, 1.0f,
//	-1.0f, -1.0f, 0.0f, 0.0f,
//	1.0f, -1.0f, 1.0f, 0.0f,
//	-1.0f, 1.0f, 0.0f, 1.0f,
//	1.0f, -1.0f, 1.0f, 0.0f,
//	1.0f, 1.0f, 1.0f, 1.0f
//};

void setRenderingAreaQuad(float* quad, render_area area);

Renderer::Renderer(Window* win)
	: m_DrawEraseShader("Shader/shaders/shader.comp"),
	m_VFShader("Shader/shaders/shader.vert", "Shader/shaders/shader.frag"),
	m_CanvasShader("Shader/shaders/canvas.comp"),
	m_SobelShader("Shader/shaders/sobelShader.comp", win->GetWidth(), win->GetHeight()),
	m_CurrentShader(NULL),
	m_Window(win)
{
	int imageWidth = win->GetImageWidth();
	int imageHeight = win->GetImageHeight();

	m_CanvasShader.CreateProgram();
	m_VFShader.CreateProgram();
	m_DrawEraseShader.CreateProgram();
	m_SobelShader.CreateProgram();

	// set up the texture to render to
	// assure the texture is complete (see https://www.khronos.org/opengl/wiki/Texture#Texture_completeness)
	glGenTextures(1, &m_RenderTexture);
	glBindTexture(GL_TEXTURE_2D, m_RenderTexture);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8UI, 256, 256, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, NULL);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, imageWidth, imageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	// mipmap completeness, integer color format do not support linear filtering, use GL_NEAREST
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, 0);
	// access paramter has to be GL_READ_WRITE if a blit operation is called on the framebuffer to whcih the image is attached?
	glBindImageTexture(0, m_RenderTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	if (m_Window->GetImageData() != NULL)
	{
		canvas_data cd = m_Window->GetCanvasTextureData();

		// glTexImage2D arguments based on image format
		glGenTextures(1, &m_Canvas);
		glBindTexture(GL_TEXTURE_2D, m_Canvas);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8UI, imageWidth, imageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_Window->GetImageData());
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, imageWidth, imageHeight, 0, cd.pixel_format, cd.pixel_type, m_Window->GetImageData());

		// for texture completeness
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glBindTexture(GL_TEXTURE_2D, 0);
		glBindImageTexture(1, m_Canvas, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

		Shader::Use(m_CanvasShader.GetID());
		m_CanvasShader.SetDimensions(imageWidth, imageHeight);
		m_CanvasShader.Execute();

		// the whole image number of pixels is dispatch initially so that the m_RenderTexture already has the canvas data wrote to itself
		// in this way only the brush area of pixel is dispatched when drawing
		// otherwise without doing this the whole image would be dispatched and each uv checked if it was inside the brush area
		// just dispatching the brush without initializing the canvas would result in a black canvas and the brush drawing the canvas when in the ERASE state
	}

	// screen quad VBO
	setRenderingAreaQuad(m_RenderingQuad, win->GetRenderArea());
	
	glGenBuffers(1, &m_ScreenQuadBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_ScreenQuadBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(m_RenderingQuad), m_RenderingQuad, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
}

void Renderer::SelectShader()
{
	uint8_t brushState = m_Window->GetBrush()->GetState();
	int windowState = m_Window->GetState();

	cursor curs = m_Window->GetCursor();
	render_area rendArea = m_Window->GetRenderArea();

	unsigned int radius = m_Window->GetBrush()->GetRadius();

	// the cursor pos must be related to the render area coordinates
	float cursorPos[2] = { curs.x - rendArea.x, curs.y - (rendArea.y - rendArea.height)};

	//printf("x: %f\ty: %f\n", cursorPos[0], cursorPos[1]);

	if (windowState != STATE_CURSOR_INSIDE)
	{
		m_CurrentShader = NULL;
		return;
	}

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
		m_SobelShader.UpdateInputs(radius, cursorPos);
		// update only at the frame when the brush state changes to STATE_SOBEL
		if(m_CurrentShader != &m_SobelShader)
			m_SobelShader.UpdateSobelCanvas();
		m_CurrentShader = &m_SobelShader;
	} break;
	case STATE_INACTIVE:
		m_CurrentShader = NULL;
		break;
	}
}

void Renderer::Draw()
{
	SelectShader();

	if (m_CurrentShader != NULL)
		m_CurrentShader->Execute();

	Shader::Use(m_VFShader.GetID());

	// bind the render texture that will be applied to the quad
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_RenderTexture);

	// give to the sampler the same value of the currently active texture image unit: 0
	glUniform1i(glGetUniformLocation(m_VFShader.GetID(), "tex"), 0);

	// draw the screen quad onto which the texture is applied
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

Renderer::~Renderer()
{
	glDeleteTextures(1, &m_RenderTexture);
	glDeleteTextures(1, &m_Canvas);
	glDeleteBuffers(1, &m_ScreenQuadBuffer);
}

void setRenderingAreaQuad(float* quad, render_area area)
{
	// vertex
	quad[0] = area.ndc_x;
	quad[1] = area.ndc_y;
	//tex coord
	quad[2] = 0.0;
	quad[3] = 0.0;
	//vertex
	quad[4] = area.ndc_x + area.ndc_width;
	quad[5] = area.ndc_y;
	//tex coord
	quad[6] = 1.0;
	quad[7] = 0.0;
	//vertex
	quad[8] = area.ndc_x;
	quad[9] = area.ndc_y + area.ndc_height;
	//tex coord
	quad[10] = 0.0;
	quad[11] = 1.0;

	//other triangle
	//vertex
	quad[12] = area.ndc_x;
	quad[13] = area.ndc_y + area.ndc_height;
	//tex coord
	quad[14] = 0.0;
	quad[15] = 1.0;
	//vertex
	quad[16] = area.ndc_x + area.ndc_width;
	quad[17] = area.ndc_y;
	//tex coord
	quad[18] = 1.0;
	quad[19] = 0.0;
	//vertex
	quad[20] = area.ndc_x + area.ndc_width;
	quad[21] = area.ndc_y + area.ndc_height;
	//tex coord
	quad[22] = 1.0;
	quad[23] = 1.0;
}