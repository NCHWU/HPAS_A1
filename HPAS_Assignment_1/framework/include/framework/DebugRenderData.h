#pragma once

#include <glm/glm.hpp>
#include <vector>

struct DebugRenderDataPoint3D
{
	glm::vec3 point;
	glm::vec3 color;
};
struct DebugRenderData3D
{
	bool enabled{ true };
	std::vector<DebugRenderDataPoint3D> points;

	void addLine(glm::vec3 start, glm::vec3 end, glm::vec3 color)
	{
		if (!enabled)
		{
			return;
		}

		points.push_back({start, color});
		points.push_back({end, color});
	}

	void addCube(glm::vec3 min, glm::vec3 max, glm::vec3 color)
	{
		if (!enabled)
		{
			return;
		}

		// Bottom
		addLine(glm::vec3(min.x, min.y, min.z), glm::vec3(max.x, min.y, min.z), color);
		addLine(glm::vec3(max.x, min.y, min.z), glm::vec3(max.x, max.y, min.z), color);
		addLine(glm::vec3(max.x, max.y, min.z), glm::vec3(min.x, max.y, min.z), color);
		addLine(glm::vec3(min.x, max.y, min.z), glm::vec3(min.x, min.y, min.z), color);

		// Top
		addLine(glm::vec3(min.x, min.y, max.z), glm::vec3(max.x, min.y, max.z), color);
		addLine(glm::vec3(max.x, min.y, max.z), glm::vec3(max.x, max.y, max.z), color);
		addLine(glm::vec3(max.x, max.y, max.z), glm::vec3(min.x, max.y, max.z), color);
		addLine(glm::vec3(min.x, max.y, max.z), glm::vec3(min.x, min.y, max.z), color);

		// Sides
		addLine(glm::vec3(min.x, min.y, min.z), glm::vec3(min.x, min.y, max.z), color);
		addLine(glm::vec3(max.x, min.y, min.z), glm::vec3(max.x, min.y, max.z), color);
		addLine(glm::vec3(max.x, max.y, min.z), glm::vec3(max.x, max.y, max.z), color);
		addLine(glm::vec3(min.x, max.y, min.z), glm::vec3(min.x, max.y, max.z), color);
	}
};

struct DebugRenderDataPoint2D
{
	glm::vec2 point;
	glm::vec3 color;
};
struct DebugRenderData2D
{
	bool enabled{ true };
	std::vector<DebugRenderDataPoint2D> points;
	std::vector<DebugRenderDataPoint2D> lines;

	void clear()
	{
		points.clear();
		lines.clear();
	}

	void addPoint(glm::vec2 point, glm::vec3 color)
	{
		if (!enabled)
		{
			return;
		}

		points.push_back(DebugRenderDataPoint2D{point, color});
	}

	void addLine(glm::vec2 start, glm::vec2 end, glm::vec3 color)
	{
		if (!enabled)
		{
			return;
		}

		lines.push_back(DebugRenderDataPoint2D{start, color});
		lines.push_back(DebugRenderDataPoint2D{end, color});
	}

	void addRect(glm::vec2 min, glm::vec2 max, glm::vec3 color)
	{
		if (!enabled)
		{
			return;
		}

		// Bottom
		addLine(glm::vec2(min.x, min.y), glm::vec2(max.x, min.y), color);
		addLine(glm::vec2(max.x, min.y), glm::vec2(max.x, max.y), color);
		addLine(glm::vec2(max.x, max.y), glm::vec2(min.x, max.y), color);
		addLine(glm::vec2(min.x, max.y), glm::vec2(min.x, min.y), color);
	}
};
