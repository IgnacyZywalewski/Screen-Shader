#version 330

in vec2 TexCoord; // (x,y)
out vec4 FragColor; //(r,g,b,a)
uniform sampler2D screenTex; //tekstura
uniform vec2 pixelSize; //rozmiar pixela

uniform bool blur;
uniform int blurRadius;

void forceKeepUniforms() {
    float dummy = texture(screenTex, vec2(0.0)).r;
    if (dummy < 0.0) discard;
}

void applyGaussianBlur(out vec3 color) {
    color = vec3(0.0);
    float sum = 0.0;

    for (int x = -blurRadius; x <= blurRadius; x++) {
        float weight = exp(-(x*x) / (2.0 * blurRadius * blurRadius));
        vec2 offset = vec2(x * pixelSize.x, 0.0);
        color += texture(screenTex, TexCoord + offset).rgb * weight;
        sum += weight;
    }

    color /= sum;

    //vec3 tempColor = color;
    //color = vec3(0.0);
    //sum = 0.0;

    //for (int y = -blurRadius; y <= blurRadius; y++) {
    //    float weight = exp(-(y*y) / (2.0 * blurRadius * blurRadius));
    //    vec2 offset = vec2(0.0, y * pixelSize.y);
    //    color += texture(screenTex, uv + offset).rgb * weight;
    //    sum += weight;
    //}

    //color /= sum;

    //color = (color + tempColor) / 2.0;
}


void main() {
    vec2 uv = TexCoord;
    vec3 color = texture(screenTex, uv).rgb;

    if(blur)
        applyGaussianBlur(color);

    FragColor = vec4(color, 1.0);
}
