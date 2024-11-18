#pragma once

#include "../nclgl/OGLRenderer.h"

class HeightMap;
class Camera;
class Light; // Predeclare for our new class type...
class Shader;

class Renderer : public OGLRenderer {
public:
	Renderer(Window& parent);
	~Renderer(void);

	void RenderScene()	override;
	void UpdateScene(float dt)	override;

protected:
	HeightMap* heightMap;
	Shader* shader;
	Camera* camera;
	Light* light; // ...and then we can use it here!
	GLuint texture;
	GLuint bumpmap;
};
