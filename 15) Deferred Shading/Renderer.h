#pragma once
#include "../nclgl/OGLRenderer.h"

class Camera;
class HeightMap;
class Mesh;

class Renderer : public OGLRenderer {
public:
	Renderer(Window& parent);
	~Renderer(void);

	void RenderScene() override;
	void UpdateScene(float dt) override;

protected:
	void FillBuffers(); // Fills the G-Buffer with data (first pass)
	void DrawPointLights(); // Draws the point lights (second pass)
	void CombineBuffers(); // Combines the G-Buffer into the final scene (final pass)

	// Make a new texture
	void GenerateScreenTexture(GLuint& into, bool depth = false);

	Shader* sceneShader; // Shader for the first pass
	Shader* pointlightShader; // Shader for the second pass
	Shader* combineShader; // Shader for the final pass

	GLuint bufferFBO; // Framebuffer object for the G-Buffer
	GLuint bufferColourTex; // Albedo texture
	GLuint bufferNormalTex; // Normal goes here
	GLuint bufferDepthTex; // Depth goes here

	GLuint pointLightFBO; // Framebuffer object for the point light pass
	GLuint lightDiffuseTex; // Store the light's diffuse pass here
	GLuint lightSpecularTex; // Store the light's specular pass here

	HeightMap* heightMap; // Terrain
	Light* pointLights; // Array of point lights
	Mesh* sphere; // Light volume
	Mesh* quad; // Screen filling quad
	Camera* camera; // Scene camera
	GLuint earthTex;
	GLuint earthBump;
};

