#include "pch.h"
#include "Ant.h"
#include "Engine/InstancedMeshObject.h"
#include "KitchenObject.h"

#include <assert.h>
#include <Kore/Math/Random.h>

using namespace Kore;

namespace {
	Kore::VertexBuffer** vertexBuffers;
	InstancedMeshObject* body;
	InstancedMeshObject* leg;

	const int maxAnts = 500;
	Ant ants[maxAnts];
	float* scent;
	const int scents = 100;
	
	float scentAt(int x, int y, int z) {
		if (z >= scents || y >= scents || z >= scents) return 0;
		if (z < 0 || y < 0 || x < 0) return 0;
		return scent[z * scents * scents + y * scents + x];
	}

	void setScent(int x, int y, int z, float value) {
		if (z >= scents || y >= scents || z >= scents) return;
		if (z < 0 || y < 0 || x < 0) return;
		scent[z * scents * scents + y * scents + x] = value;
	}

	int gridPosition(float pos) {
		return Kore::round(pos + scents / 2);
	}

	vec3i gridPosition(vec3 pos) {
		vec3i gridPos;
		gridPos.x() = gridPosition(pos.x());
		gridPos.y() = gridPosition(pos.y());
		gridPos.z() = gridPosition(pos.z());
		return gridPos;
	}

	float realPosition(int grid) {
		return grid - scents / 2;
	}

	vec3 realPosition(vec3i grid) {
		vec3 pos;
		pos.x() = realPosition(grid.x());
		pos.y() = realPosition(grid.y());
		pos.z() = realPosition(grid.z());
		return pos;
	}
}

Ant::Ant() : goingup(false) {
	rotation = mat4::Identity();
	forward = vec4(0, 0, -1, 0);
	right = vec4(1, 0, 0, 0);
}

void Ant::init() {
	scent = new float[scents * scents * scents];
	for (int i = 0; i < scents * scents * scents; ++i) {
		scent[i] = Random::get(100) / 100.0f;
	}

	VertexStructure** structures = new VertexStructure*[2];
	structures[0] = new VertexStructure();
	structures[0]->add("pos", Float3VertexData);
	structures[0]->add("tex", Float2VertexData);
	structures[0]->add("nor", Float3VertexData);

	structures[1] = new VertexStructure();
	structures[1]->add("M", Float4x4VertexData);
	structures[1]->add("N", Float4x4VertexData);
	structures[1]->add("tint", Float4VertexData);

	body = new InstancedMeshObject("Data/Meshes/ant_body.obj", "Data/Textures/tank_bottom.png", structures, 10, 10);
	leg = new InstancedMeshObject("Data/Meshes/ant_leg.obj", "Data/Textures/tank_bottom.png", structures, 10, 10);

	vertexBuffers = new VertexBuffer*[2];
	vertexBuffers[0] = body->vertexBuffers[0];
	vertexBuffers[1] = new VertexBuffer(maxAnts, *structures[1], 1);

	for (int i = 0; i < maxAnts; ++i) {
		ants[i].position = vec3(Random::get(-1000, 1000) / 10.0f, 0, Random::get(-1000, 1000) / 10.0f);
		//ants[i].rotation = Quaternion(ants[i].right, Random::get(3000.0f) / 1000.0f).matrix() * ants[i].rotation;
	}
}

void Ant::chooseScent() {
	vec3i grid = gridPosition(position);
	if (grid != lastGrid) {
		setScent(grid.x(), grid.y(), grid.z(), scentAt(grid.x(), grid.y(), grid.z()) + 100.5f);
		lastGrid = grid;
		vec3i nextGrid = gridPosition(position + vec3(forward.x(), forward.y(), forward.z()) * 0.5f);
		vec3i neighbours[8];
		for (int i = 0; i < 8; ++i) neighbours[i] = grid;
		neighbours[0].x() -= 1;
		neighbours[0].z() += 1;
		neighbours[1].x() += 0;
		neighbours[1].z() += 1;
		neighbours[2].x() += 1;
		neighbours[2].z() += 1;
		neighbours[3].x() += 1;
		neighbours[3].z() += 0;
		neighbours[4].x() += 1;
		neighbours[4].z() -= 1;
		neighbours[5].x() += 0;
		neighbours[5].z() -= 1;
		neighbours[6].x() -= 1;
		neighbours[6].z() -= 1;
		neighbours[7].x() -= 1;
		neighbours[7].z() += 0;

		float maxScent = 0;
		for (int i = 0; i < 8; ++i) {
			float scent = scentAt(neighbours[i].x(), neighbours[i].y(), neighbours[i].z());
			if (scent > maxScent) {
				if (nextGrid == neighbours[i] || (i > 0 && nextGrid == neighbours[i - 1]) || (i < 7 && nextGrid == neighbours[i + 1])) {
					maxScent = scent;
					vec3 pos = realPosition(neighbours[i]);
					vec3 forward = pos - position;
					forward = forward.normalize();
					this->forward = vec4(forward.x(), forward.y(), forward.z(), 0);
					float angle = Kore::atan2(forward.z(), forward.x());
					rotation = Quaternion(vec3(0, 1, 0), angle + pi / 2.0f).matrix();
				}
			}
		}
	}
}

extern MeshObject* objects[];
extern KitchenObject* kitchenObjects[];

void Ant::move() {
	/*
	if (goingup) {
		if (!intersects(vec4(0, 0, -1, 0)) || position.y() > 5) {
			rotation = mat4::Identity();

			forward = vec4(0, 0, -1, 0);
			//forward = rotation * vec4(0, 0, 1, 0);
			//up = rotation * vec4(0, 1, 0, 0);
			//right = rotation * vec4(1, 0, 0, 0);

			goingup = false;
		}
	}
	else {
		if (intersects(vec4(0, 0, -1, 0))) {
			rotation = Quaternion(right, pi / 2).matrix();

			forward = rotation * vec4(0, 0, 1, 0);
			//up = rotation * vec4(0, 1, 0, 0);
			//right = rotation * vec4(1, 0, 0, 0);

			goingup = true;
		}
	}
	*/
	if (legRotationUp) {
		legRotation += 0.15f;
		if (legRotation > pi / 4.0f) {
			legRotationUp = false;
		}
	}
	else {
		legRotation -= 0.15f;
		if (legRotation < -pi / 4.0f) {
			legRotationUp = true;
		}
	}

	chooseScent();
	
	position += forward * 0.05f;
}

void Ant::moveEverybody() {
	for (int i = 0; i < maxAnts; ++i) {
		ants[i].move();
	}
}

bool Ant::intersects(vec3 dir) {
	for (unsigned oi = 0; kitchenObjects[oi] != nullptr; ++oi) {
		if (intersectsWith(kitchenObjects[oi]->body, dir) || intersectsWith(kitchenObjects[oi]->door_closed, dir)) {
			return true;
		}
	}
	return false;
}

bool Ant::intersectsWith(MeshObject* obj, vec3 dir) {
    if (obj == nullptr) return false;
    for (int k = 0; k < obj->colliderCount; ++k) {
        float distance;
		if (obj->collider[k] != nullptr && obj->collider[k]->IntersectsWith(position, dir, distance) && distance < .5f) {
			return true;
		}
    }
    return false;
}

void Ant::render(ConstantLocation vLocation, TextureUnit tex, mat4 view) {
	int c = 0;
	{
		float* data = vertexBuffers[1]->lock();
		c = 0;
		for (int i = 0; i < maxAnts; i++) {
			const float scale = 0.02f;
			mat4 M = mat4::Translation(ants[i].position.x(), ants[i].position.y(), ants[i].position.z()) * ants[i].rotation * mat4::Scale(scale, scale, scale);
			setMatrix(data, i, 0, 36, M);
			setMatrix(data, i, 16, 36, calculateN(M));
			setVec4(data, i, 32, 36, vec4(1, 1, 1, 1));
			c++;
		}
		vertexBuffers[1]->unlock();
	}

	Graphics::setTexture(tex, body->image);
	VertexBuffer* vertexBuffers[2];
	vertexBuffers[0] = body->vertexBuffers[0];
	vertexBuffers[1] = ::vertexBuffers[1];
	Graphics::setVertexBuffers(vertexBuffers, 2);
	Graphics::setIndexBuffer(*body->indexBuffer);
	Graphics::drawIndexedVerticesInstanced(c);

	{
		float* data = vertexBuffers[1]->lock();
		c = 0;
		for (int i = 0; i < maxAnts; i++) {
			const float scale = 0.02f;
			mat4 M = mat4::Translation(ants[i].position.x(), ants[i].position.y() + 0.05f, ants[i].position.z()) * mat4::RotationX(ants[i].legRotation) * ants[i].rotation * mat4::Scale(scale, scale, scale);
			setMatrix(data, i, 0, 36, M);
			setMatrix(data, i, 16, 36, calculateN(M));
			setVec4(data, i, 32, 36, vec4(1, 1, 1, 1));
			c++;
		}
		vertexBuffers[1]->unlock();
	}

	vertexBuffers[0] = leg->vertexBuffers[0];
	Graphics::setVertexBuffers(vertexBuffers, 2);
	Graphics::setIndexBuffer(*leg->indexBuffer);
	Graphics::drawIndexedVerticesInstanced(c);

	{
		float* data = vertexBuffers[1]->lock();
		c = 0;
		for (int i = 0; i < maxAnts; i++) {
			const float scale = 0.02f;
			mat4 M = mat4::Translation(ants[i].position.x(), ants[i].position.y() + 0.05f, ants[i].position.z() - 0.15f) * mat4::RotationX(-ants[i].legRotation) * ants[i].rotation * mat4::Scale(scale, scale, scale);
			setMatrix(data, i, 0, 36, M);
			setMatrix(data, i, 16, 36, calculateN(M));
			setVec4(data, i, 32, 36, vec4(1, 1, 1, 1));
			c++;
		}
		vertexBuffers[1]->unlock();
	}

	vertexBuffers[0] = leg->vertexBuffers[0];
	Graphics::setVertexBuffers(vertexBuffers, 2);
	Graphics::setIndexBuffer(*leg->indexBuffer);
	Graphics::drawIndexedVerticesInstanced(c);

	{
		float* data = vertexBuffers[1]->lock();
		c = 0;
		for (int i = 0; i < maxAnts; i++) {
			const float scale = 0.02f;
			mat4 M = mat4::Translation(ants[i].position.x(), ants[i].position.y() + 0.05f, ants[i].position.z() - 0.25f) * mat4::RotationX(ants[i].legRotation) * ants[i].rotation * mat4::Scale(scale, scale, scale);
			setMatrix(data, i, 0, 36, M);
			setMatrix(data, i, 16, 36, calculateN(M));
			setVec4(data, i, 32, 36, vec4(1, 1, 1, 1));
			c++;
		}
		vertexBuffers[1]->unlock();
	}

	vertexBuffers[0] = leg->vertexBuffers[0];
	Graphics::setVertexBuffers(vertexBuffers, 2);
	Graphics::setIndexBuffer(*leg->indexBuffer);
	Graphics::drawIndexedVerticesInstanced(c);

	{
		float* data = vertexBuffers[1]->lock();
		c = 0;
		for (int i = 0; i < maxAnts; i++) {
			const float scale = 0.02f;
			mat4 M = mat4::Translation(ants[i].position.x(), ants[i].position.y() + 0.05f, ants[i].position.z()) * mat4::RotationX(-ants[i].legRotation) * mat4::RotationY(pi) * ants[i].rotation * mat4::Scale(scale, scale, scale);
			setMatrix(data, i, 0, 36, M);
			setMatrix(data, i, 16, 36, calculateN(M));
			setVec4(data, i, 32, 36, vec4(1, 1, 1, 1));
			c++;
		}
		vertexBuffers[1]->unlock();
	}

	vertexBuffers[0] = leg->vertexBuffers[0];
	Graphics::setVertexBuffers(vertexBuffers, 2);
	Graphics::setIndexBuffer(*leg->indexBuffer);
	Graphics::drawIndexedVerticesInstanced(c);

	{
		float* data = vertexBuffers[1]->lock();
		c = 0;
		for (int i = 0; i < maxAnts; i++) {
			const float scale = 0.02f;
			mat4 M = mat4::Translation(ants[i].position.x(), ants[i].position.y() + 0.05f, ants[i].position.z() - 0.15f) * mat4::RotationX(ants[i].legRotation) * mat4::RotationY(pi) * ants[i].rotation * mat4::Scale(scale, scale, scale);
			setMatrix(data, i, 0, 36, M);
			setMatrix(data, i, 16, 36, calculateN(M));
			setVec4(data, i, 32, 36, vec4(1, 1, 1, 1));
			c++;
		}
		vertexBuffers[1]->unlock();
	}

	vertexBuffers[0] = leg->vertexBuffers[0];
	Graphics::setVertexBuffers(vertexBuffers, 2);
	Graphics::setIndexBuffer(*leg->indexBuffer);
	Graphics::drawIndexedVerticesInstanced(c);

	{
		float* data = vertexBuffers[1]->lock();
		c = 0;
		for (int i = 0; i < maxAnts; i++) {
			const float scale = 0.02f;
			mat4 M = mat4::Translation(ants[i].position.x(), ants[i].position.y() + 0.05f, ants[i].position.z() - 0.25f) * mat4::RotationX(-ants[i].legRotation) * mat4::RotationY(pi) * ants[i].rotation * mat4::Scale(scale, scale, scale);
			setMatrix(data, i, 0, 36, M);
			setMatrix(data, i, 16, 36, calculateN(M));
			setVec4(data, i, 32, 36, vec4(1, 1, 1, 1));
			c++;
		}
		vertexBuffers[1]->unlock();
	}

	vertexBuffers[0] = leg->vertexBuffers[0];
	Graphics::setVertexBuffers(vertexBuffers, 2);
	Graphics::setIndexBuffer(*leg->indexBuffer);
	Graphics::drawIndexedVerticesInstanced(c);
}
