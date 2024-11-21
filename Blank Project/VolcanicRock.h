#pragma once
#include "../nclgl/SceneNode.h"

class VolcanicRock : public SceneNode {
public:
	VolcanicRock(Mesh* volcanicRockMesh);
	~VolcanicRock(void) {};
protected:
	GLuint volcanicRockTex;
};
