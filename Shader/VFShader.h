#ifndef VFSHADER_H
#define VFSHADER_H

#include "Shader.h"

class VFShader : public Shader
{
public:
	VFShader(std::string vsPath, std::string fsPath);

	void CreateProgram() override;

private:
	std::string m_vsSource;
	std::string m_fsSource;
};

#endif // !VFSHADER_H