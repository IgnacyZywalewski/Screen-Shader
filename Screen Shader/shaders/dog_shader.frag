#version 330

in vec2 TexCoord; // (x,y)
out vec4 FragColor; //(r,g,b,a)
uniform sampler2D screenTex; //tekstura
uniform vec2 pixelSize; //rozmiar pixela

uniform bool dog;
uniform float sigma;
uniform float scale;   
uniform float threshold;
uniform float tau;
uniform vec4 dogColor1;
uniform vec4 dogColor2;

vec3 gaussian1D(float sigma, bool horizontal) {
    int radius = int(ceil(sigma * 3.0f));
    vec3 color = vec3(0.0);
    float sum = 0.0;

    for(int x = -radius; x <= radius; x++){
        float weight = exp(-(x*x)/(2.0 * sigma * sigma));
        vec2 offset = horizontal ? vec2(x * pixelSize.x, 0.0) : vec2(0.0, x * pixelSize.y);

        color += texture(screenTex, TexCoord + offset).rgb * weight;
        sum += weight;
    }

    return color / sum;
}

vec3 gaussianBlur(float sigma) {
    vec3 blurH = gaussian1D(sigma, true);
    vec3 blurV = gaussian1D(sigma, false);

    return (blurH + blurV) / 2.0;
}

void main(){
    vec3 color = texture(screenTex, TexCoord).rgb;

    if(dog){
        vec3 blur1 = gaussianBlur(sigma);
        vec3 blur2 = gaussianBlur(sigma * scale);

        vec3 dogResult = (1 + tau) * blur1 - (tau * blur2);

        float intensity = dot(dogResult, vec3(0.299f, 0.587f, 0.114f));

        //1.0 + tanh(1.0f * (intensity - threshold));
        float edge = intensity > threshold ? 1.0 : 0.0;
        FragColor = mix(dogColor2, dogColor1, edge);
    } 
    else {
        FragColor = texture(screenTex, TexCoord);
    }
}
