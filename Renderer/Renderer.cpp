#include "Renderer.h"

float quad[] = {
	// 2 floats for postions coordinates, 2 for texture coordinates
	-1.0f, 1.0f, 0.0f, 1.0f,
	-1.0f, -1.0f, 0.0f, 0.0f,
	1.0f, -1.0f, 1.0f, 0.0f,
	-1.0f, 1.0f, 0.0f, 1.0f,
	1.0f, -1.0f, 1.0f, 0.0f,
	1.0f, 1.0f, 1.0f, 1.0f
};

Renderer::Renderer(Window* win)
	: m_DrawEraseShader("Shader/shaders/shader.comp"),
	m_VFShader("Shader/shaders/shader.vert", "Shader/shaders/shader.frag"),
	m_CanvasShader("Shader/shaders/canvas.comp"),
	m_SobelShader("Shader/shaders/sobelShader.comp"),
	m_CurrentShader(NULL),
	m_Window(win)
{
	unsigned int imageWidth = m_Window->GetWidth();
	unsigned int imageHeight = m_Window->GetHeight();

	m_CanvasShader.CreateProgram();
	m_VFShader.CreateProgram();
	m_DrawEraseShader.CreateProgram();
	m_SobelShader.CreateProgram();

	// set up the texture to render to
	// assure the texture is complete (see https://www.khronos.org/opengl/wiki/Texture#Texture_completeness)
	glGenTextures(1, &m_RenderTexture);
	glBindTexture(GL_TEXTURE_2D, m_RenderTexture);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8UI, 256, 256, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, NULL);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, imageWidth, imageHeight, 0, GL_RGBA, GL_FLOAT, NULL);
	// mipmap completeness, integer color format do not support linear filtering, use GL_NEAREST
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, 0);
	// access paramter has to be GL_READ_WRITE if a blit operation is called on the framebuffer to whcih the image is attached?
	glBindImageTexture(0, m_RenderTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	if (m_Window->GetImageData() != NULL)
	{
		glGenTextures(1, &m_Canvas);
		glBindTexture(GL_TEXTURE_2D, m_Canvas);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, imageWidth, imageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, m_Window->GetImageData());

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
	glGenBuffers(1, &m_ScreenQuadBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_ScreenQuadBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
}

void Renderer::SelectShader()
{
	uint8_t state = m_Window->GetBrush()->GetState();

	cursor curs = m_Window->GetCursor();
	unsigned int radius = m_Window->GetBrush()->GetRadius();
	float cursorPos[2] = { curs.x, curs.y };

	switch (state)
	{
	case STATE_DRAW: {
		Shader::Use(m_DrawEraseShader.GetID());
		m_DrawEraseShader.UpdateInputs(radius, cursorPos, state);
		m_CurrentShader = &m_DrawEraseShader;
	} break;
	case STATE_ERASE: {
		Shader::Use(m_DrawEraseShader.GetID());
		m_DrawEraseShader.UpdateInputs(radius, cursorPos, state);
		m_CurrentShader = &m_DrawEraseShader;
	} break;
	case STATE_SOBEL: {
		Shader::Use(m_SobelShader.GetID());
		m_SobelShader.UpdateInputs(radius, cursorPos);
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