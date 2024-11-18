#pragma once
#include "sceneNode.h"

class CubeRobot : public SceneNode {
public:
	CubeRobot(Mesh* sube);
	~CubeRobot(void) {};

	virtual void Update(float dt) override;

protected:
	SceneNode* head;
	SceneNode* leftArm;
	SceneNode* rightArm;
};