#pragma once

#include <nclgl/Vector3.h>
#include "glad/glad.h"
#include <nclgl/Material.h>
#include <string>
#include <vector>

struct MaterialRange {
    std::string materialName = "";  // 
    int startIndex = 0;             //
    int indexCount = 0;             //
};

class MaterialLoader {
public:
    MaterialLoader(const std::string& filePath);
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