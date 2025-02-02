#pragma once

#include <vector>
#include <string>
#include <nclgl/Vector3.h>
#include <nclgl/Vector2.h>
#include <glad/glad.h>

// Structure to hold vertex data
struct Vertex {
    Vector3 position;
    Vector2 texCoord;
    Vector3 normal;
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

private:
    GLuint textureID;
    GLuint textureLocation;

};