#pragma once

#include "ComputeShaderInterface.h"
#include "AbstractShader.h"
#include <vector>

class ComputeShader : public AbstractShader, public ComputeShaderInterface
{
public:
	ComputeShader(const char* shaderPath);
	virtual ~ComputeShader();

protected:

	// Empty constructor is only for the subclasses
	ComputeShader();
};

