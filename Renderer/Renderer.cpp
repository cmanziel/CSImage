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
	: m_ComputeShader("Shader/shaders/shader.comp"), m_VFShader("Shader/shaders/shader.vert", "Shader/shaders/shader.frag"), m_Window(win)
{
	unsigned int imageWidth = m_Window->GetWidth();
	unsigned int imageHeight = m_Window->GetHeight();

	m_ComputeShader.CreateProgram();
	m_VFShader.CreateProgram();

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

	// screen quad VBO
	glGenBuffers(1, &m_ScreenQuadBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_ScreenQuadBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
}

void Renderer::Draw()
{
	m_ComputeShader.Use();

	m_ComputeShader.Dispatch(m_Window->GetWidth() / 16, m_Window->GetHeight() / 16, 1);

	m_VFShader.Use();

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
	glDeleteBuffers(1, &m_ScreenQuadBuffer);
}