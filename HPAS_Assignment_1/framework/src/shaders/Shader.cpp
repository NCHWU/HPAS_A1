
#include "shaders/Shader.h"

Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
	ID = glCreateProgram();

	std::string vertexCode = readFile(vertexPath);
	std::string fragmentCode = readFile(fragmentPath);
	const char* vShaderCode = vertexCode.c_str();
	const char* fShaderCode = fragmentCode.c_str();

	/* Compiling the shaders */

	unsigned int vertex, fragment;
	vertex = compileShader(GL_VERTEX_SHADER, vShaderCode);
	fragment = compileShader(GL_FRAGMENT_SHADER, fShaderCode);


	/* Creating the shader program */
	glAttachShader(ID, vertex);
	glAttachShader(ID, fragment);

	linkProgram();

	// Deleting the shaders as they're linked into our program now and no longer necessary
	glDeleteShader(vertex);
	glDeleteShader(fragment);
}

Shader::Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath)
{
	ID = glCreateProgram();

	std::string vertexCode = readFile(vertexPath);
	std::string fragmentCode = readFile(fragmentPath);
	std::string geometryCode = readFile(geometryPath);
	const char* vShaderCode = vertexCode.c_str();
	const char* fShaderCode = fragmentCode.c_str();
	const char* gShaderCode = geometryCode.c_str();


	/* Compiling the shaders */

	unsigned int vertex, fragment, geometry;
	vertex = compileShader(GL_VERTEX_SHADER, vShaderCode);
	fragment = compileShader(GL_FRAGMENT_SHADER, fShaderCode);
	geometry = compileShader(GL_GEOMETRY_SHADER, gShaderCode);


	/* Creating the shader program */

	ID = glCreateProgram();
	glAttachShader(ID, vertex);
	glAttachShader(ID, fragment);
	glAttachShader(ID, geometry);

	linkProgram();

	// Deleting the shaders as they're linked into our program now and no longer necessary
	glDeleteShader(vertex);
	glDeleteShader(fragment);
	glDeleteShader(geometry);
}

Shader::~Shader()
{
}