#include "SobelShader.h"

SobelShader::SobelShader(std::string csPath, unsigned int imageWidth, unsigned int imageHeight)
	: ComputeShader(csPath), m_ImageWidth(imageWidth), m_ImageHeight(imageHeight)
{
	// set up shader buffer storage object
	glGenBuffers(1, &m_GridBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_GridBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_BRUSH_RADIUS * MAX_BRUSH_RADIUS * sizeof(float) * 4, NULL, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_GridBuffer);

	glGenTextures(1, &m_SobelCanvas);
	glBindTexture(GL_TEXTURE_2D, m_SobelCanvas);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_ImageWidth, m_ImageHeight, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindImageTexture(2, m_SobelCanvas, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
}

void SobelShader::UpdateInputs(unsigned int brushRadius, float* cursorPos)
{
	glUniform1i(glGetUniformLocation(m_ID, "brushRadius"), brushRadius);

	m_BrushRadius = brushRadius;
}

void SobelShader::UpdateSobelCanvas()
{
	// update the sobelCanvas which is the last version of the edited image before the brush state was changed to STATE_SOBEL
	glUniform1i(glGetUniformLocation(m_ID, "state"), STATE_UPDATE_SOBEL_CANVAS);
	Dispatch(m_ImageWidth, m_ImageHeight, 1);
}

void SobelShader::Execute()
{
	glUniform1i(glGetUniformLocation(m_ID, "state"), STATE_CALCULATE_GRID);
	Dispatch(m_BrushRadius, m_BrushRadius, 1); // dispatch first time for calculateing the kernels

	// set the flag to indicate that the shaders should now do the imageStores to the render texture
	glUniform1i(glGetUniformLocation(m_ID, "state"), STATE_DRAW_GRID);
	Dispatch(m_BrushRadius, m_BrushRadius, 1);
}

SobelShader::~SobelShader()
{
	glDeleteBuffers(1, &m_GridBuffer);
	glDeleteTextures(1, &m_SobelCanvas);
}