#version 330

in vec2 TexCoord; // (x,y)
out vec4 FragColor; //(r,g,b,a)
uniform sampler2D screenTex; //tekstura

uniform bool pixelate;
uniform int chunk;

void applyPixelateFilter(vec2 uv, out vec3 color) {
    uv.x -= mod(uv.x, 1.0 / chunk);
	uv.y -= mod(uv.y, 1.0 / chunk);

    color = texture2D(screenTex, uv).rgb;
}

void main() {
    vec2 uv = TexCoord;
    vec3 color = texture(screenTex, uv).rgb;

    if(pixelate)
        applyPixelateFilter(uv, color);

    FragColor = vec4(color, 1.0);
}