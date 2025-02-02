#include "BonyWall.h"

BonyWall::BonyWall(Mesh* volcanoMesh, const Matrix4& transform, GLuint bonyWallTexture) {
	SceneNode* bonyWall = new SceneNode(volcanoMesh, Vector4(1, 1, 1, 1));
	bonyWall->SetTexture(bonyWallTexture);
	bonyWall->SetModelScale(Vector3(150, 150, 150));
	//bonyWall->SetTransform(Matrix4::Translation(Vector3(7700, 500, 100)));
	bonyWall->SetTransform(transform);
	AddChild(bonyWall);
}