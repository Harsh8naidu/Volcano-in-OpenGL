#include "Renderer.h"
#include "../nclgl/Camera.h"
#include "../nclgl/Light.h"
#include "../nclgl/HeightMap.h"
#include "../nclgl/MeshAnimation.h"
#include "../nclgl/MeshMaterial.h"
#include "Volcano.h"
#include "BonyWall.h"
#include "Monster.h"
#include "../nclgl/Math_Utility.h"
#include "VolcanicRock.h"

#include <filesystem>
#include <iostream>

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	// Load .obj files
	volcanoModel = new ObjModel(MODELDIR "volcano_model.obj", MATERIALDIR "volcano_model.mtl");

	// Load the meshes
	quad = Mesh::GenerateQuad();
    
    // Push meshes into a vector array
    meshes.push_back(quad);

	// Load the heightmaps
	heightMap = new HeightMap(TEXTUREDIR "Heightmap_01_Mountain.jpg");
	heightMap2 = new HeightMap(TEXTUREDIR "Heightmap_Dunes.jpg");

	// Load the textures
    heightMapTex = SOIL_load_OGL_texture(TEXTUREDIR "noise.png", SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_TEXTURE_REPEATS);
    heightMapTex2 = SOIL_load_OGL_texture(TEXTUREDIR "Heightmap_Dunes.png", SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_TEXTURE_REPEATS);
	lavaTex = SOIL_load_OGL_texture(TEXTUREDIR "lava_texture.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_TEXTURE_REPEATS);
	earthTex = SOIL_load_OGL_texture(TEXTUREDIR "volcanic_rock.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_TEXTURE_REPEATS);
	volcanoTexture = SOIL_load_OGL_texture(TEXTUREDIR "lava_texture.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_TEXTURE_REPEATS);
    
	// Load the cubemap
	cubeMap = SOIL_load_OGL_cubemap(
		TEXTUREDIR "right.jpg", TEXTUREDIR "left.jpg",
		TEXTUREDIR "top.jpg", TEXTUREDIR "bottom.jpg",
		TEXTUREDIR "front.jpg", TEXTUREDIR "back.jpg",
		SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_TEXTURE_REPEATS
	);

    // Push textures into a vector array
    textures.push_back(heightMapTex);
    textures.push_back(heightMapTex2);
    textures.push_back(lavaTex);
    textures.push_back(earthTex);
    textures.push_back(volcanoTexture);
    textures.push_back(cubeMap);

    // Check if any texture failed to load
    for (GLuint tex : textures) {
        if (!tex) {
            std::cerr << "Failed to load a texture. Check the file paths and ensure the files exist.\n";
            return;
        }
    }

	// Set the texture to repeat
	SetTextureRepeating(earthTex, true);
	SetTextureRepeating(lavaTex, true);

	// Load the shaders
	objModelShader = new Shader("modelVertexShader.glsl", "modelFragmentShader.glsl");
	modelShader = new Shader("SceneVertex.glsl", "SceneFragment.glsl");
	reflectShader = new Shader("reflectVertex.glsl", "reflectFragment.glsl");
	skyboxShader = new Shader("skyboxVertex.glsl", "skyboxFragment.glsl");
	lightShader = new Shader("PerPixelVertex.glsl", "PerPixelFragment.glsl");
    terrainShader = new Shader("terrainVertexShader.glsl", "terrainFragmentShader.glsl");

    shaders.push_back(objModelShader);
    shaders.push_back(modelShader);
    shaders.push_back(reflectShader);
    shaders.push_back(skyboxShader);
    shaders.push_back(lightShader);
    shaders.push_back(terrainShader);

    // Check if any shader failed to load
    for (Shader* shader : shaders) {
        if (!shader->LoadSuccess()) {
            std::cerr << "Failed to load a shader. Check the file paths and ensure the files exist.\n";
            return;
        }
    }

	// Set up the camera and light
	camera = new Camera(-10.0f, 190.0f, Vector3(10000.0f, 3000.0f, 16000.0f)); // -0.88f
	camera->SetPitch(5.0f);
	camera->SetYaw(90.0f);

	// Lights for the scene
    sceneLights.push_back(new Light(Vector3(15000.0f, 3000.0f, 25000.0f), Vector4(1.0f, 0.95f, 0.6f, 1.0f), 5000.0f));
    sceneLights.push_back(new Light(Vector3(25000.0f, 3000.0f, 22000.0f), Vector4(1.0f, 0.95f, 0.6f, 1.0f), 5000.0f));
    sceneLights.push_back(new Light(Vector3(30000.0f, 3000.0f, 25000.0f), Vector4(1.0f, 0.95f, 0.6f, 1.0f), 5000.0f));
    sceneLights.push_back(new Light(Vector3(20000.0f, 2000.0f, 30000.0f), Vector4(1.0f, 0.95f, 0.6f, 1.0f), 5000.0f));

	// Set up the matrices
	projMatrix = Matrix4::Perspective(10.0f, 90000.0f, (float)width / (float)height, 45.0f);

	// Set up the root node and add the models
	rootNode = new SceneNode();
	//rootNode->AddChild(new Volcano(volcanoMesh, volcanoTexture));

	// Set up OpenGL settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	init = true;
}

Renderer::~Renderer(void) {
	// Cleanups
	delete camera;
	delete heightMap;
	delete quad;

	// shaders cleanup
    for (Shader* shader : shaders) {
        delete shader;
    }

	// light cleanup
    for (Light* light : sceneLights) {
        delete light;
    }

	// models cleanup
    for (Mesh* mesh : meshes) {
        delete mesh;
    }

	// textures cleanup
    for (GLuint tex : textures) {
        glDeleteTextures(1, &tex);
    }

	// objects cleanup
    delete volcanoModel;
}

void Renderer::UpdateScene(float dt) {
	//Update the camera
	camera->UpdateCamera(dt);
	viewMatrix = camera->BuildViewMatrix();
	//rootNode->Update(dt);
}

void Renderer::RenderScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
    DrawSkybox();
	DrawHeightmap();
	DrawVolcano();
	DrawNode(rootNode);
}

void Renderer::DrawVolcano() {
	BindShader(objModelShader);

	modelMatrix = Matrix4::Translation(Vector3(10000.0f, 90.0f, 16000.0f)) *
		Matrix4::Scale(Vector3(5.0f, 5.0f, 5.0f)) *
		Matrix4::Rotation(90, Vector3(0, 1, 0)) * 
        Matrix4::Rotation(3, Vector3(1, 0, 0));

    UpdateShaderMatrices();

    Vector3 camPos = camera->GetPosition();
    glUniform3fv(glGetUniformLocation(objModelShader->GetProgram(), "cameraPos"), 1, (float*)&camPos);

    SetShaderLights(sceneLights);

    glBindVertexArray(volcanoModel->modelVAO); // Bind the VAO
    for (const MaterialRange& range : volcanoModel->GetMaterialRanges()) {
        BindMaterial(volcanoModel->GetMaterialByName(range.materialName));
        glDrawElements(GL_TRIANGLES, range.indexCount, GL_UNSIGNED_INT, (void*)(range.startIndex * sizeof(unsigned int)));
    }
    glBindVertexArray(0);
}

void Renderer::DrawSkybox() {
	// Draw the skybox
	glDepthMask(GL_FALSE);

	BindShader(skyboxShader);
	UpdateShaderMatrices();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);
    glUniform1i(glGetUniformLocation(skyboxShader->GetProgram(), "cubeTex"), 0);

	quad->Draw();

	glDepthMask(GL_TRUE);
}

void Renderer::DrawHeightmap() {
	// Draw the heightmap
	BindShader(terrainShader);

    glUniform3fv(glGetUniformLocation(terrainShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());

    SetShaderLights(sceneLights);

    float scaleHMapX = 50.0f; 
    float scaleHMapY = 1.0f; 
    float scaleHMapZ = 50.0f;

    Vector3 hMapSize = heightMap->GetHeightmapSize();
    Vector3 nMapSize = heightMap2->GetHeightmapSize();

    // Scale noiseHeightMap to match heightMap's world size
    float noiseScaleX = scaleHMapX * (hMapSize.x / nMapSize.x);
    float noiseScaleZ = scaleHMapZ * (hMapSize.z / nMapSize.z);

    // ---------- First Terrain ----------
    glUniform2f(glGetUniformLocation(terrainShader->GetProgram(), "uvScale"), 0.01f, 0.01f);
    glUniform1i(glGetUniformLocation(terrainShader->GetProgram(), "diffuseTex"), 0);
    glUniform1i(glGetUniformLocation(terrainShader->GetProgram(), "heightMap"), 1);
    BindTexture(volcanoTexture, GL_TEXTURE0, "diffuseTex");
    BindTexture(heightMapTex2, GL_TEXTURE1, "heightMap");
    
    modelMatrix = Matrix4::Translation(Vector3(0, 20.0f, 0)) *
        Matrix4::Scale(Vector3(scaleHMapX, 12.0f, scaleHMapZ)) *
        Matrix4::Rotation(-6.2f, Vector3(0, 1, 0));
	UpdateShaderMatrices();
	heightMap->Draw();

    // ---------- Second Terrain ----------
    // UV scale for diffuse texturing
    glUniform2f(glGetUniformLocation(terrainShader->GetProgram(), "uvScale"), 0.25f, 0.25f);
    BindTexture(earthTex, GL_TEXTURE0, "diffuseTex");
    BindTexture(heightMapTex, GL_TEXTURE1, "heightMap");

    modelMatrix = Matrix4::Translation(Vector3(0.0f, 360.0f, 0.0f)) *
        Matrix4::Scale(Vector3(noiseScaleX, 8.0f, noiseScaleZ)) *
        Matrix4::Rotation(-6.2f, Vector3(0, 1, 0));
    UpdateShaderMatrices();
    heightMap2->Draw();
}

void Renderer::DrawNode(SceneNode* n) {
	// Draw all the children of the node
	BindShader(modelShader);
	UpdateShaderMatrices();

	if (n->GetMesh()) {
		Matrix4 model = n->GetWorldTransform() * Matrix4::Scale(n->GetModelScale());
		glUniformMatrix4fv(glGetUniformLocation(modelShader->GetProgram(), "modelMatrix"), 1, false, model.values);
		glUniform4fv(glGetUniformLocation(modelShader->GetProgram(), "nodeColour"), 1, (float*)&n->GetColour());
		modelTexture = n->GetTexture();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, modelTexture);
		glUniform1i(glGetUniformLocation(modelShader->GetProgram(), "useTexture"), modelTexture);
	}

	n->Draw(*this);

	for (vector<SceneNode*>::const_iterator i = n->GetChildIteratorStart();
		i != n->GetChildIteratorEnd(); ++i) {
		DrawNode(*i);
	}
}