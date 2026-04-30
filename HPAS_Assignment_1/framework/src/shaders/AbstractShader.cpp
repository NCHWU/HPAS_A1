#include "shaders/AbstractShader.h"

AbstractShader::~AbstractShader()
{
	deleteProgram();
}

void AbstractShader::use() const
{
	glUseProgram(ID);
}

void AbstractShader::setBool(const std::string& name, bool value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}
void AbstractShader::setFloat(const std::string& name, float value) const
{
	glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}
void AbstractShader::setDouble(const std::string& name, double value) const
{
	glUniform1d(glGetUniformLocation(ID, name.c_str()), value);
}
void AbstractShader::setInt(const std::string& name, int value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}
void AbstractShader::setVector2(const std::string& name, float v1, float v2) const
{
	glUniform2f(glGetUniformLocation(ID, name.c_str()), v1, v2);
}
void AbstractShader::setVector2(const std::string& name, glm::vec2 v) const
{
	glUniform2f(glGetUniformLocation(ID, name.c_str()), v.x, v.y);
}
void AbstractShader::setVector3(const std::string& name, float v1, float v2, float v3) const
{
	glUniform3f(glGetUniformLocation(ID, name.c_str()), v1, v2, v3);
}
void AbstractShader::setVector3(const std::string& name, glm::vec3 v) const
{
	glUniform3f(glGetUniformLocation(ID, name.c_str()), v.x, v.y, v.z);
}
void AbstractShader::setMat4(const std::string& name, glm::mat4 matrix) const
{
	glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(matrix));
}

void AbstractShader::deleteProgram()
{
	glDeleteProgram(ID);
}

unsigned int AbstractShader::compileShader(GLenum type, const char* code)
{
	unsigned int id;
	int success;
	char infoLog[512];

	id = glCreateShader(type);
	glShaderSource(id, 1, &code, NULL);
	glCompileShader(id);

	// Printing errors:
	glGetShaderiv(id, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(id, 512, NULL, infoLog);
		std::cerr << "shader compilation failed." << std::string(infoLog) << std::endl;
	}
	return id;
}

bool AbstractShader::replace(std::string& str, const std::string& from, const std::string& to)
{
	size_t start_pos = str.find(from);
	while (start_pos != std::string::npos)
	{
		str.replace(start_pos, from.length(), to);
		start_pos = str.find(from);
	}
	return true;
}

std::string AbstractShader::extractFileName(const char* filePath)
{
	// Convert the const char* to a string for easier manipulation
	std::string path(filePath);

	// Find the last path separator (either '/' or '\\') in the string
	size_t lastSeparator = path.find_last_of("/\\");

	if (lastSeparator != std::string::npos)
	{
		// Extract the substring after the last separator (i.e., the file name with extension)
		std::string fileNameWithExtension = path.substr(lastSeparator + 1);

		// Find the last dot in the substring to determine the file extension
		size_t lastDot = fileNameWithExtension.find_last_of('.');

		if (lastDot != std::string::npos)
		{
			// Extract the substring before the last dot (i.e., the file name without extension)
			return fileNameWithExtension.substr(0, lastDot);
		}
		else
		{
			// If there is no dot, the file has no extension, so return the whole name
			return fileNameWithExtension;
		}
	}

	// If no separator is found, return the entire path as the file name
	return path;
}

std::string AbstractShader::readFile(const char* shaderPath)
{
	/* Retrieving the shader data from the file */
	std::string shaderCode;
	std::ifstream shaderFile;

	// Ensuring ifstream objects can throw exceptions:
	shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try
	{
		// Opening files
		shaderFile.open(shaderPath);

		// Reading file into stream
		std::stringstream shaderStream;
		shaderStream << shaderFile.rdbuf();

		// Closing file handler
		shaderFile.close();

		// Converting the stream into a string
		shaderCode = shaderStream.str();
	}
	catch (std::ifstream::failure e)
	{
		std::cerr << "Error: shader file not correctly read." << std::endl;
	}
	return shaderCode;
}

void AbstractShader::linkProgram()
{
	glLinkProgram(ID);

	/* Printing linking errors if any */
	if (true)
	{
		int success;
		char infoLog[512];

		glGetProgramiv(ID, GL_LINK_STATUS, &success);

		if (!success)
		{
			// Getting the error log
			glGetProgramInfoLog(ID, 512, NULL, infoLog);

			// Printing the error log
			std::cerr << "Error: shader program linking failed." << std::endl;
			std::cerr << infoLog << std::endl;
			return;
		}
	}
}