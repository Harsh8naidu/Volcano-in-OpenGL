#include "Volcano.h"

Volcano::Volcano(Mesh* volcanoMesh) {
	std::cout << "volcano consturctor called" << std::endl;
	SceneNode* volcano = new SceneNode(volcanoMesh, Vector4(1, 1, 1, 1));
	texture = SOIL_load_OGL_texture(TEXTUREDIR"volcano_texture.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, 0);
	volcano->SetTexture(texture);
	volcano->SetModelScale(Vector3(100, 100, 100));
	volcano->SetTransform(Matrix4::Translation(Vector3(5000, 100, 1000)));
	AddChild(volcano);
}