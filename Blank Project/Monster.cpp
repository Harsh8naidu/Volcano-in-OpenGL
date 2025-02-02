#include "Monster.h"

Monster::Monster(Mesh* monsterMesh, GLuint monsterTexture) {
	SceneNode* monster = new SceneNode(monsterMesh, Vector4(1, 1, 1, 1));
	monster->SetTexture(monsterTexture);
	monster->SetModelScale(Vector3(200, 200, 200));
	monster->SetTransform(Matrix4::Translation(Vector3(5000, 100, 5000)));
	AddChild(monster);
}