#version 330

in vec2 TexCoord; // (x,y)
out vec4 FragColor; //(r,g,b,a)
uniform sampler2D screenTex; //tekstura
uniform vec2 pixelSize; //rozmiar pixela

uniform bool dog;
uniform int radius1;
uniform int radius2;   
uniform float threshold;

vec3 gaussian1D(int radius, bool horizontal) {
    float sigma = float(radius) / 3.0f;
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

vec3 gaussianBlur(int radius) {
    vec3 blurH = gaussian1D(radius, true);
    vec3 blurV = gaussian1D(radius, false);

    return (blurH + blurV) / 2.0;
}

void main(){
    vec3 color = texture(screenTex, TexCoord).rgb;

    if(dog){
        vec3 blur1 = gaussianBlur(radius1);
        vec3 blur2 = gaussianBlur(radius2);

        vec3 dogResult = blur1 - blur2;

        float intensity = dot(dogResult, vec3(0.299, 0.587, 0.114));

        float edge = intensity > threshold ? 1.0 : 0.0;
        FragColor = vec4(vec3(edge), 1.0);
    } 
    else {
        FragColor = texture(screenTex, TexCoord);
    }
}
