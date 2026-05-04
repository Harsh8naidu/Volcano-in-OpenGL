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

// Please find the screenshots and link to the youtube video in a folder 
// named "Youtube Video and Screenshots" in the root directory of the project

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	// Load .obj files
	testObjModel = new ObjModel(MODELDIR "volcano_model.obj", MATERIALDIR "volcano_model.mtl");

	// Load the meshes
	quad = Mesh::GenerateQuad();

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

    // TODO: use a vector array to push all textures and check for any that failed to load in a loop instead of individually
	if (!earthTex || !earthBump || !lavaTex || !volcanoTexture || !cubeMap || !heightMapTex || !heightMapTex2) {
		return;
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

	if (!reflectShader->LoadSuccess() || 
        !skyboxShader->LoadSuccess() || 
        !lightShader->LoadSuccess() || 
        !modelShader->LoadSuccess() || 
        !objModelShader->LoadSuccess() ||
        !terrainShader->LoadSuccess()) {
		return;
    }

	Vector3 hMapSize = heightMap->GetHeightmapSize();

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

	straightMoveDirection = Vector3(0, 0, 1); // Moving along negative Z-axis

	elapsedTime = 0.0f;
	sceneChanged = false;
	isLavaFlowing = true;

	// Set up OpenGL settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	lavaRotate = 0.0f;
	lavaCycle = 0.0f;
	init = true;
}

Renderer::~Renderer(void) {
	// Cleanups
	delete camera;
	delete heightMap;
	delete quad;

	// shaders cleanup
	delete reflectShader;
	delete skyboxShader;
	delete lightShader;
	delete modelShader;
	delete flashShader;
	delete objModelShader;

	// light cleanup
	delete sceneLight;

	// models cleanup
	delete volcanoMesh;
	delete bonyWallMesh;
	delete monsterMesh;
	delete volcanicRockMesh;

	// textures cleanup
	glDeleteTextures(1, &lavaTex);
	glDeleteTextures(1, &earthTex);
	glDeleteTextures(1, &earthBump);

	// objects cleanup
	delete testObjModel;
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

    // Assign sampler units explicitly
    glUniform1i(glGetUniformLocation(objModelShader->GetProgram(),
        "diffuseTexture"), 0);
    glUniform1i(glGetUniformLocation(objModelShader->GetProgram(),
        "roughnessTexture"), 1);
    glUniform1i(glGetUniformLocation(objModelShader->GetProgram(),
        "metallicTexture"), 2);

	modelMatrix = Matrix4::Translation(Vector3(10000.0f, 90.0f, 16000.0f)) *
		Matrix4::Scale(Vector3(5.0f, 5.0f, 5.0f)) *
		Matrix4::Rotation(90, Vector3(0, 1, 0)) * 
        Matrix4::Rotation(3, Vector3(1, 0, 0));

    UpdateShaderMatrices();

    // Camera position
    Vector3 camPos = camera->GetPosition();
    glUniform3fv(glGetUniformLocation(objModelShader->GetProgram(), "cameraPos"), 1, (float*)&camPos);

    // Upload lights
    int lightCount = (int)sceneLights.size();
    glUniform1i(glGetUniformLocation(objModelShader->GetProgram(), "lightCount"), lightCount);

    for (int i = 0; i < lightCount; i++) {
        std::string base = "lights[" + std::to_string(i) + "]";
        Vector3 lPos = sceneLights[i]->GetPosition();
        Vector4 lCol = sceneLights[i]->GetColour();
        float lRadius = sceneLights[i]->GetRadius();
        glUniform3fv(glGetUniformLocation(objModelShader->GetProgram(), (base + ".position").c_str()), 1, (float*)&lPos);
        glUniform4fv(glGetUniformLocation(objModelShader->GetProgram(), (base + ".color").c_str()), 1, (float*)&lCol);
        glUniform1f(glGetUniformLocation(objModelShader->GetProgram(), (base + ".radius").c_str()), lRadius);
    }

    // Iterate through all the material ranges, then bind and draw them
    const std::vector<MaterialRange>& materialRange = testObjModel->GetMaterialRanges();

    glBindVertexArray(testObjModel->modelVAO); // Bind the VAO

    for (const MaterialRange& range : materialRange) {
        const Material& mat = testObjModel->GetMaterialByName(range.materialName);

        // Bind the material properties to the shader
        glUniform1f(glGetUniformLocation(objModelShader->GetProgram(), "shininess"), mat.shininess);
        glUniform3fv(glGetUniformLocation(objModelShader->GetProgram(), "ambient"), 1, (float*)&mat.ambient);
        glUniform3fv(glGetUniformLocation(objModelShader->GetProgram(), "specular"), 1, (float*)&mat.specular);
        glUniform3fv(glGetUniformLocation(objModelShader->GetProgram(), "emission"), 1, (float*)&mat.emission);
        glUniform1f(glGetUniformLocation(objModelShader->GetProgram(), "ior"), mat.ior);
        glUniform1f(glGetUniformLocation(objModelShader->GetProgram(), "dissolveFactor"), mat.dissolveFactor);
        glUniform1i(glGetUniformLocation(objModelShader->GetProgram(), "illuminationModel"), mat.illuminationModel);

        // Texture binding helper function
        auto bindTexture = [&](GLuint texture, GLenum textureUnit, const char* uniformName) {
            if (texture > 0) {
                glActiveTexture(textureUnit);
                glBindTexture(GL_TEXTURE_2D, texture);
                GLint loc = glGetUniformLocation(objModelShader->GetProgram(), uniformName);
                if (loc != -1) {
                    glUniform1i(loc, textureUnit - GL_TEXTURE0);
                }
            }
        };

        // Bind textures
        bindTexture(mat.diffuseTexture, GL_TEXTURE0, "diffuseTexture");
        bindTexture(mat.roughnessTexture, GL_TEXTURE1, "roughnessTexture");
        bindTexture(mat.metallicTexture, GL_TEXTURE2, "metallicTexture");

        if (mat.diffuseTexture == 0) {
            std::cout << "Material " << range.materialName
                << " has no diffuse texture. Using fallback.\n";
        }

        // Draw the model
        glDrawElements(GL_TRIANGLES, range.indexCount, GL_UNSIGNED_INT, (void*)(range.startIndex * sizeof(unsigned int)));
    }

    glBindVertexArray(0);

    // Reset texture state
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::DrawSkybox() {
	// Draw the skybox
	glDepthMask(GL_FALSE);

	BindShader(skyboxShader);
	UpdateShaderMatrices();

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);

	quad->Draw();

	glDepthMask(GL_TRUE);
}

void Renderer::DrawHeightmap() {
	// Draw the heightmap
	BindShader(terrainShader);

    // Camera position
    glUniform3fv(glGetUniformLocation(terrainShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());

    // Upload all lights to shader
    int lightCount = (int)sceneLights.size();
    glUniform1i(glGetUniformLocation(terrainShader->GetProgram(), "lightCount"), lightCount);
    
    for (int i = 0; i < lightCount; i++) {
        std::string base = "lights[" + std::to_string(i) + "]";
        Vector3 lPos = sceneLights[i]->GetPosition();
        Vector4 lCol = sceneLights[i]->GetColour();
        float lRadius = sceneLights[i]->GetRadius();
        glUniform3fv(glGetUniformLocation(terrainShader->GetProgram(), (base + ".position").c_str()), 1, (float*)&lPos);
        glUniform4fv(glGetUniformLocation(terrainShader->GetProgram(), (base + ".color").c_str()), 1, (float*)&lCol);
        glUniform1f(glGetUniformLocation(terrainShader->GetProgram(), (base + ".radius").c_str()), lRadius);
    }

    float scaleHMapX = 100.0f; 
    float scaleHMapY = 1.0f; 
    float scaleHMapZ = 100.0f;

    Vector3 hMapSize = heightMap->GetHeightmapSize();
    Vector3 nMapSize = heightMap2->GetHeightmapSize();

    // Scale noiseHeightMap to match heightMap's world size
    float noiseScaleX = scaleHMapX * (hMapSize.x / nMapSize.x);
    float noiseScaleZ = scaleHMapZ * (hMapSize.z / nMapSize.z);

    // ---------- First Terrain ----------
    // UV scale for diffuse texturing
    glUniform2f(glGetUniformLocation(terrainShader->GetProgram(), "uvScale"), 0.01f, 0.01f);

    glUniform1i(glGetUniformLocation(terrainShader->GetProgram(), "diffuseTex"), 0);
    glUniform1i(glGetUniformLocation(terrainShader->GetProgram(), "heightMap"), 1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, volcanoTexture);
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, heightMapTex2);
    

    modelMatrix = Matrix4::Translation(Vector3(0, 20.0f, 0)) *
        Matrix4::Scale(Vector3(scaleHMapX, 12.0f, scaleHMapZ)) *
        Matrix4::Rotation(-6.2f, Vector3(0, 1, 0));
	UpdateShaderMatrices();
	heightMap->Draw();

    // ---------- Second Terrain ----------
    // UV scale for diffuse texturing
    glUniform2f(glGetUniformLocation(terrainShader->GetProgram(), "uvScale"), 0.25f, 0.25f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, earthTex);

    // Reuse same heightmap texture unless another is intended
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, heightMapTex);

    modelMatrix = Matrix4::Translation(Vector3(0.0f, 360.0f, 0.0f)) *
        Matrix4::Scale(Vector3(noiseScaleX, 8.0f, noiseScaleZ)) *
        Matrix4::Rotation(-6.2f, Vector3(0, 1, 0));
    UpdateShaderMatrices();
    heightMap2->Draw();
    
    // Unbind textures
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
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