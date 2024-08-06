#ifndef CANVAS_H
#define CANVAS_H

#include "ComputeShader.h"

class CanvasShader : public ComputeShader
{
public:
	using ComputeShader::ComputeShader;
	void SetDimensions(unsigned int width, unsigned int height);
	void Execute() override;
private:
	unsigned int m_ImageWidth;
	unsigned int m_ImageHeight;
};

#endif // !CANVAS_H