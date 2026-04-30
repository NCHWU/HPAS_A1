#include "shaders/ComputeShader.h"

ComputeShader::ComputeShader(const char* shaderPath)
{
	ID = glCreateProgram();

	std::string shaderCode = readFile(shaderPath);
	const char* shaderCodeChars = shaderCode.c_str();


	/* Compiling the shaders */

	unsigned int shader;
	shader = compileShader(GL_COMPUTE_SHADER, shaderCodeChars);


	/* Creating the shader program */

	ID = glCreateProgram();
	glAttachShader(ID, shader);
	linkProgram();

	// Deleting the shader as it's linked into our program now and no longer necessary
	glDeleteShader(shader);
}

ComputeShader::~ComputeShader()
{
}

ComputeShader::ComputeShader()
{
}
