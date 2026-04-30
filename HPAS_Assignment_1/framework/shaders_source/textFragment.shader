#version 410 core
in vec2 TexCoords;
in vec4 toFrag_color;
out vec4 color;

uniform sampler2D text;

void main()
{
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
    color = sampled * toFrag_color;
}