#ifndef SHADER_H
#define SHADER_H

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>

#include <GL/glew.h>

class Shader
{
public:
	~Shader();

	unsigned int GetID();
	virtual void CreateProgram() = 0;
	static unsigned int CompileShader(unsigned int type, std::string source);

	void Use(); // use the program to which all the shaders are attached

	unsigned int m_ID;
private:
};

#endif // !SHADER_H


