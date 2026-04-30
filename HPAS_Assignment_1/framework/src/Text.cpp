#include "Text.h"

void Text::render()
{
    Shader* shader = FontHandler::getTextShader();

    shader->use();

    Font& font{ FontHandler::getFont() };

    shader->setInt("text", 0);
    shader->setInt("width", WindowHandler::getWindowSize().x);
    shader->setInt("height", WindowHandler::getWindowSize().y);

    shader->setFloat("scale", scale);
    shader->setVector2("position", position+glm::vec2(padding.x, -padding.y));
    shader->setVector2("textBoundsSize", textBoundsSize);
    shader->setBool("originLeft", originLeft);
    shader->setBool("originTop", originTop);

    // Binding the texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, font.textureID);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    // Binding the vertex data
    glBindVertexArray(textVAO);

    glDrawArrays(GL_TRIANGLES, 0, 6 * characterCount);

    glBindVertexArray(0);
}

void Text::setText(const std::string& text)
{
    this->text = text;

    updateVAO();
}

void Text::setPosition(glm::vec2 position)
{
    this->position = position;
}

void Text::setNormalizedPosition(glm::vec2 position)
{
    this->position = (position + glm::vec2(1.0f)) * 0.5f * glm::vec2(WindowHandler::getWindowSize());
}

void Text::setVerticalOriginTop()
{
    originTop = true;
}

void Text::setVerticalOriginBottom()
{
    originTop = false;
}

void Text::setHorizontalOriginLeft()
{
    originLeft = true;
}

void Text::setHorizontalOriginRight()
{
    originLeft = false;
}

void Text::setScale(float scale)
{
    this->scale = scale;
}

Text::Text()
{
    // Generating the required objects
    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);

    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);

    // Letting OpenGL know how to interpret the data:
    // 2 floats for position
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // 2 floats for uv
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // 4 floats for color
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(4 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Unbinding
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

Text::~Text()
{
    glDeleteVertexArrays(1, &textVAO);
    glDeleteBuffers(1, &textVBO);
}

void Text::updateVAO()
{
    characterCount = 0;

    Font& font{ FontHandler::getFont() };

    characterSpacing = 0.05f * permanentScaleMult;
    float additionalLineHeight{ 0.0f };
    lineHeight = (font.lineHeight + additionalLineHeight) * permanentScaleMult;

    // Buffer with space for:
    // 6 vertices per character, each with 2 floats for pos, 2 floats for uv.
    std::vector<TextVertexData> vertexData;
    // The number of indices to offset to the next character:
    // 6 vertices, each with 2 times 2 floats (pos, uv)
    unsigned int indexOffsetPerCharacter{ 6 * (2 + 2 + 4) };

    unsigned int characterIndex{ 0 };

    // Start at top-left
    float xOrigin{ 0.0f };
    float yOrigin{ 0.0f };

    float xOffset{ 0.0f };
    // Moving down by one line height by default
    float yOffset{ -lineHeight };

    textBoundsPosition.x = xOrigin;
    textBoundsPosition.y = yOrigin;

    // Iterator through the string
    const char* c = text.c_str();

    float wordLength{ 0.0f };

    // Keeps track of the first character of the current line,
    // So that we can iterate back to it
    const char* firstCharacterOfCurrentLine{ c };

    unsigned int line{ 0 };
    unsigned int column{ 0 };

    float maxX = 0.0f;
    float maxY = 0.0f;

    glm::vec4 tokenColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

    while (*c != '\0') // Go on until hitting delimitter
    {
        // On a newline, move to the next line
        if (*c == '\n')
        {
            // Move back to the left side
            xOffset = 0.0f;

            // And move down
            yOffset -= lineHeight;

            // Moving to the next character
            c++;
            line++;
            column = 0;
            continue;
        }

        Character* ch = FontHandler::getCharacter(*c);

        if (ch == nullptr)
        {
            std::cout << "Failed to load character " << *c << std::endl;

            characterIndex++;
            c++;

            continue;
        }

        // Calculating the top-left position of this character
        float xPos{ xOrigin + xOffset + (ch->offset.x) * permanentScaleMult };
        float yPos{ yOrigin + yOffset + (ch->offset.y) * permanentScaleMult };
        float xSize{ ch->size.x * permanentScaleMult };
        float ySize{ ch->size.y * permanentScaleMult };

        // Adding additional offset for the character not being in the center
        float spacingDifference = characterSpacing - xSize;
        xPos += spacingDifference / 2.0f;

        // Reference index for the vertex
        unsigned int vertexStartIndex{ characterIndex * indexOffsetPerCharacter };

        // Setting the positions of the vertices
        vertexData.emplace_back(
            TextVertexData{glm::vec2(xPos, yPos),
            glm::vec2(ch->uv.x, ch->uv.y + ch->textureSize.y),
            tokenColor});

        vertexData.emplace_back(
            TextVertexData{glm::vec2(xPos, yPos - ySize),
            glm::vec2(ch->uv.x, ch->uv.y),
            tokenColor});

        vertexData.emplace_back(
            TextVertexData{glm::vec2(xPos + xSize, yPos - ySize),
            glm::vec2(ch->uv.x + ch->textureSize.x, ch->uv.y),
            tokenColor});

        vertexData.emplace_back(
            TextVertexData{glm::vec2(xPos, yPos),
            glm::vec2(ch->uv.x, ch->uv.y + ch->textureSize.y),
            tokenColor}
        );

        vertexData.emplace_back(
            TextVertexData{glm::vec2(xPos + xSize, yPos - ySize),
            glm::vec2(ch->uv.x + ch->textureSize.x, ch->uv.y),
            tokenColor}
        );

        vertexData.emplace_back(
            TextVertexData{glm::vec2(xPos + xSize, yPos),
            glm::vec2(ch->uv.x + ch->textureSize.x, ch->uv.y + ch->textureSize.y),
            tokenColor}
        );

        /*
        std::cout << "======== Character " << *c << " bounds ========\n";
        std::cout << "Start: " << xPos << ", " << yPos << "\n";
        std::cout << "End: " << xPos + ch->size.x << ", " << yPos + ch->size.y << "\n";
        std::cout << "UV Start: " << ch->uv.x << ", " << ch->uv.y << "\n";
        std::cout << "UV End: " << ch->uv.x + ch->textureSize.x << ", " << ch->uv.y + ch->textureSize.y;
        std::cout << std::endl;
        */
        characterCount++;

        // Moving to the next character in the input string
        characterIndex++;
        xOffset += characterSpacing;
        //xOffset += xSize + font.characterSpacing * 0.01f;

        if (xOffset > maxX)
        {
            maxX = xOffset;
        }
        if (yOffset < maxY) {
            maxY = yOffset;
        }

        c++;
        column++;
    }

    textBoundsSize.x = maxX - xOrigin + padding.x * 2;
    textBoundsSize.y = yOrigin - maxY + padding.y * 2;

    // Putting the vertex data into a vertex buffer

    // Making sure everything gets put on this specific VAO
    glBindVertexArray(textVAO);

    // Binding the buffer
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);

    // Putting the vertices into the buffer
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(TextVertexData), vertexData.data(), GL_STATIC_DRAW);

    // Unbinding
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
