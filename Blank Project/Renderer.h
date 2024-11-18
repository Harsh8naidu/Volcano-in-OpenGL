#pragma once
#include "../nclgl/OGLRenderer.h"
class Camera;
class Shader;
class HeightMap;

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

	Shader* lightShader;
	Shader* reflectShader;
	Shader* skyboxShader;

	HeightMap* heightMap;
	HeightMap* noiseHeightMap;

	Mesh* quad;

	Light* light;
	Camera* camera;

	GLuint cubeMap;
	GLuint lavaTex;
	GLuint earthTex;
	GLuint earthBump;

	float waterRotate;
	float waterCycle;
};