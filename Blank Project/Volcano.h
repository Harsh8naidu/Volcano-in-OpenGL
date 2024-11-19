#pragma once
#include "../nclgl/SceneNode.h"

class Volcano : public SceneNode {
public:
	Volcano(Mesh* volcanoMesh);
	~Volcano(void) {};
protected:
	GLuint volcanoTex;
};