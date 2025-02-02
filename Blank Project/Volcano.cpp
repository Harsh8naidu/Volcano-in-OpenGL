#include "Volcano.h"

Volcano::Volcano(Mesh* volcanoMesh, GLuint volcanoTexture) {
	SceneNode* volcano = new SceneNode(volcanoMesh, Vector4(1, 1, 1, 1));
	volcano->SetTexture(volcanoTexture);
	volcano->SetModelScale(Vector3(20, 20, 20));
	volcano->SetTransform(Matrix4::Translation(Vector3(4000, 120, 4000)));
	AddChild(volcano);
}