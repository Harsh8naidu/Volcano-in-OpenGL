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

    // Circle movement
    Vector3 center; // Center of the circular path
    float orbitRadius; // Radius of the circular path
    float orbitAngle; // Current angle around the circle
    float orbitSpeed; // Speed of orbiting (radians per second)
    float height; // Height above the ground
    float scale; // Scale of the mesh
    float currentYaw; // Current yaw angle for rotation
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
    void RegisterAnimatedMesh(Mesh* mesh, MeshAnimation* anim, MeshMaterial* material, Vector3 center, float radius, float startAngle, float speed, float height, float scale);
    void DrawAnimatedMesh();
	void DrawNode(SceneNode* n);
    void DrawPostProcessQuad();

    // Shaders
	Shader* lightShader = nullptr;
	Shader* reflectShader = nullptr;
	Shader* skyboxShader = nullptr;
	Shader* modelShader = nullptr;
	Shader* flashShader = nullptr;
	Shader* objModelShader = nullptr;
    Shader* terrainShader = nullptr;
    Shader* skinningShader = nullptr;

    // Post-processing shaders
    Shader* brightShader;
    Shader* blurShader;
    Shader* combineShader;

    std::vector<Shader*> shaders;

    // Heightmaps
	HeightMap* heightMap = nullptr;
	HeightMap* heightMap2 = nullptr;

    // Meshes
	Mesh* quad = nullptr;
    Mesh* bird = nullptr;
    Mesh* postProcessQuad = nullptr;

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