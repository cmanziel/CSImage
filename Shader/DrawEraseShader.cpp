#include "DrawEraseShader.h"

void DrawEraseShader::UpdateInputs(unsigned int brushRadius, float* cursorPos, uint8_t state)
{
	glUniform1i(glGetUniformLocation(m_ID, "brushRadius"), brushRadius);
	glUniform1i(glGetUniformLocation(m_ID, "drawFlag"), state);

	m_BrushRadius = brushRadius;
}

void DrawEraseShader::Execute()
{
	Dispatch(m_BrushRadius, m_BrushRadius, 1);
}