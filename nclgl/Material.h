#pragma once

#include <string>
#include "Vector3.h"
#include "glad/glad.h"

struct Material {
    std::string name;
    float shininess; // Ns
    Vector3 ambient;  // Ka
    Vector3 specular; // Ks
    Vector3 emission; // Ke
    float ior;   // Ni
    float dissolveFactor; // d
    int illuminationModel; // illum
    GLuint diffuseTexture;  // map_Kd
    GLuint roughnessTexture; // map_Ns
    GLuint metallicTexture; // map_refl

    Material();
};