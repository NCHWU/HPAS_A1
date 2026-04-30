#pragma once
#ifndef SHADER_H
#define SHADER_H

#include "AbstractShader.h"

class Scene;

class Shader : public AbstractShader
{
public:
	Shader(const char* vertexPath, const char* fragmentPath);

	// Instantiate a shader including a geometry shader
	Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath);

	virtual ~Shader();
};

#endif