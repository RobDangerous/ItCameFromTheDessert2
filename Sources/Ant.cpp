#include "pch.h"
#include "Ant.h"
#include "Engine/InstancedMeshObject.h"

#include <Kore/Math/Random.h>

using namespace Kore;

namespace {
	Kore::VertexBuffer** vertexBuffers;
	InstancedMeshObject* body;
	const int maxAnts = 500;
	Ant ants[maxAnts];
}

void Ant::init() {
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

	for (int i = 0; i < maxAnts; ++i) {
		ants[i].position = vec3(Random::get(-1000, 1000) / 10.0f, 0, Random::get(-1000, 1000) / 10.0f);
		ants[i].rotation = Quaternion(ants[i].right, Random::get(3000.0f) / 1000.0f).matrix() * ants[i].rotation;
	}
}

Ant::Ant() {
	rotation = mat4::Identity();
	right = vec4(1, 0, 0, 0);
}

void Ant::move() {
	for (int i = 0; i < maxAnts; ++i) {
		//	for (unsigned i = 0; i < objects.size(); ++i) {
		//		if (objects[i]->Collider.IntersectsWith(position, forward)) {
		ants[i].rotation = Quaternion(ants[i].right, 0.1f).matrix() * ants[i].rotation;

		ants[i].forward = ants[i].rotation * vec4(0, 0, 1, 0);
		ants[i].up = ants[i].rotation * vec4(0, 1, 0, 0);
		ants[i].right = ants[i].rotation * vec4(1, 0, 0, 0);
		//		}
		//	}
	}
}

void Ant::render(ConstantLocation vLocation, TextureUnit tex, mat4 view) {
	float* data = vertexBuffers[1]->lock();
	int c = 0;
	for (int i = 0; i < maxAnts; i++) {
		mat4 M = mat4::Translation(ants[i].position.x(), ants[i].position.y(), ants[i].position.z()) * ants[i].rotation * mat4::Scale(0.1f, 0.1f, 0.1f); // physicsObject[i]->GetMatrix();
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
