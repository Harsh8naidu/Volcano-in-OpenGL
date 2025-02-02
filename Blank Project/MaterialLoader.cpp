#include "MaterialLoader.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <SOIL/SOIL.h>

Material::Material()
    : shininess(0.0f),
    ambient(Vector3(0.0f, 0.0f, 0.0f)),
    specular(Vector3(0.0f, 0.0f, 0.0f)),
    emission(Vector3(0.0f, 0.0f, 0.0f)),
    ior(1.0f),
    dissolveFactor(1.0f),
    illuminationModel(0),
    diffuseTexture(""),
    bumpTexture(""),
    textureID(0) {}


bool MaterialLoader::LoadMTL(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filePath << std::endl;
        return false;
    }

    Material currentMaterial;
    std::string line;

    while (std::getline(file, line)) {
        std::istringstream lineStream(line);
        std::string type;
        lineStream >> type;

        // Ignore comments
        if (type.empty() || type[0] == '#') continue;

        if (type == "newmtl") { // New material
            if (!currentMaterial.name.empty()) {
                // Save the previous material before starting a new one
                materials.push_back(currentMaterial);
            }
            lineStream >> currentMaterial.name; // Read the material name
        }
        else if (type == "Ns") { // Shininess
            lineStream >> currentMaterial.shininess;
        }
        else if (type == "Ka") { // Ambient color
            lineStream >> currentMaterial.ambient.x >> currentMaterial.ambient.y >> currentMaterial.ambient.z;
        }
        else if (type == "Ks") { // Specular color
            lineStream >> currentMaterial.specular.x >> currentMaterial.specular.y >> currentMaterial.specular.z;
        }
        else if (type == "Ke") { // Emission color
            lineStream >> currentMaterial.emission.x >> currentMaterial.emission.y >> currentMaterial.emission.z;
        }
        else if (type == "Ni") { // Index of Refraction (IOR)
            lineStream >> currentMaterial.ior;
        }
        else if (type == "d") { // Dissolve factor
            lineStream >> currentMaterial.dissolveFactor;
        }
        else if (type == "illum") { // Illumination model
            lineStream >> currentMaterial.illuminationModel;
        }
        else if (type == "map_Kd") { // Diffuse texture map
            lineStream >> currentMaterial.diffuseTexture;

			// Loading the textures immediately and storing the texture ID in the material
			currentMaterial.textureID = SOIL_load_OGL_texture(
                currentMaterial.diffuseTexture.c_str(),
                SOIL_LOAD_AUTO, 
                SOIL_CREATE_NEW_ID, 
                SOIL_FLAG_MIPMAPS
            );
        }
        else if (type == "map_Bump") { // Bump map
            lineStream >> currentMaterial.bumpTexture;
        }
    }

    if (!currentMaterial.name.empty()) {
        materials.push_back(currentMaterial);
    }

    file.close();
    return true;
}

Material* MaterialLoader::GetMaterialByName(const std::string& materialName) {
    for (auto& material : materials) {
        if (material.name == materialName) {
            return &material;
        }
    }
    return nullptr; // Return nullptr if no material matches the name
}