#ifndef SOBEL_H
#define SOBEL_H

#include "ComputeShader.h"

#define MAX_BRUSH_RADIUS 200

enum sobel_states
{
	STATE_CALCULATE_GRID, STATE_DRAW_GRID
};

class SobelShader : public ComputeShader
{
public:
	//using ComputeShader::ComputeShader;

	SobelShader(std::string source);
	~SobelShader();

	void UpdateInputs(unsigned int brushRadius, float* cursorPos);

	void Execute() override;
private:
	unsigned int m_BrushRadius;
	GLuint m_GridBuffer;
};

#endif // !SOBEL_H