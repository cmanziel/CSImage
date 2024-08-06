#include "SobelShader.h"

SobelShader::SobelShader(std::string csPath)
	: ComputeShader(csPath)
{
	// set up shader buffer storage object
	glGenBuffers(1, &m_GridBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_GridBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_BRUSH_RADIUS * MAX_BRUSH_RADIUS * sizeof(float) * 4, NULL, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_GridBuffer);
}

void SobelShader::UpdateInputs(unsigned int brushRadius, float* cursorPos)
{
	glUniform1i(glGetUniformLocation(m_ID, "brushRadius"), brushRadius);

	m_BrushRadius = brushRadius;
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
}