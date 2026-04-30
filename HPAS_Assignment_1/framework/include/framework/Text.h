#pragma once

#include <string>
#include <vector>

#include "FontHandler.h"
#include "WindowHandler.h"

struct TextVertexData
{
	glm::vec2 pos;
	glm::vec2 uv;
	glm::vec4 color;
};

class Text
{
public:
	Text();
	virtual ~Text();

	virtual void render();

	virtual void setText(const std::string& text);

	void setPosition(glm::vec2 position);
	void setNormalizedPosition(glm::vec2 position);

	void setVerticalOriginTop();
	void setVerticalOriginBottom();
	void setHorizontalOriginLeft();
	void setHorizontalOriginRight();
	void setScale(float scale);

protected:

	virtual void updateVAO();

	// Members
	unsigned int characterCount{ 0 };

	std::string text;

	bool originTop{ false };
	bool originLeft{ false };

	glm::vec2 size{ glm::vec2(300.0f) };
	glm::vec2 position{ glm::vec2(0.0f) };
	glm::vec2 padding{ glm::vec2(0.0f) };
	float scale{ 1.0f };
	const float permanentScaleMult{ 200.0f };
	float characterSpacing{ 0.05f * permanentScaleMult };
	float lineHeight{ 0.0f };
	unsigned int textVAO{ 0 };
	unsigned int textVBO{ 0 };
	glm::vec2 textBoundsPosition{ glm::vec2(0.0f) };
	glm::vec2 textBoundsSize{ glm::vec2(0.0f) };
};

