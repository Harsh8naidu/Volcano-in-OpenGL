#pragma once
#include "../nclgl/SceneNode.h"

class Monster : public SceneNode {
public:
	Monster(Mesh* monsterMesh);
	~Monster(void) {};
protected:
	GLuint monsterTex;
};
