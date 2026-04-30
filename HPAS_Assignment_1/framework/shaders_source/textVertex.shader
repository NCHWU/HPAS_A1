#version 410 core
layout(location = 0) in vec2 vertexPos;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec4 color;
out vec2 TexCoords;
out vec4 toFrag_color;

uniform mat4 transform;
uniform float scale;
uniform vec2 position;
uniform vec2 textBoundsSize;
uniform bool originLeft;
uniform bool originTop;

uniform int width;
uniform int height;

void main()
{
    float fwidth = float(width);
    float fheight = float(height);
    

    vec2 pos = vertexPos * scale + vec2(0.0f, -fheight);

    if (!originLeft) {
        // Origin must be on the right: move everything left
        pos.x -= textBoundsSize.x * scale;
    }
    if (!originTop) {
        // Origin must be on the bottom: move everything up
        pos.y += textBoundsSize.y * scale;
    }

    pos += position;

    vec2 relativePos = vec2(pos.x / fwidth, pos.y / fheight);
    vec2 absolutePos = relativePos * 2 - vec2(1.) + vec2(0., 2.);
    gl_Position = vec4(absolutePos, 0.0, 1.0);
    TexCoords = uv;
    toFrag_color = color;
}