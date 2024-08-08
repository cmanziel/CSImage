#ifndef SOBEL_H
#define SOBEL_H

#include "ComputeShader.h"

#define MAX_BRUSH_RADIUS 200

enum sobel_states
{
	STATE_CALCULATE_GRID, STATE_DRAW_GRID, STATE_UPDATE_SOBEL_CANVAS
};

class SobelShader : public ComputeShader
{
public:
	//using ComputeShader::ComputeShader;

	SobelShader(std::string source, unsigned int imageWidth, unsigned int imageHeight);
	~SobelShader();

	void UpdateSobelCanvas();
	void UpdateInputs(unsigned int brushRadius, float* cursorPos);

	void Execute() override;
private:
	unsigned int m_BrushRadius;
	GLuint m_GridBuffer;
	GLuint m_SobelCanvas;
	unsigned int m_ImageWidth;
	unsigned int m_ImageHeight;
};

#endif // !SOBEL_H