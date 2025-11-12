#version 460

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform bool horizontalSwap;
uniform bool verticalSwap;

void main()
{
    vec2 tex = aTexCoord;

    if (horizontalSwap) 
        tex.x = 1.0 - tex.x;
    if (verticalSwap) 
        tex.y = 1.0 - tex.y;

    tex += vec2(float(horizontalSwap) * 0.0, float(verticalSwap) * 0.0);

    TexCoord = tex;
    gl_Position = vec4(aPos, 0.0, 1.0);
}
