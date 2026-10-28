// ----- Definitions ----- //

// Must match NR_POINT_LIGHTS / NR_DIR_LIGHTS in LightSystem.cpp
#define NR_POINT_LIGHTS 256
#define NR_DIR_LIGHT 8

const float constant = 1.f;
const float linear = .35f;
const float quadratic = .44f;

const float lightAmbient = .2f;
const float lightDiffuse = .5f;
const float lightSpecular = 1.f;

const float PI = 3.14159265359;
const float PI2 = 6.28318530718;