#include "Shader.h"

Shader::~Shader()
{
	glDeleteProgram(m_ID);
}

unsigned int Shader::CompileShader(unsigned int type, std::string source)
{
	unsigned int id = glCreateShader(type);
	const char* src = source.c_str();

	glShaderSource(id, 1, &src, NULL);
	glCompileShader(id);

	int success;
	char infoLog[512];

	glGetShaderiv(id, GL_COMPILE_STATUS, &success);

	if (success == GL_FALSE) {
		glGetShaderInfoLog(id, 512, nullptr, infoLog);
		std::cout << "COMPILATION FAILED\n" << infoLog << std::endl;
	}

	return id;
}

unsigned int Shader::GetID()
{
	return m_ID;
}

void Shader::Use(GLuint programID)
{
	glUseProgram(programID);
}