#include "FontHandler.h"

namespace FontHandler
{
    namespace
    {
        Font font;
        Shader* textRenderShader;
    }

    void initialise()
    {
        textRenderShader = new Shader(RESOURCE_ROOT "shaders_source/textVertex.shader", RESOURCE_ROOT "shaders_source/textFragment.shader");
    }

    Shader* getTextShader()
    {
        return textRenderShader;
    }

    void terminate()
    {
        delete textRenderShader;
        glDeleteTextures(1, &font.textureID);
    }

    Character* getCharacter(char c)
    {
        std::map<char, Character>::iterator iterator = font.characters.find(c);

        // Found the character
        if (iterator != font.characters.end())
            return &iterator->second;

        // Did not find the character
        return nullptr;
    }

    void loadFont(const std::string& fontPath,
        float characterSpacing,
        float spaceWidth,
        unsigned int resolution)
    {
        if (resolution < 12)
        {
            std::cout << "ERROR: font resolution cannot be less than 12" << std::endl;
            return;
        }

        FT_Library ft;
        // All functions return a value different than 0 whenever an error occurred
        if (FT_Init_FreeType(&ft))
        {
            std::cout << "ERROR::FREETYPE: Failed to initialise FreeType Library" << std::endl;
            return;
        }

        // Loading the font as a face
        FT_Face face;
        if (FT_New_Face(ft, fontPath.c_str(), 0, &face))
        {
            std::cout << "ERROR::FREETYPE: Failed to load font " << fontPath << std::endl;
            return;
        }

        // Setting size to load glyphs as
        FT_Set_Pixel_Sizes(face, 0, 96);

        // Disabling the restriction for byte-alignment
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        unsigned int textureWidth{ 0 };
        unsigned int textureHeight{ 0 };
        unsigned int characterTextureGap{ 1 }; // Number of pixel in between each character

        // First calculating the total width and height required for the texture atlas
        for (unsigned char c{ 0 }; c < 128; c++)
        {
            // Loading character glyph 
            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            {
                // If it fails: display an error message and go to the next character
                std::cout << "ERROR::FREETYPE: Failed to load Glyph at character " << c << std::endl;
                continue;
            }

            FT_Bitmap* bitmap = &face->glyph->bitmap;

            // Summing up the texture widths
            textureWidth += bitmap->width;

            // And finding the maximum texture height
            if (bitmap->rows > textureHeight)
            {
                textureHeight = bitmap->rows;
            }
        }

        // Add characterSpacing pixels in between each character to the width
        textureWidth += 127 * characterTextureGap;

        // Creating an array for the final texture
        char* pixels = (char*)calloc(textureWidth * textureHeight, sizeof(char));

        // The total number of pixel travelled along the texture
        unsigned int widthOffset{ 0 };

        // The mapping from a character to a specific texture and its data
        std::map<char, Character> characters;

        // If we got here, the font was successfully loaded
        // Then we go and load the first 128 characters of the ASCII set
        for (unsigned char c{ 0 }; c < 128; c++)
        {
            // Loading character glyph 
            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            {
                // If it fails: go to the next character
                continue;
            }

            //std::cout << "Loading character: " << c << std::endl;

            FT_Bitmap* bitmap = &face->glyph->bitmap;

            // Writing this character's texture to the big texture
            for (unsigned int row = 0; row < bitmap->rows; ++row)
            {
                for (unsigned int col = 0; col < bitmap->width; ++col)
                {
                    unsigned int x = widthOffset + col;
                    unsigned int y = textureHeight - row - 1;
                    pixels[y * textureWidth + x] = bitmap->buffer[row * bitmap->pitch + col];
                }
            }

            // Calculating inmportant character data
            Character character{
                glm::vec2(bitmap->width, bitmap->rows) / (float)textureHeight * 0.1f, // Size: width, height
                //glm::vec2(face->glyph->bitmap_left, face->glyph->bitmap_top) / (float)textureHeight * 0.1f, // Offset from top left
                glm::vec2(0.0f, face->glyph->bitmap_top) / (float)textureHeight * 0.1f, // Offset from top left
                glm::vec2(widthOffset / (float)textureWidth, 1.0f - (bitmap->rows / (float)textureHeight)), // UV
                glm::vec2(bitmap->width / (float)textureWidth, bitmap->rows / (float)textureHeight) // Texture space size
            };

            /*
            std::cout << "======== Character " << c << " ========\n";
            std::cout << "Size: " << character.size.x << ", " << character.size.y << "\n";
            std::cout << "Offset: " << character.offset.x << ", " << character.offset.y << "\n";
            std::cout << "UV: " << character.uv.x << ", " << character.uv.y << "\n";
            std::cout << "Texture size: " << character.textureSize.x << ", " << character.textureSize.y;
            std::cout << std::endl;
            */
            characters.emplace(c, character);

            // Moving along by the character's width + the spacing (pixels)
            widthOffset += bitmap->width + characterTextureGap;
        }

        // Creating a texture slot, binding it, and filling it with data
        unsigned int textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(
            GL_TEXTURE_2D,              // Type
            0,                          // Level
            GL_RED,                     // Channel: only red (it is black/white but red is the first and only channel need)
            textureWidth,               // Width
            textureHeight,              // Height
            0,                          // Border: none
            GL_RED,                     // Format
            GL_UNSIGNED_BYTE,           // Type: unsigned byte = 1 byte
            pixels                      // Data pointer
        );

        // Setting relevant texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        /*
        std::cout << "======== FONT TEXTURE DATA ========\n";
        std::cout << "Width: " << textureWidth << "px\n";
        std::cout << "Height: " << textureHeight << "px\n";
        std::cout << std::endl;
        */

        // Modifying the width of the space
        characters[' '].size.x = spaceWidth * 0.02f;

        font = Font{ textureID, characterSpacing, 0.1f, characters };

        // Destroying freetype after using it to extract all data
        FT_Done_Face(face);
        FT_Done_FreeType(ft);
    }

    Font& getFont()
    {
        return font;
    }
};