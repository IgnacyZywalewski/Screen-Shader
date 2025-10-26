#version 130

in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D screenTex;

uniform float brightness;
uniform float gamma;
uniform float contrast;
uniform float saturation;

uniform float red;
uniform float green;
uniform float blue;

uniform bool colorInversion;

uniform bool blackWhite;


vec3 rgb2hsv(vec3 color) {
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(color.bg, K.wz), vec4(color.gb, K.xy), step(color.b, color.g));
    vec4 q = mix(vec4(p.xyw, color.r), vec4(color.r, p.yzx), step(p.x, color.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 color) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(color.xxx + K.xyz) * 6.0 - K.www);
    return color.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), color.y);
}

void changeBrightness(inout vec3 color) {
    vec3 hsv = rgb2hsv(color);
    hsv.z = clamp(hsv.z * brightness, 0.0, 1.0);
    color = hsv2rgb(hsv);
}

void changeGamma(inout vec3 color) {
    color = clamp(pow(color, vec3(1.0 / gamma)), 0.0, 1.0);
}

void changeContrast(inout vec3 color) {
    float contrastFactor = (259.0 * (contrast + 255.0)) / (255.0 * (259.0 - contrast));
    color = clamp(contrastFactor * (color - 0.5) + 0.5, 0.0, 1.0);
}

void changeSaturation(inout vec3 color) {
    vec3 hsv = rgb2hsv(color);
    hsv.y = clamp(hsv.y * saturation, 0.0, 1.0);
    color = hsv2rgb(hsv);
}

void applyColorInversion(out vec3 color) {
    color = 1.0 - color;
}

void changeRGB(out vec3 color) {
    color.r = clamp(color.r * red, 0.0, 1.0);
    color.g = clamp(color.g * green, 0.0, 1.0);
    color.b = clamp(color.b * blue, 0.0, 1.0);
}

void apllyBlackWhiteFilter(out vec3 color) {
    color = vec3(dot(color.rgb, vec3(0.299, 0.587, 0.114)));
}


void main() {
    vec3 color = texture(screenTex, TexCoord).rgb;

    changeBrightness(color);
    changeGamma(color);
    changeContrast(color);
    changeSaturation(color);

    changeRGB(color);

    if (colorInversion)
        applyColorInversion(color);

    if(blackWhite)
        apllyBlackWhiteFilter(color);

    FragColor = vec4(color, 1.0);
}
