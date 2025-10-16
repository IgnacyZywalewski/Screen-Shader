#version 130

in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D screenTex;

uniform float brightness;
uniform float gamma;
uniform float contrast;

uniform bool colorInversion;

uniform bool redColor;
uniform bool greenColor;
uniform bool blueColor;

uniform bool blackWhite;


void main() {
    vec3 color = texture(screenTex, TexCoord).rgb;

    // jasnoœæ
    color *= brightness;

    // gamma
    color = pow(color, vec3(1.0 / gamma));

    // kontrast
    float contrastFactor = (259.0 * (contrast + 255.0)) / (255.0 * (259.0 - contrast));
    color = clamp(contrastFactor * (color - 0.5) + 0.5, 0.0, 1.0);

    // inwersja
    if (colorInversion)
        color = 1.0 - color;

    // RGB
    if (!redColor)   color.r = 0.0;
    if (!greenColor) color.g = 0.0;
    if (!blueColor)  color.b = 0.0;

    //filtr czarno bialy
    if(blackWhite)
        color = vec3(dot(color.rgb, vec3(0.299, 0.587, 0.114)));

    FragColor = vec4(color, 1.0);

}
