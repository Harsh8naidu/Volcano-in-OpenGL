#include "ModelLoader.h"

#include <fstream>
#include <sstream>
#include <iostream>

ModelLoader::ModelLoader(const std::string& filePath)
{
	LoadOBJ(filePath);
}

bool ModelLoader::LoadOBJ(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filePath << std::endl;
        return false;
    }

    std::vector<Vector3> positions;
	std::vector<Vector2> texCoords;
	std::vector<Vector3> normals;
    std::vector<unsigned int> positionIndices, texCoordIndices, normalIndices;

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream lineStream(line);
        std::string type;
        lineStream >> type;

        if (type == "v") { // Vertex position
            Vector3 position;
            lineStream >> position.x >> position.y >> position.z;
			positions.push_back(position);
        }
        else if (type == "vt") { // Texture coordinate
            Vector2 texCoord;
            lineStream >> texCoord.x >> texCoord.y;
            texCoords.push_back(texCoord);
        }
        else if (type == "vn") { // Normal
            Vector3 normal;
            lineStream >> normal.x >> normal.y >> normal.z;
            normals.push_back(normal);
        }
        else if (type == "f") { // Face indices
            unsigned int posIdx[3], texIdx[3], normIdx[3];
            for (int i = 0; i < 3; ++i) {
                char slash;
                lineStream >> posIdx[i] >> slash >> texIdx[i] >> slash >> normIdx[i];
                posIdx[i]--; texIdx[i]--; normIdx[i]--; // OBJ format is 1-based, adjust to 0-based
                positionIndices.push_back(posIdx[i]);
                texCoordIndices.push_back(texIdx[i]);
                normalIndices.push_back(normIdx[i]);
            }
        }
    }

    file.close();

    // Combine indices into vertices
    for (size_t i = 0; i < positionIndices.size(); ++i) {
        Vertex vertex = {};
        vertex.position = positions[positionIndices[i]];
        vertex.texCoord = texCoords[texCoordIndices[i]];
        vertex.normal = normals[normalIndices[i]];
        vertices.push_back(vertex);
        indices.push_back(static_cast<unsigned int>(i)); // Directly map index
    }

    return true;
}