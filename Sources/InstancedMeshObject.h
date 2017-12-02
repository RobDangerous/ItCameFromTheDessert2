#pragma once

#include <Kore/Math/Core.h>
#include <Kore/System.h>
#include <Kore/Graphics1/Image.h>
#include <Kore/Graphics4/Graphics.h>

#include "MeshObject.h"
//#include "PhysicsObject.h"

class InstancedMeshObject {
public:
	InstancedMeshObject(const char* meshFile, const char* textureFile, Kore::Graphics4::VertexStructure** structures, int maxCount, float scale = 1.0f);
	
	Kore::Graphics4::VertexBuffer** vertexBuffers;
	void render(Kore::Graphics4::TextureUnit tex, int instances);

	Kore::Graphics4::IndexBuffer* indexBuffer;

	Mesh* mesh;
	Kore::Graphics4::Texture* image;
};
