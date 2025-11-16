#version 460

in vec2 aPos;
in vec2 aTexCoord;
out vec2 TexCoord;

uniform bool horizontalSwap;
uniform bool verticalSwap;

void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;

    if(horizontalSwap)
        TexCoord.x = 1.0 - TexCoord.x;

    if(verticalSwap)
        TexCoord.y = 1.0 - TexCoord.y;

}