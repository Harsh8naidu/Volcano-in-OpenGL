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

	void UpdateScene(float dt) override;

protected:
	void DrawHeightmap();
	void DrawSkybox();
	void DrawVolcano();
	void DrawNode(SceneNode* n);

    // Shaders
	Shader* lightShader = nullptr;
	Shader* reflectShader = nullptr;
	Shader* skyboxShader = nullptr;
	Shader* modelShader = nullptr;
	Shader* flashShader = nullptr;
	Shader* objModelShader = nullptr;
    Shader* terrainShader = nullptr;

    std::vector<Shader*> shaders;

    // Heightmaps
	HeightMap* heightMap = nullptr;
	HeightMap* heightMap2 = nullptr;

    // Meshes
	Mesh* quad = nullptr;

    std::vector<Mesh*> meshes;

    // Scene Lights
	Light* sceneLight = nullptr;
    Light* sceneLight2 = nullptr;
    Light* sceneLight3 = nullptr;
    Light* sceneLight4 = nullptr;

    std::vector<Light*> sceneLights;

	Camera* camera = nullptr;

    // Textures
	GLuint cubeMap;
	GLuint lavaTex;
    GLuint heightMapTex;
    GLuint heightMapTex2;
	GLuint earthTex;
	GLuint earthBump;
	GLuint modelTexture;
	GLuint volcanoTexture;

    std::vector<GLuint> textures;

	SceneNode* rootNode = nullptr;

	// .obj objects
	ObjModel* volcanoModel = nullptr;
};