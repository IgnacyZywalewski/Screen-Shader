#version 330

in vec2 TexCoord; // (x,y)
out vec4 FragColor; //(r,g,b,a)
uniform sampler2D screenTex; //tekstura
uniform vec2 pixelSize; //rozmiar pixela

uniform bool kuwahara;
uniform int kuwaharaRadius;

void applyKuwaharaFilter(out vec3 color) {
    vec3 mean[4];
    vec3 variance[4];

    for (int i = 0; i < 4; i++) {
        mean[i] = vec3(0.0);
        variance[i] = vec3(0.0);
    }

    int count[4] = int[4](0,0,0,0);

    // 0
    for (int y = -kuwaharaRadius; y <= 0; y++) {
        for (int x = -kuwaharaRadius; x <= 0; x++) {
            vec3 sample = texture(screenTex, TexCoord + vec2(x, y) * pixelSize).rgb;
            mean[0] += sample;
            variance[0] += sample * sample;
            count[0]++;
        }
    }

    // 1
    for (int y = -kuwaharaRadius; y <= 0; y++) {
        for (int x = 0; x <= kuwaharaRadius; x++) {
            vec3 sample = texture(screenTex, TexCoord + vec2(x, y) * pixelSize).rgb;
            mean[1] += sample;
            variance[1] += sample * sample;
            count[1]++;
        }
    }

    // 2
    for (int y = 0; y <= kuwaharaRadius; y++) {
        for (int x = -kuwaharaRadius; x <= 0; x++) {
            vec3 sample = texture(screenTex, TexCoord + vec2(x, y) * pixelSize).rgb;
            mean[2] += sample;
            variance[2] += sample * sample;
            count[2]++;
        }
    }

    // 3
    for (int y = 0; y <= kuwaharaRadius; y++) {
        for (int x = 0; x <= kuwaharaRadius; x++) {
            vec3 sample = texture(screenTex, TexCoord + vec2(x, y) * pixelSize).rgb;
            mean[3] += sample;
            variance[3] += sample * sample;
            count[3]++;
        }
    }


    // wybor min
    float minVar = 1e20;
    vec3 result = color;

    for (int i = 0; i < 4; i++) {
        mean[i] /= float(count[i]);
        variance[i] = abs(variance[i] / float(count[i]) - mean[i] * mean[i]);

        float var = variance[i].r + variance[i].g + variance[i].b;

        if (var < minVar) {
            minVar = var;
            result = mean[i];
        }
    }

    color = result;
}

void main() {
    vec2 uv = TexCoord;
    vec3 color = texture(screenTex, uv).rgb;

    if(kuwahara)
        applyKuwaharaFilter(color);

    FragColor = vec4(color, 1.0);

}