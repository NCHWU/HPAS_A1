#pragma once

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

// Matrix math
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


/*
* The structure of the shader classes:
* 
* ShaderInterface contains all methods necessary for interacting with a shader.
* 
* AbstractShader contains all methods and members necessary for holding and interacting with
* an actual shader: this means that it has an actual ID.
* 
* ComputeShaderInterface contains all methods necessary for interacting with a compute shader.
*/

class ShaderInterface
{
public:

	// Uniform setting functions
	virtual void setBool(const std::string& name, bool value) const = 0;
	virtual void setFloat(const std::string& name, float value) const = 0;
	virtual void setDouble(const std::string& name, double value) const = 0;
	virtual void setInt(const std::string& name, int value) const = 0;
	virtual void setVector2(const std::string& name, float v1, float v2) const = 0;
	virtual void setVector2(const std::string& name, glm::vec2 v) const = 0;
	virtual void setVector3(const std::string& name, float v1, float v2, float v3) const = 0;
	virtual void setVector3(const std::string& name, glm::vec3 v) const = 0;
	virtual void setMat4(const std::string& name, glm::mat4 matrix) const = 0;
};

