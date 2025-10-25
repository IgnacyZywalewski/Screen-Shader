#version 130

in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D screenTex;

uniform float brightness;
uniform float gamma;
uniform float contrast;

uniform bool colorInversion;

uniform float redColor;
uniform float greenColor;
uniform float blueColor;

uniform bool blackWhite;


void changeBrightness(out vec3 color){
    color *= brightness;
}

void changeGamma(out vec3 color){
    color = pow(color, vec3(1.0 / gamma));
}

void changeContrast(out vec3 color){
    float contrastFactor = (259.0 * (contrast + 255.0)) / (255.0 * (259.0 - contrast));
    color = clamp(contrastFactor * (color - 0.5) + 0.5, 0.0, 1.0);
}

void applyColorInversion(out vec3 color){
    color = 1.0 - color;
}

void changeRGB(out vec3 color){
    color.r *= redColor;
    color.g *= greenColor;
    color.b *= blueColor;
}

void apllyBlackWhiteFilter(out vec3 color){
    color = vec3(dot(color.rgb, vec3(0.299, 0.587, 0.114)));
}

void main() {
    vec3 color = texture(screenTex, TexCoord).rgb;

    // jasnoœæ
    changeBrightness(color);

    // gamma
    changeGamma(color);

    // kontrast
    changeContrast(color);

    // inwersja
    if (colorInversion){
        applyColorInversion(color);
    }

    // RGB
    changeRGB(color);

    //filtr czarno bialy
    if(blackWhite){
        apllyBlackWhiteFilter(color);
    }

    FragColor = vec4(color, 1.0);
}
