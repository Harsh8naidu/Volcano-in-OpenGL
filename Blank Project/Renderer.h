#pragma once
#include "../nclgl/OGLRenderer.h"

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
	void UpdateScene(float dt) override;

protected:
	void DrawHeightmap();
	void DrawLava();
	void DrawSkybox();
	void DrawVolcano();
	void DrawNode(SceneNode* n);

	Shader* lightShader;
	Shader* reflectShader;
	Shader* skyboxShader;
	Shader* modelShader;

	HeightMap* heightMap;
	HeightMap* noiseHeightMap;

	Mesh* quad;

	Light* light;
	Camera* camera;

	GLuint cubeMap;
	GLuint lavaTex;
	GLuint earthTex;
	GLuint earthBump;
	GLuint modelTexture;

	float lavaRotate;
	float lavaCycle;

	SceneNode* rootNode;

	Mesh* volcanoMesh;
	Mesh* bonyWallMesh;
	Mesh* monsterMesh;
};