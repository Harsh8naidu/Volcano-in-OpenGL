#pragma once
#include "../nclgl/OGLRenderer.h"
#include "../nclgl/Vector3.h"
#include "ObjModel.h"
#include <nclgl/MeshAnimation.h>

class Camera;
class Shader;
class HeightMap;
class Mesh;
class MeshAnimation;
class MeshMaterial;
class SceneNode;

struct AnimatedMesh {
    Mesh* mesh;
    MeshAnimation* animation;
    MeshMaterial* material;
    std::vector<GLuint> textures;
    int currentFrame = 0;
    float frameTime = 0.0f;
};

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
    void RegisterAnimatedMesh(Mesh* mesh, MeshAnimation* anim, MeshMaterial* material);
    void DrawAnimatedMesh();
	void DrawNode(SceneNode* n);

    // Shaders
	Shader* lightShader = nullptr;
	Shader* reflectShader = nullptr;
	Shader* skyboxShader = nullptr;
	Shader* modelShader = nullptr;
	Shader* flashShader = nullptr;
	Shader* objModelShader = nullptr;
    Shader* terrainShader = nullptr;
    Shader* skinningShader = nullptr;

    std::vector<Shader*> shaders;

    // Heightmaps
	HeightMap* heightMap = nullptr;
	HeightMap* heightMap2 = nullptr;

    // Meshes
	Mesh* quad = nullptr;
    Mesh* bird = nullptr;

    std::vector<Mesh*> meshes;

    // Animations
    MeshAnimation* birdAnim = nullptr;

    std::vector<MeshAnimation*> animations;
    
    std::vector<AnimatedMesh> animatedMeshes;

   // Materials
    MeshMaterial* birdMaterial = nullptr;

    std::vector<MeshMaterial*> meshMaterials;

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