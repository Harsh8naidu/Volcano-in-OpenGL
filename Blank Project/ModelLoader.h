#pragma once

#include <vector>
#include "MaterialLoader.h"
#include <string>
#include <nclgl/Vector3.h>
#include <nclgl/Vector2.h>
#include <glad/glad.h>

// Structure to hold vertex data
struct Vertex {
    Vector3 v_position;
    Vector2 v_texCoord;
    Vector3 v_normal;
};

// ModelLoader class to handle loading the OBJ file
class ModelLoader {
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    ModelLoader() = default;

    ModelLoader(const std::string& filePath);

    // Loads the OBJ file from the given file path
    bool LoadOBJ(const std::string& filePath);

    std::vector<MaterialRange> materialRanges;

    const std::vector<MaterialRange>& GetMaterialRanges() const { return materialRanges; }

private:
    GLuint textureID;
    GLuint textureLocation;
};