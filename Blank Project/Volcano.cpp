#include "Volcano.h"

Volcano::Volcano(Mesh* volcanoMesh) {
	SceneNode* volcano = new SceneNode(volcanoMesh, Vector4(1, 1, 1, 1));
	texture = SOIL_load_OGL_texture(TEXTUREDIR"dark_rock_texture.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, 0);
	volcano->SetTexture(texture);
	volcano->SetModelScale(Vector3(20, 20, 20));
	volcano->SetTransform(Matrix4::Translation(Vector3(4000, 120, 4000)));
	AddChild(volcano);
}