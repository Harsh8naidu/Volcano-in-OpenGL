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

// Please find the screenshots and link to the youtube video in a folder 
// named "Youtube Video and Screenshots" in the root directory of the project

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	// Load the meshes
	quad = Mesh::GenerateQuad();
	volcanoMesh = Mesh::LoadFromMeshFile("Volcano.msh");
	bonyWallMesh = Mesh::LoadFromMeshFile("BonyWall.msh");
	monsterMesh = Mesh::LoadFromMeshFile("Role_T.msh");
	volcanicRockMesh = Mesh::LoadFromMeshFile("VolcanicRock.msh");

	// Load the heightmaps
	heightMap = new HeightMap(TEXTUREDIR "volcano_heightmap.png");
	noiseHeightMap = new HeightMap(TEXTUREDIR "noise.png");

	// Load the textures
	lavaTex = SOIL_load_OGL_texture(TEXTUREDIR "lava_texture.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	earthTex = SOIL_load_OGL_texture(TEXTUREDIR "volcanic_rock.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	earthBump = SOIL_load_OGL_texture(TEXTUREDIR "Barren RedsDOT3.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	flashTexture = SOIL_load_OGL_texture(TEXTUREDIR "full_screen_effect.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	// Load the cubemap
	cubeMap = SOIL_load_OGL_cubemap(
		TEXTUREDIR "right.jpg", TEXTUREDIR "left.jpg",
		TEXTUREDIR "top.jpg", TEXTUREDIR "bottom.jpg",
		TEXTUREDIR "front.jpg", TEXTUREDIR "back.jpg",
		SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0
	);

	if (!earthTex || !earthBump || !lavaTex || !cubeMap || !flashTexture) {
		return;
	}

	// Set the texture to repeat
	SetTextureRepeating(earthTex, true);
	SetTextureRepeating(earthBump, true);
	SetTextureRepeating(lavaTex, true);

	// Load the shaders
	modelShader = new Shader("SceneVertex.glsl", "SceneFragment.glsl");
	reflectShader = new Shader("reflectVertex.glsl", "reflectFragment.glsl");
	skyboxShader = new Shader("skyboxVertex.glsl", "skyboxFragment.glsl");
	lightShader = new Shader("PerPixelVertex.glsl", "PerPixelFragment.glsl");

	if (!reflectShader->LoadSuccess() || !skyboxShader->LoadSuccess() || !lightShader->LoadSuccess()) {
		return;
	}

	Vector3 heightmapSize = heightMap->GetHeightmapSize();

	// Set up the camera and light
	camera = new Camera(-10.0f, 190.0f, heightmapSize * Vector3(0.67f, 2.2f, -0.88f));

	// One light for the scene
	sceneLight = new Light(heightmapSize * Vector3(0.5f, 1.5f, 0.5f), Vector4(1, 1, 1, 1), heightmapSize.x);

	// Set up the matrices
	projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height, 45.0f);

	// Set up the root node and add the models
	rootNode = new SceneNode();
	rootNode->AddChild(new Volcano(volcanoMesh));

	// Bony Walls (4 walls for each side of the map)
	// Horizontal walls
	for (int i = 0; i < 8; i++) {
		rootNode->AddChild(new BonyWall(bonyWallMesh, Matrix4::Translation(Vector3(7700 - i * 1000, 500, 100))));
	}

	// Horizontal walls
	for (int i = 0; i < 8; i++) {
		rootNode->AddChild(new BonyWall(bonyWallMesh, Matrix4::Translation(Vector3(7700 - i * 1000, 500, 8200))));
	}
	
	// Vertical walls
	for (int i = 0; i < 8; i++) {
		Matrix4 transform = Matrix4::Translation(Vector3(200, 500, 1000 + i * 1000)) * Matrix4::Rotation(90.0f, Vector3(0, 1, 0));
		rootNode->AddChild(new BonyWall(bonyWallMesh, transform));
	}

	// Vertical walls
	for (int i = 0; i < 8; i++) {
		Matrix4 transform = Matrix4::Translation(Vector3(8000, 500, 1000 + i * 1000)) * Matrix4::Rotation(90.0f, Vector3(0, 1, 0));
		rootNode->AddChild(new BonyWall(bonyWallMesh, transform));
	}

	// Monsters
	rootNode->AddChild(new Monster(monsterMesh));
	
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
	delete noiseHeightMap;
	delete quad;

	// shaders cleanup
	delete reflectShader;
	delete skyboxShader;
	delete lightShader;
	delete modelShader;

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
	glDeleteTextures(1, &flashTexture);
}

void Renderer::UpdateScene(float dt) {
	// Changing camera automatically
	cameraTime += dt;

	// Move the camera in the straightMoveDirection
	Vector3 currentPosition = camera->GetPosition();
	Vector3 newPosition = currentPosition + (straightMoveDirection * dt * 80.0f); // Adjust speed
	camera->SetPosition(newPosition);

	// Rotate the camera
	rotationSpeed = 3.0f; // Adjust rotation speed
	camera->SetYaw(camera->GetYaw() - rotationSpeed * dt); // Rotate left

	// Second Scene (starts here)
	elapsedTime += dt; // Increment elapsed time

	if (!sceneChanged && elapsedTime >= 130.0f) {
		isFlashing = true; // Start the full screen effect
		flashTime = 2.0f;

		// Change the texture of the heightmap
		GLuint newEarthTexture = SOIL_load_OGL_texture(TEXTUREDIR "snow_texture.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
		if (newEarthTexture) {
			glDeleteTextures(1, &earthTex); // Delete old texture
			earthTex = newEarthTexture;         // Assign new texture
			SetTextureRepeating(earthTex, true);
		}

		GLuint newLavaTexture = SOIL_load_OGL_texture(TEXTUREDIR "frozen_lava_texture.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
		if (newLavaTexture) {
			glDeleteTextures(1, &lavaTex); // Delete old texture
			lavaTex = newLavaTexture;         // Assign new texture
			SetTextureRepeating(lavaTex, true);
			isLavaFlowing = false; // Stop the lava flow
		}

		// Add new objects for the second scene
		rootNode->AddChild(new VolcanicRock(volcanicRockMesh));
		sceneChanged = true; // Mark scene as changed
	}

	if (isFlashing) {
		flashTime -= dt; // Decrement flash time
		if (flashTime <= 0.0f) {
			isFlashing = false; // Stop the full screen effect
		}
	}

	//Update the camera
	camera->UpdateCamera(dt);
	viewMatrix = camera->BuildViewMatrix();
	if (isLavaFlowing) {
		lavaRotate += dt * 2.0f; // Rotate the water texture
		lavaCycle += dt * 0.25f; // Cycle the water texture
	}

	rootNode->Update(dt);

}

void Renderer::RenderScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	DrawSkybox();
	DrawHeightmap();
	DrawLava();
	DrawNode(rootNode);
	CreateFlashEffect();
}

void Renderer::CreateFlashEffect() {
	// Render the full screen effect
	if (isFlashing) {
		glUseProgram(0); // Use the fixed-function pipeline for simplicity

		// Disable depth testing for fullscreen quad
		glDisable(GL_DEPTH_TEST);

		// Enable 2D texture
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, flashTexture);

		// Render fullscreen quad
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 1.0f); glVertex2f(-1.0f, -1.0f); // Bottom-left
		glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, -1.0f); // Bottom-right
		glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, 1.0f); // Top-right
		glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.0f, 1.0f); // Top-left
		glEnd();

		// Restore depth testing
		glEnable(GL_DEPTH_TEST);
	}
}

void Renderer::DrawSkybox() {
	// Draw the skybox
	glDepthMask(GL_FALSE);

	BindShader(skyboxShader);
	UpdateShaderMatrices();

	quad->Draw();

	glDepthMask(GL_TRUE);
}

void Renderer::DrawHeightmap() {
	// Draw the heightmap
	BindShader(lightShader);
	SetShaderLight(*sceneLight);
	glUniform3fv(glGetUniformLocation(lightShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());

	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, earthTex);

	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "bumpTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, earthBump);

	modelMatrix.ToIdentity();
	textureMatrix.ToIdentity();

	UpdateShaderMatrices();
	heightMap->Draw();

	modelMatrix.ToIdentity();

	// Scaling for the noiseHeightMap only
	float noiseScaleX = 2.0f;  // scale factor for X axis
	float noiseScaleY = 1.0f;  // scale factor for Y axis (adjust as needed)
	float noiseScaleZ = 2.0f;

	modelMatrix = Matrix4::Scale(Vector3(noiseScaleX, noiseScaleY, noiseScaleZ)) * modelMatrix;

	UpdateShaderMatrices();
	noiseHeightMap->Draw();
	
}

void Renderer::DrawLava() {
	// Draw the lava
	BindShader(reflectShader);

	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "cubeTex"), 2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, lavaTex);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);

	Vector3 hSize = heightMap->GetHeightmapSize();

	float waterOffset = -20.0f; // Value to move the water up or down
	modelMatrix = Matrix4::Translation(hSize * Vector3(0.5f, 0.5f, 0.5f)) *
		Matrix4::Translation(Vector3(0.0f, waterOffset, 0.0f)) * // Move water down
		Matrix4::Scale(hSize * 0.5f) *
		Matrix4::Rotation(90, Vector3(1, 0, 0));

	textureMatrix = Matrix4::Translation(Vector3(lavaCycle, 0.0f, lavaCycle)) *
		Matrix4::Scale(Vector3(10, 10, 10)) *
		Matrix4::Rotation(lavaRotate, Vector3(0, 0, 1));

	UpdateShaderMatrices();

	quad->Draw();
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