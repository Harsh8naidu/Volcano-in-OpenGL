#pragma once

#include <nclgl/Vector3.h>
#include "glad/glad.h"

#include <string>
#include <vector>

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
    GLuint bumpTexture; // map_Bump

    Material();
};

class MaterialLoader {
public:
    bool LoadMTL(const std::string& filePath);
    Material* GetMaterialByName(const std::string& materialName);
    const std::vector<Material>& GetMaterials() const { return materials; }

private:
    std::vector<Material> materials;
};