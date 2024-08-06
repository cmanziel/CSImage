#ifndef CSHADER_H

#define CSHADER_H

#include "Shader.h"

class ComputeShader : public Shader
{
public:
	ComputeShader(std::string csPath);

	void CreateProgram() override;
	void Dispatch(unsigned int xGroups, unsigned int yGroups, unsigned int zGroups); // call glCOmputeDispatch for the compute shader

	virtual void Execute() = 0;

	std::string m_csSource;
private:
};

#endif // !CSHADER_H