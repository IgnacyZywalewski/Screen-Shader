#version 330

in vec2 TexCoord; // (x,y)
out vec4 FragColor; //(r,g,b,a)
uniform sampler2D screenTex; //tekstura
uniform vec2 pixelSize; //rozmiar pixela

uniform bool sharpness;

void changeSharpness(inout vec3 color) {
    vec3 sum = vec3(0.0);

    for(int x = -1; x <= 1; x++) {
        vec2 offset = vec2(x, 0.0) * pixelSize;
        if(x == 0)
            sum += 5 * texture(screenTex, TexCoord + offset).rgb; 
        else
            sum += (-1) * texture(screenTex, TexCoord + offset).rgb; 
    }

    for(int y = -1; y <= 1; y++) {
        vec2 offset = vec2(0.0, y) * pixelSize;
        if(y != 0)
            sum += (-1) * texture(screenTex, TexCoord + offset).rgb; 
    }

    color = clamp(sum, 0.0, 1.0);
}



void main() {
    vec2 uv = TexCoord;
    vec3 color = texture(screenTex, uv).rgb;

    if(sharpness)
        changeSharpness(color);

    FragColor = vec4(color, 1.0);
}