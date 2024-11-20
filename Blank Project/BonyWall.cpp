#include "BonyWall.h"

BonyWall::BonyWall(Mesh* volcanoMesh, const Matrix4& transform) {
	SceneNode* bonyWall = new SceneNode(volcanoMesh, Vector4(1, 1, 1, 1));
	texture = SOIL_load_OGL_texture(TEXTUREDIR"bonywall_texture.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, 0);
	bonyWall->SetTexture(texture);
	bonyWall->SetModelScale(Vector3(150, 150, 150));
	//bonyWall->SetTransform(Matrix4::Translation(Vector3(7700, 500, 100)));
	bonyWall->SetTransform(transform);
	AddChild(bonyWall);
}