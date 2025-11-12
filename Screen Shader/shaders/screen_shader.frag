#version 460

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


void _forceKeepUniforms()
{
    float dummy = brightness + gamma + contrast + saturation + red + green + blue;
    dummy += float(colorInversion) + float(blackWhite);
    dummy += texture(screenTex, vec2(0.0)).r;
    if (dummy < 0.0) discard;
}

vec3 rgb2hsv(vec3 c) {
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));
    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
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

void applyColorInversion(inout vec3 color) {
    color = 1.0 - color;
}

void changeRGB(inout vec3 color) {
    color.r = clamp(color.r * red, 0.0, 1.0);
    color.g = clamp(color.g * green, 0.0, 1.0);
    color.b = clamp(color.b * blue, 0.0, 1.0);
}

void applyBlackWhiteFilter(inout vec3 color) {
    color = vec3(dot(color.rgb, vec3(0.299, 0.587, 0.114)));
}

void main()
{
    //_forceKeepUniforms();

    vec2 uv = TexCoord;
    vec3 color = texture(screenTex, uv).rgb;

    changeBrightness(color);
    changeGamma(color);
    changeContrast(color);
    changeSaturation(color);
    changeRGB(color);

    if (colorInversion)
        applyColorInversion(color);

    if (blackWhite)
        applyBlackWhiteFilter(color);

    FragColor = vec4(color, 1.0);
}
