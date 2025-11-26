#version 330

in vec2 TexCoord; // (x,y)
out vec4 FragColor; //(r,g,b,a)
uniform sampler2D screenTex; //tekstura
uniform vec2 pixelSize; //rozmiar pixela
uniform float time;

uniform float brightness;
uniform float gamma;
uniform float contrast;
uniform float saturation;

uniform float red;
uniform float green;
uniform float blue;

uniform bool colorInversion;
uniform bool blackWhite;
uniform bool emboss;

uniform bool filmGrain;
uniform float grainAmount;

uniform bool vignette;
uniform float vigRadius;
uniform float vigSmoothness;


void forceKeepUniforms() {
    float dummy = brightness + gamma + contrast + saturation + red + green + blue;
    dummy += float(colorInversion) + float(blackWhite);
    dummy += texture(screenTex, vec2(0.0)).r;
    if (dummy < 0.0) discard;
}

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

void applyEmbossFilter(out vec3 color) {
    color = vec3(0.5);

    color += texture(screenTex, TexCoord - pixelSize).rgb + 1.0;
    color -= texture(screenTex, TexCoord + pixelSize).rgb + 1.0;

    color = vec3((color.r + color.g + color.b) / 3.0);
}

void applyVignetteFilter(out vec3 color) {
    vec2 uv = TexCoord - vec2(0.5);
    
    vec2 texSize = textureSize(screenTex, 0);
    float aspectRatio = texSize.x / texSize.y;

    uv.x *= aspectRatio;
    float dist = length(uv);
    float vignetteValue = smoothstep(vigRadius, vigRadius - vigSmoothness, dist);

    color *= vignetteValue;
}

void applyFilmGrain(out vec3 color) {
    vec3 p = fract(vec3(TexCoord * 800.0, time * 60.0) * 0.1031);
    p += dot(p, p.yzx + 33.33);

    float noise = fract((p.x + p.y) * p.z) - 0.5;

    float luminance = dot(color, vec3(0.299, 0.587, 0.114));
    float fade = 1.0 - luminance;

    color += noise * grainAmount * fade;
}


void main() {
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

    if(emboss)
        applyEmbossFilter(color);

    if(vignette)
        applyVignetteFilter(color);

    if(filmGrain)
        applyFilmGrain(color);

    FragColor = vec4(color, 1.0);
}