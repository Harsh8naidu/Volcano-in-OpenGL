#pragma once
#include "../nclgl/SceneNode.h"

class Volcano : public SceneNode {
public:
	Volcano(Mesh* volcanoMesh, GLuint volcanoTexture);
	~Volcano(void) {};
};