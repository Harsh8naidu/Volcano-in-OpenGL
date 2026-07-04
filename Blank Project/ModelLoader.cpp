#include "ModelLoader.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <tuple>

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

    std::string currentMaterialName = "";
    int currentStartIndex = 0;

    std::map<std::tuple<int, int, int>, unsigned int> vertexMap;

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
        else if (type == "usemtl") {
            // Save previous material range before switching
            if (!currentMaterialName.empty()) {
                MaterialRange range;
                range.materialName = currentMaterialName;
                range.startIndex = currentStartIndex;
                range.indexCount = (int)indices.size() - currentStartIndex;
                materialRanges.push_back(range);

                std::cout << "Material Range Added: " << range.materialName
                    << " | Start: " << range.startIndex
                    << " | Count: " << range.indexCount << std::endl;
            }

            // Start tracking new material
            lineStream >> currentMaterialName;
            currentStartIndex = (int)indices.size();
        }
        else if (type == "f") { // Face indices
            std::string vertexData;
            std::vector<unsigned int> faceIndices;

            while (lineStream >> vertexData) {
                unsigned int posIdx, texIdx, normIdx;
                sscanf(vertexData.c_str(), "%u/%u/%u", &posIdx, &texIdx, &normIdx);
                posIdx--; texIdx--; normIdx--;

                // Check if this vertex combination already exists
                auto key = std::make_tuple(posIdx, texIdx, normIdx);
                auto it = vertexMap.find(key);

                if (it != vertexMap.end()) {
                    faceIndices.push_back(it->second);
                }
                else {
                    // Add new unique vertex
                    unsigned int newIndex = (unsigned int)vertices.size();
                    vertexMap[key] = newIndex;

                    Vertex vertex = {};
                    vertex.v_position = positions[posIdx];
                    vertex.v_texCoord.x = texCoords[texIdx].x;
                    vertex.v_texCoord.y = 1.0f - texCoords[texIdx].y; // flip V
                    vertex.v_normal   = normals[normIdx];
                    vertices.push_back(vertex);

                    faceIndices.push_back(newIndex);
                }
            }

            // Triangulate face (can handle both triangles and quads)
            for (size_t i = 1; i + 1 < faceIndices.size(); ++i) {
                indices.push_back(faceIndices[0]);
                indices.push_back(faceIndices[i]);
                indices.push_back(faceIndices[i + 1]);
            }
        }
    }

    // Save the last material range
    if (!currentMaterialName.empty()) {
        MaterialRange range;
        range.materialName = currentMaterialName;
        range.startIndex = currentStartIndex;
        range.indexCount = (int)indices.size() - currentStartIndex;
        materialRanges.push_back(range);
    }

    file.close();

    return true;
}