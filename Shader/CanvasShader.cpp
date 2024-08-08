#include "CanvasShader.h"

void CanvasShader::SetDimensions(unsigned int width, unsigned int height)
{
	m_ImageWidth = width;
	m_ImageHeight = height;
}

void CanvasShader::Execute()
{
	Dispatch(m_ImageWidth, m_ImageHeight, 1);
}