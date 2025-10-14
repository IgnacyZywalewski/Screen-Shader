#version 130

in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D screenTex;

uniform float brightness; // 0.0–2.0
uniform float contrast;   // -255–255
uniform float gamma;      // 0.0–8.0
uniform bool colorInversion;
uniform bool redColor;
uniform bool greenColor;
uniform bool blueColor;

void main() {
    vec3 color = texture(screenTex, TexCoord).rgb;

    // jasnoœæ
    color *= brightness;

    // kontrast
    float contrastFactor = (259.0 * (contrast + 255.0)) / (255.0 * (259.0 - contrast));
    color = clamp(contrastFactor * (color - 0.5) + 0.5, 0.0, 1.0);

    // gamma
    color = pow(color, vec3(1.0 / gamma));

    // inwersja
    if (colorInversion)
        color = 1.0 - color;

    // RGB
    if (!redColor)   color.r = 0.0;
    if (!greenColor) color.g = 0.0;
    if (!blueColor)  color.b = 0.0;

    FragColor = vec4(color, 1.0);
}
