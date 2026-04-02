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
    Vector3 lightDirection;
    float ior;   // Ni
    float dissolveFactor; // d
    int illuminationModel; // illum
    GLuint diffuseTexture;  // map_Kd
	GLuint roughnessTexture; // map_Ns
	GLuint metallicTexture; // map_refl

    Material();
};

struct MaterialRange {
    std::string materialName = "";  // 
    int startIndex = 0;             //
    int indexCount = 0;             //
};

class MaterialLoader {
public:
    bool LoadMTL(const std::string& filePath);

    const std::vector<Material>& GetMaterials() const { return materials; }
    const Material& GetMaterialByName(const std::string& name) const {
        for (const Material& mat : materials) {
            if (mat.name == name) {
                return mat;
            }
        }
        std::cerr << "Warning Material '" << name << "' not found! Using first." << std::endl;
        static Material defaultMaterial; // static so reference stays valid
        return defaultMaterial;
    }

private:
    std::vector<Material> materials;

    std::string materialDir = "C:/Users/harsh/source/repos/My_New_Assignment/Advanced-Graphics-For-Games-master/Materials/";
};