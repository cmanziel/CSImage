#ifndef DRAWERASE_H
#define DRAWERASE_H

#include "ComputeShader.h"

class DrawEraseShader : public ComputeShader
{
public:
	using ComputeShader::ComputeShader;
	void UpdateInputs(unsigned int brushRadius, float* cursorPos, uint8_t state);

	void Execute() override;
private:
	unsigned int m_BrushRadius;
	uint8_t m_State; // either draw or erase
};

#endif // !DRAWERASE_H