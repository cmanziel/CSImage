#include "ComputeShader.h"

ComputeShader::ComputeShader(std::string csPath)
{
	std::ifstream csFile;
	std::stringstream csSourceStream; // can't directly copy the contents of the file into a string (can't do vsSource << vsFile.rdbuf());

	csFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try
	{
		// open files
		csFile.open(csPath);
		// read file's buffer contents into streams
		csSourceStream << csFile.rdbuf();
		// close file handlers
		csFile.close();
		// convert stream into string
		m_csSource = csSourceStream.str();
	}
	catch (std::ifstream::failure& e)
	{
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << e.what() << std::endl;
	}
}

void ComputeShader::CreateProgram()
{
	unsigned int cs;

	// compile every shader neede in the pipeline and attach them to the program tha will be used for rendering
	cs = CompileShader(GL_COMPUTE_SHADER, m_csSource);

	m_ID = glCreateProgram();
	glAttachShader(m_ID, cs);

	glLinkProgram(m_ID);

	int success;
	char infoLog[512]; // allocate enough chars for the log message

	glGetProgramiv(m_ID, GL_LINK_STATUS, &success);
	if (success == GL_FALSE) {
		glGetProgramInfoLog(m_ID, 512, nullptr, infoLog);
		std::cout << "LINKING FAILED\n" << infoLog << std::endl;
	}

	glDeleteShader(cs);
}

void ComputeShader::Dispatch(unsigned int xGroups, unsigned int yGroups, unsigned int zGroups)
{
	glDispatchCompute(xGroups, yGroups, zGroups);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
}