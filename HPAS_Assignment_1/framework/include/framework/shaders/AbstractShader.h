#pragma once

#include "ShaderInterface.h"

class AbstractShader : public ShaderInterface
{
public:
	// Program ID
	unsigned int ID{ 0 };

	// Activates the shader
	virtual void use() const;

	// Uniform setting functions
	void setBool(const std::string& name, bool value) const override;
	void setFloat(const std::string& name, float value) const override;
	void setDouble(const std::string& name, double value) const override;
	void setInt(const std::string& name, int value) const override;
	void setVector2(const std::string& name, float v1, float v2) const override;
	void setVector2(const std::string& name, glm::vec2 v) const override;
	void setVector3(const std::string& name, float v1, float v2, float v3) const override;
	void setVector3(const std::string& name, glm::vec3 v) const override;
	void setMat4(const std::string& name, glm::mat4 matrix) const override;

	// Delete the shader in this object
	virtual void deleteProgram();

protected:
	unsigned int compileShader(GLenum type, const char* code);
	bool replace(std::string& str, const std::string& from, const std::string& to);
	std::string extractFileName(const char* filePath);
	std::string readFile(const char* shaderPath);
	void linkProgram();

	// Cannot be instantiated
	AbstractShader() {}
	~AbstractShader();

	const char* cacheLocation{ nullptr };
};