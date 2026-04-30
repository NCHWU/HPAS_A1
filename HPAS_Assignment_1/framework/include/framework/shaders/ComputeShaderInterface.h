#pragma once

#include <glad/glad.h>

class ComputeShaderInterface
{
public:
	virtual void run(unsigned int numGroupsX, unsigned int numGroupsY = 1, unsigned int numGroupsZ = 1) const;
};

