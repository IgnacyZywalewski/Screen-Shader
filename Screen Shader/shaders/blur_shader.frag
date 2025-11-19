#version 330

in vec2 TexCoord; // (x,y)
out vec4 FragColor; //(r,g,b,a)
uniform sampler2D screenTex; //tekstura
uniform vec2 pixelSize; //rozmiar pixela

uniform bool blur;
uniform int blurRadius;

vec3 gaussian1D(bool horizontal) {
    vec3 color = vec3(0.0);
    float sum = 0.0;
    float sigma = float(blurRadius) / 3.0f;

    for (int i = -blurRadius; i <= blurRadius; i++) {
        float x = float(i);
        float weight = exp(-(x*x)/(2.0*sigma*sigma));

        vec2 offset = horizontal ? vec2(x * pixelSize.x, 0.0) : vec2(0.0, x * pixelSize.y);

        color += texture(screenTex, TexCoord + offset).rgb * weight;
        sum += weight;
    }

    return color / sum;
}

vec3 gaussianBlur2D() {
    vec3 blurH = gaussian1D(true);
    vec3 blurV = gaussian1D(false);

    return (blurH + blurV) / 2.0;
}

void main() {
    vec3 color = texture(screenTex, TexCoord).rgb;

    if(blur)
        color = gaussianBlur2D();
    
    FragColor = vec4(color, 1.0);
}
