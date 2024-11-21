#include "VolcanicRock.h"

VolcanicRock::VolcanicRock(Mesh* volcanicRockMesh) {
	SceneNode* volcanicRock = new SceneNode(volcanicRockMesh, Vector4(1, 1, 1, 1));
	texture = SOIL_load_OGL_texture(TEXTUREDIR"molten_rock_texture.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, 0);
	volcanicRock->SetTexture(texture);
	volcanicRock->SetModelScale(Vector3(200, 200, 200));
	volcanicRock->SetTransform(Matrix4::Translation(Vector3(3000, 20, 7000)));
	AddChild(volcanicRock);
}