#ifndef BLUR_H
#define BLUR_H

#include "ComputeShader.h"

#define MAX_BRUSH_RADIUS 200

enum blur_states
{
	STATE_BLUR_CALCULATE_GRID, STATE_BLUR_DRAW_GRID, STATE_BLUR_UPDATE_CANVAS
};

class BlurShader : public ComputeShader
{
public:
	//using ComputeShader::ComputeShader;

	BlurShader(std::string source);
	~BlurShader();

	void UpdateBlurCanvas();
	void UpdateInputs(unsigned int brushRadius, unsigned int imageWidth, unsigned int imageHeight);

	void Execute() override;
private:
	unsigned int m_BrushRadius;
	unsigned int m_ImageWidth;
	unsigned int m_ImageHeight;
};

#endif // !SOBEL_H