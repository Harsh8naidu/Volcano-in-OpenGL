#pragma once

#include "ModelLoader.h"
#include "MaterialLoader.h"
#include <SOIL/SOIL.h>
#include <nclgl/OGLRenderer.h>

class ObjModel {
public:
    ModelLoader* modelLoader;   // Model loader for handling the geometry (vertices, indices)
    MaterialLoader* materialLoader;   // Material loader for handling materials
    GLuint modelTextureID;   // Texture ID for the model
    GLuint modelVAO;         // Vertex Array Object for the model
	GLuint modelVBO;         // Vertex Buffer Object for the model
	GLuint modelEBO;         // Element Buffer Object for the model

    ObjModel(const std::string& objFilePath, const std::string& mtlFilePath) {
        // Load the model (geometry)
        modelLoader = new ModelLoader(objFilePath);

		// Load the materials (textures, colors, and other properties)
        materialLoader = new MaterialLoader();
        materialLoader->LoadMTL(mtlFilePath);

        // Create a VAO for this model
        glGenVertexArrays(1, &modelVAO);
        glBindVertexArray(modelVAO);

		// Create a VBO for the model vertices
		glGenBuffers(1, &modelVBO);
		glBindBuffer(GL_ARRAY_BUFFER, modelVBO);
		glBufferData(GL_ARRAY_BUFFER, modelLoader->vertices.size() * sizeof(Vertex), modelLoader->vertices.data(), GL_STATIC_DRAW);

		// Create an EBO for the model indices
		glGenBuffers(1, &modelEBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, modelLoader->indices.size() * sizeof(unsigned int), modelLoader->indices.data(), GL_STATIC_DRAW);

		// Define the vertex attributes (position, texture coordinates, normals)
		glEnableVertexAttribArray(0); // Position
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

		glEnableVertexAttribArray(1); // Texture coordinates
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));

		glEnableVertexAttribArray(2); // Normals
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

		glBindVertexArray(0); // Unbind the VAO after setup

        // Generate and load texture based on material
        LoadModelTexture();
    }

    ~ObjModel() {
        delete modelLoader;
        delete materialLoader;
        glDeleteVertexArrays(1, &modelVAO);  // Clean up the VAO
		glDeleteBuffers(1, &modelVBO);       // Clean up the VBO
		glDeleteBuffers(1, &modelEBO);       // Clean up the EBO
    }

    void LoadModelTexture() {
        // load the texture for the model using the material info
        if (!materialLoader->GetMaterials().empty()) {
            const Material& material = materialLoader->GetMaterials()[0]; // Assume the first material for now
            modelTextureID = SOIL_load_OGL_texture(material.diffuseTexture.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
        }
    }

    void Render(Shader* shader, Matrix4& modelMat, Matrix4& viewMat, Matrix4& projMat) {
		glUseProgram(shader->GetProgram()); 

        // Bind the texture for the model (if any)
		glBindTexture(GL_TEXTURE_2D, modelTextureID);

        // Set the model matrix uniform in the shader
        GLint modelMatrixLoc = glGetUniformLocation(shader->GetProgram(), "modelMatrix");
        glUniformMatrix4fv(modelMatrixLoc, 1, GL_TRUE, modelMat.values);

        // Set the view and projection matrices
        GLint viewMatrixLoc = glGetUniformLocation(shader->GetProgram(), "viewMatrix");
        glUniformMatrix4fv(viewMatrixLoc, 1, GL_TRUE, viewMat.values);  // Assuming you have a view matrix from the camera

        GLint projMatrixLoc = glGetUniformLocation(shader->GetProgram(), "projMatrix");
        glUniformMatrix4fv(projMatrixLoc, 1, GL_TRUE, projMat.values);  // Assuming you have a projection matrix

        // Set the texture unit
        GLint textureLoc = glGetUniformLocation(shader->GetProgram(), "modelTexture");
        glUniform1i(textureLoc, 0);  // Texture unit 0

        // Bind the VAO and draw the model
        glBindVertexArray(modelVAO);

		// Ensuring the indices exist before rendering/drawing I was getting a crash without this check LOL :)
        if (!modelLoader->indices.empty()) {
            // Log all vertex positions before drawing
            std::cout << "Logging vertex positions:\n";
            for (const auto& vertex : modelLoader->vertices) {
                std::cout << "Position: (" << vertex.position.x << ", "
                    << vertex.position.y << ", "
                    << vertex.position.z << ")\n";
            }

            glDrawElements(GL_TRIANGLES, modelLoader->indices.size(), GL_UNSIGNED_INT, 0);
        }
        else {
			std::cerr << "Error: No indices found for the model" << std::endl;
        }
        
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glUseProgram(0);
    }
};
