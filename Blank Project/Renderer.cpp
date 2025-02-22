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
	
	// Load .obj files
	testObjModel = new ObjModel(MODELDIR "Mug.obj", MATERIALDIR "Mug.mtl");

	// Load the meshes
	quad = Mesh::GenerateQuad();
	volcanoMesh = Mesh::LoadFromMeshFile("Volcano.msh");
	//bonyWallMesh = Mesh::LoadFromMeshFile("BonyWall.msh");
	//monsterMesh = Mesh::LoadFromMeshFile("Role_T.msh");
	//volcanicRockMesh = Mesh::LoadFromMeshFile("VolcanicRock.msh");

	// Load the heightmaps
	heightMap = new HeightMap(TEXTUREDIR "volcano_heightmap.png");
	noiseHeightMap = new HeightMap(TEXTUREDIR "noise.png");

	// Load the textures
	lavaTex = SOIL_load_OGL_texture(TEXTUREDIR "lava_texture.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	earthTex = SOIL_load_OGL_texture(TEXTUREDIR "volcanic_rock.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	earthBump = SOIL_load_OGL_texture(TEXTUREDIR "Barren RedsDOT3.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	//flashTexture = SOIL_load_OGL_texture(TEXTUREDIR "full_screen_effect.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	//bonyWallTexture = SOIL_load_OGL_texture(TEXTUREDIR"bonywall_texture.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, 0);

	volcanoTexture = SOIL_load_OGL_texture(TEXTUREDIR"volcano_molten_lava.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, 0);

	//monsterTexture = SOIL_load_OGL_texture(TEXTUREDIR"metallic_texture.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, 0);

	//snowTexture = SOIL_load_OGL_texture(TEXTUREDIR "snow_texture.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	frozenLavaTexture = SOIL_load_OGL_texture(TEXTUREDIR "frozen_lava_texture.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	// Load the cubemap
	cubeMap = SOIL_load_OGL_cubemap(
		TEXTUREDIR "right.jpg", TEXTUREDIR "left.jpg",
		TEXTUREDIR "top.jpg", TEXTUREDIR "bottom.jpg",
		TEXTUREDIR "front.jpg", TEXTUREDIR "back.jpg",
		SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0
	);

	if (!earthTex || !earthBump || !lavaTex || !cubeMap) {
		return;
	}

	// Set the texture to repeat
	SetTextureRepeating(earthTex, true);
	SetTextureRepeating(earthBump, true);
	SetTextureRepeating(lavaTex, true);

	// Load the shaders
	objModelShader = new Shader("modelVertexShader.glsl", "modelFragmentShader.glsl");
	modelShader = new Shader("SceneVertex.glsl", "SceneFragment.glsl");
	reflectShader = new Shader("reflectVertex.glsl", "reflectFragment.glsl");
	skyboxShader = new Shader("skyboxVertex.glsl", "skyboxFragment.glsl");
	lightShader = new Shader("PerPixelVertex.glsl", "PerPixelFragment.glsl");

	if (!reflectShader->LoadSuccess() || !skyboxShader->LoadSuccess() || !lightShader->LoadSuccess() ||
		!modelShader->LoadSuccess()) {
		return;
	}

	if (!objModelShader->LoadSuccess()) {
		return;
	}

	Vector3 heightmapSize = heightMap->GetHeightmapSize();

	// Set up the camera and light
	camera = new Camera(-10.0f, 190.0f, heightmapSize * Vector3(0.67f, 3.6f, 0.3f)); // -0.88f
	camera->SetPitch(5.0f);
	camera->SetYaw(270.0f);

	// One light for the scene
	sceneLight = new Light(heightmapSize * Vector3(0.5f, 1.5f, 0.5f), Vector4(1, 1, 1, 1), heightmapSize.x);

	// Set up the matrices
	projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height, 45.0f);

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
	delete noiseHeightMap;
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
	// Changing camera automatically
	cameraTime += dt;

	// Move the camera in the straightMoveDirection
	Vector3 currentPosition = camera->GetPosition();
	Vector3 newPosition = currentPosition + (straightMoveDirection * dt * 80.0f); // Adjust speed
	//camera->SetPosition(newPosition);

	// Rotate the camera
	rotationSpeed = 3.0f; // Adjust rotation speed
	//camera->SetYaw(camera->GetYaw() - rotationSpeed * dt); // Rotate left

	// Second Scene (starts here)
	elapsedTime += dt; // Increment elapsed time

	if (!sceneChanged && elapsedTime >= 130.0f) {
		isFlashing = true; // Start the full screen effect
		flashTime = 2.0f;

		// Change the texture of the heightmap
		if (snowTexture) {
			glDeleteTextures(1, &earthTex); // Delete old texture
			earthTex = snowTexture;         // Assign new texture
			SetTextureRepeating(earthTex, true);
		}

		if (frozenLavaTexture) {
			glDeleteTextures(1, &lavaTex); // Delete old texture
			lavaTex = frozenLavaTexture;         // Assign new texture
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
	//DrawHeightmap();
	DrawLava();
	DrawMug();
	//DrawNode(rootNode);
	//CreateFlashEffect(); // Not working
}

void Renderer::DrawMug() {
	BindShader(objModelShader);
	UpdateShaderMatrices();

	Vector3 hSize = heightMap->GetHeightmapSize();

	float waterOffset = 20.0f; // Value to move the water up or down
	modelMatrix = Matrix4::Translation(hSize * Vector3(0.7f, 3.5f, 0.3f)) *
		Matrix4::Scale(Vector3(10.0f, 10.0f, 10.0f)) *
		Matrix4::Rotation(90, Vector3(0, 1, 0));

	// Iterate through all materials and bind them
	const std::vector<Material>& materials = testObjModel->GetMaterials();

	for (const Material& mat : materials) {
		// Bind the material properties to the shader
		glUniform1f(glGetUniformLocation(objModelShader->GetProgram(), "shininess"), mat.shininess);
		glUniform3fv(glGetUniformLocation(objModelShader->GetProgram(), "ambient"), 1, (float*)&mat.ambient);
		glUniform3fv(glGetUniformLocation(objModelShader->GetProgram(), "specular"), 1, (float*)&mat.specular);
		glUniform3fv(glGetUniformLocation(objModelShader->GetProgram(), "emission"), 1, (float*)&mat.emission);
		glUniform1f(glGetUniformLocation(objModelShader->GetProgram(), "ior"), mat.ior);
		glUniform1f(glGetUniformLocation(objModelShader->GetProgram(), "dissolveFactor"), mat.dissolveFactor);
		glUniform1i(glGetUniformLocation(objModelShader->GetProgram(), "illuminationModel"), mat.illuminationModel);

		// Bind the diffuse texture
		if (mat.diffuseTexture > 0) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, mat.diffuseTexture);
			std::cout << "diffuseTexture: " << mat.diffuseTexture << std::endl;
			glUniform1i(glGetUniformLocation(objModelShader->GetProgram(), "diffuseTexture"), 0);
		}

		glBindBuffer(GL_ARRAY_BUFFER, testObjModel->modelVBO); // Bind the VBO
		glBindVertexArray(testObjModel->modelVAO); // Bind the VAO
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, testObjModel->modelEBO); // Bind the EBO

		// Draw the model
		glDrawElements(GL_TRIANGLES, testObjModel->indices, GL_UNSIGNED_INT, nullptr);
		// Unbind the VAO
		glBindVertexArray(0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
}

//void Renderer::CreateFlashEffect() {
//	// Render the full screen effect
//	if (isFlashing) {
//		BindShader(flashShader); // Use the custom flash shader
//
//		// Update projection and model-view matrices for the fullscreen quad
//		projMatrix = Matrix4::Orthographic(-1, 1, 1, -1, -1, 1);
//		modelMatrix.ToIdentity();
//		viewMatrix.ToIdentity();
//		UpdateShaderMatrices();
//
//		// Bind the flash texture
//		glActiveTexture(GL_TEXTURE0);
//		glBindTexture(GL_TEXTURE_2D, flashTexture);
//		glUniform1i(glGetUniformLocation(flashShader->GetProgram(), "flashTex"), 0);
//
//		// Draw the full-screen quad
//		quad->Draw();
//	}
//}

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
	//BindShader(reflectShader);

	//glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "diffuseTex"), 0);
	//glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "cubeTex"), 2);

	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, lavaTex);

	//Vector3 hSize = heightMap->GetHeightmapSize();

	//float waterOffset = -20.0f; // Value to move the water up or down
	//modelMatrix = Matrix4::Translation(hSize * Vector3(0.5f, 0.5f, 0.5f)) *
	//	Matrix4::Translation(Vector3(0.0f, waterOffset, 0.0f)) * // Move water down
	//	Matrix4::Scale(Vector3(10.0f, 10.0f, 10.0f)) *
	//	Matrix4::Rotation(180, Vector3(1, 0, 0));

	//textureMatrix = Matrix4::Translation(Vector3(lavaCycle, 0.0f, lavaCycle)) *
	//	Matrix4::Scale(Vector3(1, 1, 1)) *
	//	Matrix4::Rotation(lavaRotate, Vector3(0, 0, 1));

	//UpdateShaderMatrices();

	//quad->Draw();
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