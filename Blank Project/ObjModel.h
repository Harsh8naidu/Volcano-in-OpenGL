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
	int indices;			 // Number of indices in the model

	Material* material; // store the material properties

    ObjModel(const std::string& objFilePath, const std::string& mtlFilePath) {
        // Load the model (geometry)
        modelLoader = new ModelLoader(objFilePath);

		// Load the material
		materialLoader = new MaterialLoader();
		materialLoader->LoadMTL(mtlFilePath);

		// Get the material
		material = materialLoader->GetMaterialByName("default_material_name");

		// Initialize the indices
		indices = modelLoader->indices.size();

        // Create and bind VAO
        glGenVertexArrays(1, &modelVAO); // Generate the VAO
		glBindVertexArray(modelVAO); // Bind the VAO

        // Load the vertices and indices into VBOs
        glGenBuffers(1, &modelVBO); // Generate the VBO for vertices
        glBindBuffer(GL_ARRAY_BUFFER, modelVBO); // Bind it to the buffer target
        glBufferData(GL_ARRAY_BUFFER, modelLoader->vertices.size() * sizeof(Vertex), &modelLoader->vertices[0], GL_STATIC_DRAW); // Load data

        // Generate EBO for indices
        glGenBuffers(1, &modelEBO);  // EBO for indices
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, modelLoader->indices.size() * sizeof(unsigned int), &modelLoader->indices[0], GL_STATIC_DRAW);
		
		
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, v_position)); // Position
        glEnableVertexAttribArray(0); // Enable the attribute for position at 0
		
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, v_texCoord)); // Texture coordinates
        glEnableVertexAttribArray(1); // Enable the attribute for texture coordinates 1
		
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, v_normal)); // Normal
        glEnableVertexAttribArray(2); // Enable the attribute for normal at 2

        // Now, unbind the buffers (good practice to unbind after setup)
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		// Unbind the VAO
		glBindVertexArray(0);
    }

    ~ObjModel() {
        delete modelLoader;
        delete materialLoader;
        glDeleteVertexArrays(1, &modelVAO);  // Clean up the VAO
		glDeleteBuffers(1, &modelVBO);       // Clean up the VBO
		glDeleteBuffers(1, &modelEBO);       // Clean up the EBO
    }

    GLuint getVAO() const {
        return modelVAO;
    }

	GLuint getEBO() const {
		return modelEBO;
	}

    unsigned int getIndexCount() const {
        return static_cast<unsigned int>(indices);
    }
};
