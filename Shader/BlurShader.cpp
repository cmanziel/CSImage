#include "BlurShader.h"

BlurShader::BlurShader(std::string csPath, unsigned int imageWidth, unsigned int imageHeight)
	: ComputeShader(csPath), m_ImageWidth(imageWidth), m_ImageHeight(imageHeight)
{

}

void BlurShader::UpdateInputs(unsigned int brushRadius, float* cursorPos)
{
	glUniform1i(glGetUniformLocation(m_ID, "brushRadius"), brushRadius);

	m_BrushRadius = brushRadius;
}

void BlurShader::UpdateBlurCanvas()
{
	// update the sobelCanvas which is the last version of the edited image before the brush state was changed to STATE_SOBEL
	glUniform1i(glGetUniformLocation(m_ID, "state"), STATE_BLUR_UPDATE_CANVAS);
	Dispatch(m_ImageWidth, m_ImageHeight, 1);
}

void BlurShader::Execute()
{
	glUniform1i(glGetUniformLocation(m_ID, "state"), STATE_BLUR_CALCULATE_GRID);
	Dispatch(m_BrushRadius, m_BrushRadius, 1); // dispatch first time for calculateing the kernels

	// set the flag to indicate that the shaders should now do the imageStores to the render texture
	glUniform1i(glGetUniformLocation(m_ID, "state"), STATE_BLUR_DRAW_GRID);
	Dispatch(m_BrushRadius, m_BrushRadius, 1);
}

BlurShader::~BlurShader()
{

}