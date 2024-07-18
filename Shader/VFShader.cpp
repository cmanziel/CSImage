#include "VFShader.h"

VFShader::VFShader(std::string vsPath, std::string fsPath)
{
	std::ifstream vsFile, fsFile;
	std::stringstream vsSourceStream, fsSourceStream; // can't directly copy the contents of the file into a string (can't do vsSource << vsFile.rdbuf());

	vsFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fsFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try
	{
		// open files
		vsFile.open(vsPath);
		fsFile.open(fsPath);
		// read file's buffer contents into streams
		vsSourceStream << vsFile.rdbuf();
		fsSourceStream << fsFile.rdbuf();
		// close file handlers
		vsFile.close();
		fsFile.close();
		// convert stream into string
		m_vsSource = vsSourceStream.str();
		m_fsSource = fsSourceStream.str();
	}
	catch (std::ifstream::failure& e)
	{
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << e.what() << std::endl;
	}
}

void VFShader::CreateProgram()
{
	unsigned int vs, fs;

	vs = CompileShader(GL_VERTEX_SHADER, m_vsSource);
	fs = CompileShader(GL_FRAGMENT_SHADER, m_fsSource);

	m_ID = glCreateProgram();
	glAttachShader(m_ID, vs);
	glAttachShader(m_ID, fs);

	glLinkProgram(m_ID);

	int success;
	char infoLog[512];

	//glValidateProgram(program); used as an ulterior check of the execution of the program, used with glGetProgram (similar to glGetProgramiv ecc.)
	glGetProgramiv(m_ID, GL_LINK_STATUS, &success);
	if (success == GL_FALSE) {
		glGetProgramInfoLog(m_ID, 512, nullptr, infoLog);
		std::cout << "LINKING FAILED\n" << infoLog << std::endl;
	}

	glDeleteShader(vs);
	glDeleteShader(fs);
}