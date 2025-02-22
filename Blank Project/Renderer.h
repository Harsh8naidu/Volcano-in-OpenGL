#pragma once
#include "../nclgl/OGLRenderer.h"
#include "../nclgl/Vector3.h"
#include "ObjModel.h"

class Camera;
class Shader;
class HeightMap;
class Mesh;
class MeshAnimation;
class MeshMaterial;
class SceneNode;

class Renderer : public OGLRenderer {
public:
	Renderer(Window& parent);
	~Renderer(void);

	void RenderScene() override;

	void DrawMug();
	
	void UpdateScene(float dt) override;

protected:
	void DrawHeightmap();
	void DrawLava();
	void DrawSkybox();
	void DrawVolcano();
	void DrawNode(SceneNode* n);

	void CreateFlashEffect();

	Shader* lightShader = nullptr;
	Shader* reflectShader = nullptr;
	Shader* skyboxShader = nullptr;
	Shader* modelShader = nullptr;
	Shader* flashShader = nullptr;
	Shader* objModelShader = nullptr;

	HeightMap* heightMap = nullptr;
	HeightMap* noiseHeightMap = nullptr;

	Mesh* quad = nullptr;

	Light* sceneLight = nullptr;
	Camera* camera = nullptr;

	GLuint cubeMap;
	GLuint lavaTex;
	GLuint earthTex;
	GLuint earthBump;
	GLuint modelTexture;
	GLuint bonyWallTexture;
	GLuint volcanoTexture;
	GLuint monsterTexture;
	GLuint snowTexture;
	GLuint frozenLavaTexture;

	float lavaRotate;
	float lavaCycle;

	SceneNode* rootNode = nullptr;

	Mesh* volcanoMesh = nullptr;
	Mesh* bonyWallMesh = nullptr;
	Mesh* monsterMesh = nullptr;
	Mesh* volcanicRockMesh = nullptr;

	Vector3 straightMoveDirection; // Direction for straight camera movement
	float cameraTime; // Time for camera movement
	float rotationSpeed; // Rotation speed for camera

	// Scene change
	float elapsedTime; // To track the elapsed time
	bool sceneChanged; // To indicate whether the scene has changed
	bool isLavaFlowing;

	// Full screen effect
	//GLuint flashTexture;
	bool isFlashing;
	float flashTime;

	// .obj objects
	ObjModel* testObjModel = nullptr;
};