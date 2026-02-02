#version 330

in vec2 TexCoord; // (x,y)
out vec4 FragColor; //(r,g,b,a)
uniform sampler2D screenTex; //tekstura

uniform bool simProtanopia;
uniform bool protanopia;
uniform float protanopiaStrength;

uniform bool simDeuteranopia;
uniform bool deuteranopia;
uniform float deuteranopiaStrength;

uniform bool simTritanopia;
uniform bool tritanopia;
uniform float tritanopiaStrength;


vec3 rgb2lms(vec3 color) {
    mat3 matrix = mat3(
        0.313990, 0.155372, 0.017752,
        0.639512, 0.757894, 0.109442,
        0.046497, 0.086701, 0.872569
    );

    vec3 lms = matrix * color;
    return lms;
}

vec3 lms2rgb(vec3 color) {
    mat3 matrix = mat3(
        5.47221206, -1.1252419,   0.02980165,
        -4.6419601,   2.29317094, -0.19318073,
        0.16963708, -0.1678952,   1.16364789
    );

    vec3 rgb = matrix * color;
    return rgb;
}


void simulateProtanopia(inout vec3 color) {
    vec3 lms = rgb2lms(color);

    mat3 protanMatrix  = mat3(
        0, 0, 0, 
        1.05118294, 1, 0,
        -0.05116099, 0, 1
    );

    vec3 lmsProtan = protanMatrix * lms;
    //vec3 lmsFinal = mix(lms, lmsProtan, 1.0);

    color = lms2rgb(lmsProtan);
    color = clamp(color, 0.0, 1.0);
}

void daltonizeProtanopia(inout vec3 color) {
    vec3 lms = rgb2lms(color);

    mat3 protMatrix = mat3(
        0, 0, 0, 
        1.05118294, 1, 0,
        -0.05116099, 0, 1
    );

    vec3 lmsProtan = protMatrix * lms;

    vec3 error = lms - lmsProtan;

    mat3 correction = mat3(
        0.0, 0.7, 0.7,
        0.0, 1.0, 0.0,
        0.0, 0.0, 1.0
    );

    vec3 lmsDaltonized = lms + correction * (protanopiaStrength * error);

    color = lms2rgb(lmsDaltonized);
    color = clamp(color, 0.0, 1.0);
}


void simulateDeuteranopia(inout vec3 color) {
    vec3 lms = rgb2lms(color);

    mat3 deuterMatrix  = mat3(
        1, 0.9513092, 0, 
        0, 0, 0,
        0, 0.04866992, 1
    );

    vec3 lmsDetuer = deuterMatrix * lms;
    //vec3 lmsFinal = mix(lms, lmsDetuer, 1.0);

    color = lms2rgb(lmsDetuer);
    color = clamp(color, 0.0, 1.0);
}

void daltonizeDeuteranopia(inout vec3 color) {
    vec3 lms = rgb2lms(color);

    mat3 deuterMatrix  = mat3(
        1, 0.9513092, 0, 
        0, 0, 0,
        0, 0.04866992, 1
    );

    vec3 lmsDetuer = deuterMatrix * lms;

    vec3 error = lms - lmsDetuer;

    mat3 correction = mat3(
        1.0, 0.0, 0.0,
        0.7, 0.0, 0.7,
        0.0, 0.0, 1.0
    );

    vec3 lmsDaltonized = lms + correction * (error * deuteranopiaStrength);

    color = lms2rgb(lmsDaltonized);
    color = clamp(color, 0.0, 1.0);
}


void simulateTritanopia(inout vec3 color) {
    vec3 lms = rgb2lms(color);

    mat3 tritMatrix  = mat3(
        1, 0, -0.86744736, 
        0, 1, 1.86727089,
        0, 0, 0
    );

    vec3 lmsTrit = tritMatrix * lms;
    //vec3 lmsFinal = mix(lms, lmsTrit, 1.0);

    color = lms2rgb(lmsTrit);
    color = clamp(color, 0.0, 1.0);
}

void daltonizeTritanopia(inout vec3 color) {
    vec3 lms = rgb2lms(color);

    mat3 tritMatrix = mat3(
        1, 0, -0.86744736, 
        0, 1, 1.86727089,
        0, 0, 0
    );

    vec3 lmsTrit = tritMatrix * lms;

    vec3 error = lms - lmsTrit;

    mat3 correction = mat3(
        1.0, 0.0, 0.0,
        0.0, 1.0, 0.0,
        0.7, 0.7, 0.0
    );

    vec3 lmsDaltonized = lms + correction * (error * tritanopiaStrength);

    color = lms2rgb(lmsDaltonized);
    color = clamp(color, 0.0, 1.0);
}



void main() {
    vec2 uv = TexCoord;
    vec3 color = texture(screenTex, uv).rgb;

    color = pow(color, vec3(2.2));

    if(protanopia)
        daltonizeProtanopia(color);
    if(simProtanopia)
        simulateProtanopia(color);

    if(deuteranopia)
        daltonizeDeuteranopia(color);
    if(simDeuteranopia)
        simulateDeuteranopia(color);

    if(tritanopia)
        daltonizeTritanopia(color);
    if(simTritanopia)
        simulateTritanopia(color);
   
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);
}