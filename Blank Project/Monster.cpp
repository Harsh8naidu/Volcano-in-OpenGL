#include "Monster.h"

Monster::Monster(Mesh* monsterMesh) {
	SceneNode* monster = new SceneNode(monsterMesh, Vector4(1, 1, 1, 1));
	texture = SOIL_load_OGL_texture(TEXTUREDIR"metallic_texture.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, 0);
	monster->SetTexture(texture);
	monster->SetModelScale(Vector3(200, 200, 200));
	monster->SetTransform(Matrix4::Translation(Vector3(5000, 100, 5000)));
	AddChild(monster);
}