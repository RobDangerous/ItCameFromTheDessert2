#include "pch.h"
#include "Ant.h"
#include "Engine/InstancedMeshObject.h"

using namespace Kore;

Ant::Ant() {
	maxAnts = 1;

	rotation = mat4::Identity();

	position = vec3(0, 0, 0);
	right = vec4(1, 0, 0, 0);

	VertexStructure** structures = new VertexStructure*[2];
	structures[0] = new VertexStructure();
	structures[0]->add("pos", Float3VertexData);
	structures[0]->add("tex", Float2VertexData);
	structures[0]->add("nor", Float3VertexData);

	structures[1] = new VertexStructure();
	structures[1]->add("M", Float4x4VertexData);
	structures[1]->add("N", Float4x4VertexData);
	structures[1]->add("tint", Float4VertexData);

	body = new InstancedMeshObject("Data/Meshes/tank_bottom.obj", "Data/Textures/tank_bottom.png", structures, 10, 10);

	vertexBuffers = new VertexBuffer*[2];
	vertexBuffers[0] = body->vertexBuffers[0];
	vertexBuffers[1] = new VertexBuffer(maxAnts, *structures[1], 1);
}

void Ant::move() {
//	for (unsigned i = 0; i < objects.size(); ++i) {
//		if (objects[i]->Collider.IntersectsWith(position, forward)) {
	rotation = Quaternion(right, 0.1f).matrix() * rotation;

	forward = rotation * vec4(0, 0, 1, 0);
	up = rotation * vec4(0, 1, 0, 0);
	right = rotation * vec4(1, 0, 0, 0);
//		}
//	}
}

void Ant::render(ConstantLocation vLocation, TextureUnit tex, mat4 view) {
	float* data = vertexBuffers[1]->lock();
	int c = 0;
	for (int i = 0; i < maxAnts; i++) {
		mat4 M = rotation * mat4::Translation(position.x(), position.y(), position.z()); // physicsObject[i]->GetMatrix();
		setMatrix(data, i, 0, 36, M);
		setMatrix(data, i, 16, 36, calculateN(M));
		setVec4(data, i, 32, 36, vec4(1, 1, 1, 1));
		c++;
	}
	vertexBuffers[1]->unlock();

	Graphics::setTexture(tex, body->image);
	Graphics::setVertexBuffers(vertexBuffers, 2);
	Graphics::setIndexBuffer(*body->indexBuffer);
	Graphics::drawIndexedVerticesInstanced(c);
}
