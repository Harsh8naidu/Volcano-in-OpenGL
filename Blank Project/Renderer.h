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

	Shader* lightShader;
	Shader* reflectShader;
	Shader* skyboxShader;
	Shader* modelShader;
	Shader* flashShader;
	Shader* objModelShader;

	HeightMap* heightMap;
	HeightMap* noiseHeightMap;

	Mesh* quad;

	Light* sceneLight;
	Camera* camera;

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

	SceneNode* rootNode;

	Mesh* volcanoMesh;
	Mesh* bonyWallMesh;
	Mesh* monsterMesh;
	Mesh* volcanicRockMesh;

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
	ObjModel* testObjModel;
};