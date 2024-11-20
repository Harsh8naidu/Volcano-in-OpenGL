#pragma once
#include "../nclgl/SceneNode.h"

class BonyWall : public SceneNode {
public:
	BonyWall(Mesh* bonyWallMesh, const Matrix4& transform);
	~BonyWall(void) {};
protected:
	GLuint bonyWallTex;
};