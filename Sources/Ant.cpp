#include "pch.h"
#include "Ant.h"
#include "InstancedMeshObject.h"
//#include "Engine/TriggerCollider.h"
//#include "KitchenObject.h"
#include "Rendering.h"
#include "Kitchen.h"

#include <assert.h>

#include <Kore/Math/Random.h>
#include <Kore/Log.h>

using namespace Kore;

namespace {
	vec3 getAxisVector(const mat4& transform, int i) {
		return vec3(transform.data[i], transform.data[i + 4], transform.data[i + 8]);
	}

	struct Box {
		vec3 getAxis(unsigned index) const { return getAxisVector(transform, index); }
		mat4 transform;
		vec3 halfSize;
	};

	Graphics4::VertexBuffer* boxVertexBuffer;
	Graphics4::IndexBuffer* boxIndexBuffer;
	Graphics4::Texture* boxTexture;

	vec3 transformInverse(const mat4& transform, const vec3& vector) {
		vec3 tmp = vector;
		tmp.x() -= transform.data[3];
		tmp.y() -= transform.data[7];
		tmp.z() -= transform.data[11];
		return vec3(
			tmp.x() * transform.data[0] + tmp.y() * transform.data[4] + tmp.z() * transform.data[8],
			tmp.x() * transform.data[1] + tmp.y() * transform.data[5] + tmp.z() * transform.data[9],
			tmp.x() * transform.data[2] + tmp.y() * transform.data[6] + tmp.z() * transform.data[10]
		);
	}

	bool boxAndPoint(const Box& box, const vec3& point, vec3& contactNormal, vec3& contactPoint, float& contatctDepth) {
		vec3 relPt = transformInverse(box.transform, point);
		vec3 normal;

		float min_depth = box.halfSize.x() - Kore::abs(relPt.x());
		if (min_depth < 0) return false;
		normal = box.getAxis(0) * ((relPt.x() < 0) ? -1.0f : 1.0f);

		float depth = box.halfSize.y() - Kore::abs(relPt.y());
		if (depth < 0) return false;
		else if (depth < min_depth) {
			min_depth = depth;
			normal = box.getAxis(1) * ((relPt.y() < 0) ? -1.0f : 1.0f);
		}

		depth = box.halfSize.z() - Kore::abs(relPt.z());
		if (depth < 0) return false;
		else if (depth < min_depth) {
			min_depth = depth;
			normal = box.getAxis(2) * ((relPt.z() < 0) ? -1.0f : 1.0f);
		}

		contactNormal = normal;
		contactPoint = point;
		contatctDepth = min_depth;

		return true;
	}

	Box boxes[256];

	Kore::Graphics4::VertexBuffer** vertexBuffers;
	MeshObject* body;
	MeshObject* leg;
	MeshObject* feeler;

#ifdef NDEBUG
	const int maxAnts = 500;
#else
	const int maxAnts = 50;
#endif
	Ant ants[maxAnts];
	float* scent;
	const int scents = 100;
    int antsDead = 0;
	
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

	int count = 0;
	int collisionObjects = 0;
	
	void collisionDetection(MeshObject* object) {
		if (object == nullptr) return;
		
		for (int j = 0; j < object->meshesCount; ++j) {
			Mesh* mesh = object->meshes[j];
			if (false) { //collisionObjects == 28 || collisionObjects == 31 || collisionObjects == 44 || collisionObjects == 45 || collisionObjects == 46) {
				boxes[collisionObjects].transform = mat4::Translation(-1000, -1000, -1000).Transpose();
				boxes[collisionObjects].halfSize = vec3(0, 0, 0);
			}
			else {
				boxes[collisionObjects].transform = mat4::Translation(mesh->xmin + (mesh->xmax - mesh->xmin) / 2.0f, mesh->ymin + (mesh->ymax - mesh->ymin) / 2.0f, mesh->zmin + (mesh->zmax - mesh->zmin) / 2.0f).Transpose();
				boxes[collisionObjects].halfSize = vec3((mesh->xmax - mesh->xmin) / 2.0f, (mesh->ymax - mesh->ymin) / 2.0f, (mesh->zmax - mesh->zmin) / 2.0f);
			}
			++collisionObjects;
		 }
	}
}

Ant::Ant() : mode(Floor) {
	rotation = mat4::Identity();
	forward = vec4(0, 0, -1, 0);
	right = vec4(1, 0, 0, 0);
	up = vec4(0, 1, 0, 0);
}

void Ant::init() {
	Random::init(System::timestamp());

	scent = new float[scents * scents * scents];
	for (int i = 0; i < scents * scents * scents; ++i) {
		scent[i] = Random::get(100) / 200.0f;
	}

	Graphics4::VertexStructure** structures = new Graphics4::VertexStructure*[2];
	structures[0] = new Graphics4::VertexStructure();
	structures[0]->add("pos", Graphics4::Float3VertexData);
	structures[0]->add("tex", Graphics4::Float2VertexData);
	structures[0]->add("nor", Graphics4::Float3VertexData);

	structures[1] = new Graphics4::VertexStructure();
	structures[1]->add("M", Graphics4::Float4x4VertexData);
	structures[1]->add("N", Graphics4::Float4x4VertexData);
	structures[1]->add("tint", Graphics4::Float4VertexData);

	body = new MeshObject("ant/AntBody.ogex", "ant/", *structures[0], 10);
	leg = new MeshObject("ant/AntLeg.ogex", "ant/", *structures[0], 10);
	feeler = new MeshObject("ant/AntFeeler.ogex", "ant/", *structures[0], 10);

	vertexBuffers = new Graphics4::VertexBuffer*[2];
	vertexBuffers[0] = body->vertexBuffers[0];
	vertexBuffers[1] = new Graphics4::VertexBuffer(maxAnts, *structures[1], 1);

	for (int i = 0; i < maxAnts; ++i) {
		vec3 start(0, 0.0f, 0);
		ants[i].position = vec3(start.x() + Random::get(-100, 100) / 50.0f, start.y(), start.z() + Random::get(-100, 100) / 50.0f); // vec3(Random::get(-100, 100) / 10.0f, -1, Random::get(-100, 100) / 10.0f);
		//ants[i].rotation = Quaternion(ants[i].right, Random::get(3000.0f) / 1000.0f).matrix() * ants[i].rotation;
        
        ants[i].energy = 0;
        ants[i].dead = false;
		float value = Random::get(-100.0f, 100.0f) / 10.0f;
		ants[i].forward = vec4(Kore::sin(value), 0.0f, Kore::cos(value), 1.0f);
		ants[i].rotation = Quaternion(vec3(0, 1, 0), pi / -2.0f + Kore::atan2(ants[i].forward.z(), ants[i].forward.x())).matrix() * Quaternion(vec3(1, 0, 0), pi / 2.0f).matrix();
	}

	collisionObjects = 0;
	for (int i = 0; i < maxObjects; ++i) {
		KitchenObject* kitchenObj = objects[i];
		if (kitchenObj == nullptr) continue;
		
		// TODO: Robert check this
		collisionDetection(kitchenObj->getBody());
		collisionDetection(kitchenObj->getClosedDoor());
		collisionDetection(kitchenObj->getOpenDoor());
	}

	collisionObjects = 0;
	boxes[collisionObjects].transform = mat4::Translation(0, 0, 0).Transpose();
	boxes[collisionObjects].halfSize = vec3(5, 5, 5);
	collisionObjects = 9;

	boxes[0].transform = mat4::Translation(-0.2f, 0, 0).Transpose();
	boxes[0].halfSize = vec3(2.6f, 1.394f, 1);
	boxes[1].transform = mat4::Translation(3, 0, 0).Transpose();
	boxes[1].halfSize = vec3(0.7f, 2.9f, 1.015f);
	boxes[2].transform = mat4::Translation(0.75f, 2.9f, 0).Transpose();
	boxes[2].halfSize = vec3(3.0f, 0.6f, 0.665f);
	boxes[3].transform = mat4::Translation(0, 0, -0.999f).Transpose();
	boxes[3].halfSize = vec3(10, 10, 1);
	boxes[4].transform = mat4::Translation(0, -1.0f, 0).Transpose();
	boxes[4].halfSize = vec3(10, 1, 10);
	boxes[5].transform = mat4::Translation(0, 7.0f, 0).Transpose();
	boxes[5].halfSize = vec3(10, 1, 10);
	boxes[6].transform = mat4::Translation(8.999f, 0, 0).Transpose();
	boxes[6].halfSize = vec3(1, 10, 10);
	boxes[7].transform = mat4::Translation(0, 0, 10.99f).Transpose();
	boxes[7].halfSize = vec3(10, 10, 1);
	boxes[8].transform = mat4::Translation(-8.99f, 0, 0).Transpose();
	boxes[8].halfSize = vec3(1, 10, 10);
	boxes[9].transform = mat4::Translation(0, 0, 0).Transpose();
	boxes[9].halfSize = vec3(0, 0, 0);

	//boxes[collisionObjects].transform = mat4::Translation(0, -1, 0).Transpose();
	//boxes[collisionObjects].halfSize = vec3(100, 1, 100);
	//++collisionObjects;
	//boxes[1].transform = mat4::Translation(4, 0, 0).Transpose();
	//boxes[1].halfSize = vec3(1.5f, 1.5f, 1.5f);

	//collisionObjects = 47;
	//collisionObjects = 15;

	boxVertexBuffer = new Graphics4::VertexBuffer(24, *structures[0]);
	float* vertices = boxVertexBuffer->lock();
	int i = 0;
	vertices[i++] = -1; vertices[i++] = -1; vertices[i++] = -1; vertices[i++] = 0; vertices[i++] = 0; vertices[i++] = -1; vertices[i++] = 0; vertices[i++] =  0;
	vertices[i++] = -1; vertices[i++] = -1; vertices[i++] =  1; vertices[i++] = 0; vertices[i++] = 0; vertices[i++] = -1; vertices[i++] = 0; vertices[i++] =  0;
	vertices[i++] = -1; vertices[i++] =  1; vertices[i++] = -1; vertices[i++] = 0; vertices[i++] = 0; vertices[i++] = -1; vertices[i++] = 0; vertices[i++] =  0;
	vertices[i++] = -1; vertices[i++] =  1; vertices[i++] =  1; vertices[i++] = 0; vertices[i++] = 0; vertices[i++] = -1; vertices[i++] = 0; vertices[i++] =  0;

	vertices[i++] = -1; vertices[i++] =  1; vertices[i++] = -1; vertices[i++] = 0; vertices[i++] = 0; vertices[i++] =  0; vertices[i++] = 0; vertices[i++] = -1;
	vertices[i++] =  1; vertices[i++] =  1; vertices[i++] = -1; vertices[i++] = 0; vertices[i++] = 0; vertices[i++] =  0; vertices[i++] = 0; vertices[i++] = -1;
	vertices[i++] = -1; vertices[i++] = -1; vertices[i++] = -1; vertices[i++] = 0; vertices[i++] = 0; vertices[i++] =  0; vertices[i++] = 0; vertices[i++] = -1;
	vertices[i++] =  1; vertices[i++] = -1; vertices[i++] = -1; vertices[i++] = 0; vertices[i++] = 0; vertices[i++] =  0; vertices[i++] = 0; vertices[i++] = -1;

	vertices[i++] =  1; vertices[i++] = -1; vertices[i++] = -1; vertices[i++] = 0; vertices[i++] = 0; vertices[i++] =  1; vertices[i++] = 0; vertices[i++] =  0;
	vertices[i++] =  1; vertices[i++] =  1; vertices[i++] = -1; vertices[i++] = 0; vertices[i++] = 0; vertices[i++] =  1; vertices[i++] = 0; vertices[i++] =  0;
	vertices[i++] =  1; vertices[i++] =  1; vertices[i++] =  1; vertices[i++] = 0; vertices[i++] = 0; vertices[i++] =  1; vertices[i++] = 0; vertices[i++] =  0;
	vertices[i++] =  1; vertices[i++] = -1; vertices[i++] =  1; vertices[i++] = 0; vertices[i++] = 0; vertices[i++] =  1; vertices[i++] = 0; vertices[i++] =  0;

	vertices[i++] = -1; vertices[i++] =  1; vertices[i++] =  1; vertices[i++] = 0; vertices[i++] = 0; vertices[i++] = 0; vertices[i++] = 0; vertices[i++] =  1;
	vertices[i++] =  1; vertices[i++] =  1; vertices[i++] =  1; vertices[i++] = 0; vertices[i++] = 0; vertices[i++] = 0; vertices[i++] = 0; vertices[i++] =  1;
	vertices[i++] = -1; vertices[i++] = -1; vertices[i++] =  1; vertices[i++] = 0; vertices[i++] = 0; vertices[i++] = 0; vertices[i++] = 0; vertices[i++] =  1;
	vertices[i++] =  1; vertices[i++] = -1; vertices[i++] =  1; vertices[i++] = 0; vertices[i++] = 0; vertices[i++] = 0; vertices[i++] = 0; vertices[i++] =  1;

	vertices[i++] = -1; vertices[i++] = -1; vertices[i++] = -1; vertices[i++] = 0; vertices[i++] = 0; vertices[i++] = 0; vertices[i++] = -1; vertices[i++] = 0;
	vertices[i++] = -1; vertices[i++] = -1; vertices[i++] =  1; vertices[i++] = 0; vertices[i++] = 0; vertices[i++] = 0; vertices[i++] = -1; vertices[i++] = 0;
	vertices[i++] =  1; vertices[i++] = -1; vertices[i++] =  1; vertices[i++] = 0; vertices[i++] = 0; vertices[i++] = 0; vertices[i++] = -1; vertices[i++] = 0;
	vertices[i++] =  1; vertices[i++] = -1; vertices[i++] = -1; vertices[i++] = 0; vertices[i++] = 0; vertices[i++] = 0; vertices[i++] = -1; vertices[i++] = 0;

	vertices[i++] = -1; vertices[i++] =  1; vertices[i++] = -1; vertices[i++] = 0; vertices[i++] = 0; vertices[i++] = 0; vertices[i++] =  1; vertices[i++] = 0;
	vertices[i++] = -1; vertices[i++] =  1; vertices[i++] =  1; vertices[i++] = 0; vertices[i++] = 0; vertices[i++] = 0; vertices[i++] =  1; vertices[i++] = 0;
	vertices[i++] =  1; vertices[i++] =  1; vertices[i++] =  1; vertices[i++] = 0; vertices[i++] = 0; vertices[i++] = 0; vertices[i++] =  1; vertices[i++] = 0;
	vertices[i++] =  1; vertices[i++] =  1; vertices[i++] = -1; vertices[i++] = 0; vertices[i++] = 0; vertices[i++] = 0; vertices[i++] =  1; vertices[i++] = 0;

	boxVertexBuffer->unlock();

	boxIndexBuffer = new Graphics4::IndexBuffer(6 * 6);
	int* indices = boxIndexBuffer->lock();
	i = 0;
	indices[i++] = 0; indices[i++] = 1; indices[i++] = 2;
	indices[i++] = 1; indices[i++] = 3; indices[i++] = 2;

	indices[i++] = 4; indices[i++] = 5; indices[i++] = 6;
	indices[i++] = 5; indices[i++] = 7; indices[i++] = 6;

	indices[i++] =  8; indices[i++] =  9; indices[i++] = 10;
	indices[i++] = 10; indices[i++] = 11; indices[i++] = 8;

	indices[i++] = 12; indices[i++] = 13; indices[i++] = 14;
	indices[i++] = 13; indices[i++] = 15; indices[i++] = 14;

	indices[i++] = 16; indices[i++] = 17; indices[i++] = 18;
	indices[i++] = 18; indices[i++] = 19; indices[i++] = 16;

	indices[i++] = 20; indices[i++] = 21; indices[i++] = 22;
	indices[i++] = 22; indices[i++] = 23; indices[i++] = 20;
	boxIndexBuffer->unlock();

	boxTexture = new Graphics4::Texture("white.png");
}

void Ant::chooseScent(bool force) {
	return;
	vec3i grid = gridPosition(position);
	if (force || grid != lastGrid) {
		if (!force) {
			float scent = scentAt(grid.x(), grid.y(), grid.z());
			scent = Kore::min(scent + 0.2f, 1.0f);
			setScent(grid.x(), grid.y(), grid.z(), scent);
		}
		lastGrid = grid;
		vec3i nextGrid = gridPosition(position + vec3(forward.x(), forward.y(), forward.z()) * 1.0f);
		if (mode == Floor) {
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
					int i1 = i - 1 < 0 ? 7 : i - 1;
					int i2 = i;
					int i3 = i + 1 > 7 ? 0 : i + 1;
					if (nextGrid == neighbours[i1] || nextGrid == neighbours[i2] || nextGrid == neighbours[i3]) {
						maxScent = scent;
						vec3 pos = realPosition(neighbours[i]);
						vec3 forward = pos - position;
						forward = forward.normalize();
						//vec3 right = forward.cross(vec3(up.x(), up.y(), up.z()));
						vec3 right = vec3(up.x(), up.y(), up.z()).cross(forward);
						this->right = vec4(right.x(), right.y(), right.z(), 0);
						this->forward = vec4(forward.x(), forward.y(), forward.z(), 0);
						float angle = Kore::atan2(forward.z(), forward.x());
						rotation = Quaternion(vec3(up.x(), up.y(), up.z()), angle + pi / 2.0f).matrix();
					}
				}
			}
		}
		else if (mode == FrontWall) {
			vec3i neighbours[8];
			for (int i = 0; i < 8; ++i) neighbours[i] = grid;
			neighbours[0].x() -= 1;
			neighbours[0].y() += 1;
			neighbours[1].x() += 0;
			neighbours[1].y() += 1;
			neighbours[2].x() += 1;
			neighbours[2].y() += 1;
			neighbours[3].x() += 1;
			neighbours[3].y() += 0;
			neighbours[4].x() += 1;
			neighbours[4].y() -= 1;
			neighbours[5].x() += 0;
			neighbours[5].y() -= 1;
			neighbours[6].x() -= 1;
			neighbours[6].y() -= 1;
			neighbours[7].x() -= 1;
			neighbours[7].y() += 0;

			float maxScent = 0;
			for (int i = 0; i < 8; ++i) {
				float scent = scentAt(neighbours[i].x(), neighbours[i].y(), neighbours[i].z());
				if (scent > maxScent) {
					int i1 = i - 1 < 0 ? 7 : i - 1;
					int i2 = i;
					int i3 = i + 1 > 7 ? 0 : i + 1;
					if (nextGrid == neighbours[i1] || nextGrid == neighbours[i2] || nextGrid == neighbours[i3]) {
						maxScent = scent;
						vec3 pos = realPosition(neighbours[i]);
						vec3 forward = pos - position;
						forward = forward.normalize();
						up = vec4(0, 0, -1, 0);
						vec3 right = forward.cross(vec3(up.x(), up.y(), up.z()));
						//vec3 right = vec3(up.x(), up.y(), up.z()).cross(forward);
						this->right = vec4(right.x(), right.y(), right.z(), 0);
						this->forward = vec4(forward.x(), forward.y(), forward.z(), 0);
						float angle = Kore::atan2(forward.y(), forward.x());
						rotation = Quaternion(vec3(up.x(), up.y(), up.z()), angle + pi / 2.0f).matrix() * Quaternion(vec3(1, 0, 0), pi / 2.0f).matrix();
					}
				}
			}
		}
		else if (mode == BackWall) {
			vec3i neighbours[8];
			for (int i = 0; i < 8; ++i) neighbours[i] = grid;
			neighbours[0].x() -= 1;
			neighbours[0].y() += 1;
			neighbours[1].x() += 0;
			neighbours[1].y() += 1;
			neighbours[2].x() += 1;
			neighbours[2].y() += 1;
			neighbours[3].x() += 1;
			neighbours[3].y() += 0;
			neighbours[4].x() += 1;
			neighbours[4].y() -= 1;
			neighbours[5].x() += 0;
			neighbours[5].y() -= 1;
			neighbours[6].x() -= 1;
			neighbours[6].y() -= 1;
			neighbours[7].x() -= 1;
			neighbours[7].y() += 0;

			float maxScent = 0;
			for (int i = 0; i < 8; ++i) {
				float scent = scentAt(neighbours[i].x(), neighbours[i].y(), neighbours[i].z());
				if (scent > maxScent) {
					int i1 = i - 1 < 0 ? 7 : i - 1;
					int i2 = i;
					int i3 = i + 1 > 7 ? 0 : i + 1;
					if (nextGrid == neighbours[i1] || nextGrid == neighbours[i2] || nextGrid == neighbours[i3]) {
						maxScent = scent;
						vec3 pos = realPosition(neighbours[i]);
						vec3 forward = pos - position;
						forward = forward.normalize();
						up = vec4(0, 0, -1, 0);
						vec3 right = forward.cross(vec3(up.x(), up.y(), up.z()));
						//vec3 right = vec3(up.x(), up.y(), up.z()).cross(forward);
						this->right = vec4(right.x(), right.y(), right.z(), 0);
						this->forward = vec4(forward.x(), forward.y(), forward.z(), 0);
						float angle = Kore::atan2(forward.y(), forward.x());
						rotation = Quaternion(vec3(up.x(), up.y(), up.z()), angle + pi / 2.0f).matrix() * Quaternion(vec3(1, 0, 0), pi / 2.0f).matrix();
					}
				}
			}
		}
	}
}

void Ant::morePizze(Kore::vec3 position) {
	vec3i pos = gridPosition(position);
	for (int x = pos.x() - 5; x <= pos.x() + 5; ++x) {
		for (int y = pos.y() - 5; y <= pos.y() + 5; ++y) {
			for (int z = pos.z() - 5; z <= pos.z() + 5; ++z) {
				setScent(x, y, z, scentAt(x, y, z) + 5.0f);
			}
		}
	}
}

void Ant::lessPizza(Kore::vec3 position) {
	vec3i pos = gridPosition(position);
	for (int x = pos.x() - 5; x <= pos.x() + 5; ++x) {
		for (int y = pos.y() - 5; y <= pos.y() + 5; ++y) {
			for (int z = pos.z() - 5; z <= pos.z() + 5; ++z) {
				setScent(x, y, z, scentAt(x, y, z) - 5.0f);
			}
		}
	}
}

//extern MeshObject* objects[];
//extern KitchenObject* kitchenObjects[];
//extern TriggerCollider* triggerCollider[];
//extern MeshObject* roomObjects[7];
extern KitchenObject* objects[];

void Ant::move(float deltaTime) {
	legRotation += 0.2f;
	bool flying = true;
	for (int i = 0; i < collisionObjects; ++i) {
		vec3 normal;
		vec3 contact;
		float depth;
		if (boxAndPoint(boxes[i], position + (up.xyz() * 0.1f) + (forward.xyz() * 0.004f), normal, contact, depth)) {
			mat4 newrotation = Quaternion(up.xyz().cross(normal), pi / -2.0f).matrix();
			forward = newrotation * forward;
			up = newrotation * up;
			right = newrotation * right;
			rotation = newrotation * rotation;
			return;
		}
		if (boxAndPoint(boxes[i], position + (up.xyz() * -0.1f), normal, contact, depth)) {
			flying = false;
			lastNormal = normal;
		}
	}
	if (flying) {
		//forward = vec4(0, 0, 0, 0);
		mat4 newrotation = Quaternion(up.xyz().cross(lastNormal), pi / -2.0f).matrix();
		forward = newrotation * forward;
		up = newrotation * up;
		right = newrotation * right;
		rotation = newrotation * rotation;
	}
	position += forward * 0.004f;
}

void Ant::moveEverybody(float deltaTime) {
	++count;
	/*if (count % 10 == 0) {
		Ant& ant = ants[Random::get(maxAnts - 1)];

		ant.rotation = mat4::Identity();
		ant.forward = vec4(0, 0, -1, 0);
		ant.right = vec4(1, 0, 0, 0);
		ant.up = vec4(0, 1, 0, 0);

		vec3 start(0, 1.5, 0);
		ant.position = vec3(start.x() + Random::get(-100, 100) / 100.0f, start.y(), start.z() + Random::get(-100, 100) / 100.0f); // vec3(Random::get(-100, 100) / 10.0f, -1, Random::get(-100, 100) / 10.0f);
																																	  //ants[i].rotation = Quaternion(ants[i].right, Random::get(3000.0f) / 1000.0f).matrix() * ants[i].rotation;
		ant.energy = 0;
		ant.dead = false;
	}*/

	for (int i = 0; i < maxAnts; ++i) {
		ants[i].move(deltaTime);
	}
}

bool Ant::intersects(vec3 dir) {
	if ((position + dir * 0.5f).y() <= -1) {
		return true;
	}
	/*for (unsigned oi = 0; kitchenObjects[oi] != nullptr; ++oi) {
		if (intersectsWith(kitchenObjects[oi]->body, dir) || intersectsWith(kitchenObjects[oi]->door_closed, dir)) {
			return true;
		}
	}
	for (int i = 0; roomObjects[i] != nullptr; ++i) {
		if (intersectsWith(roomObjects[i], dir)) {
			return true;
		}
	}*/
	return false;
}

/*bool Ant::intersectsWith(MeshObject* obj, vec3 dir) {
    if (obj == nullptr) return false;
    for (int k = 0; k < obj->colliderCount; ++k) {
        //float distance;
		//if (obj->collider[k] != nullptr && obj->collider[k]->IntersectsWith(position, dir, distance) && distance < .1f) {
		//for (float offset = 0.0f; offset < 10.0f; offset += 0.1f) {
		//	if (obj->collider[k] != nullptr && obj->collider[k]->IsInside(position + dir * offset) && dir.y() == -1) {
		//		return true;
		//	}
		//}

		if (obj->collider[k] != nullptr && obj->collider[k]->IsInside(position + dir * 1.0f)) {
			return true;
		}
    }
    return false;
}*/

bool Ant::isDying() {
    /*for (unsigned oi = 0; kitchenObjects[oi] != nullptr; ++oi) {
        if (kitchenObjects[oi]->triggerCollider != nullptr && kitchenObjects[oi]->triggerCollider->collider != nullptr && kitchenObjects[oi]->triggerCollider->collider->IsInside(position)) {
            if (kitchenObjects[oi]->closed)
                return true;
        }
    }*/
    return false;
}

namespace {
	void renderMesh(MeshObject* mesh, Kore::Graphics4::TextureUnit tex, Kore::Graphics4::ConstantLocation mLocation, Kore::Graphics4::ConstantLocation mLocationInverse, Kore::Graphics4::ConstantLocation diffuseLocation, Kore::Graphics4::ConstantLocation specularLocation, Kore::Graphics4::ConstantLocation specularPowerLocation) {
		for (int i = 0; i < mesh->meshesCount; ++i) {
			Geometry* geometry = mesh->geometries[i];
			mat4 modelMatrix = mesh->M * geometry->transform;
			mat4 modelMatrixInverse = modelMatrix.Invert();

			Graphics4::setMatrix(mLocation, modelMatrix);
			Graphics4::setMatrix(mLocationInverse, modelMatrixInverse);

			unsigned int materialIndex = geometry->materialIndex;
			Material* material = mesh->findMaterialWithIndex(materialIndex);
			if (material != nullptr) {
				Graphics4::setFloat3(diffuseLocation, material->diffuse);
				Graphics4::setFloat3(specularLocation, material->specular);
				Graphics4::setFloat(specularPowerLocation, material->specular_power);
			}
			else {
				Graphics4::setFloat3(diffuseLocation, vec3(1.0, 1.0, 1.0));
				Graphics4::setFloat3(specularLocation, vec3(1.0, 1.0, 1.0));
				Graphics4::setFloat(specularPowerLocation, 1.0);
			}

			Graphics4::Texture* image = mesh->images[i];
			if (image != nullptr) Graphics4::setTexture(tex, image);

			Graphics4::setVertexBuffer(*mesh->vertexBuffers[i]);
			Graphics4::setIndexBuffer(*mesh->indexBuffers[i]);
			Graphics4::drawIndexedVertices();
		}
	}
}

void Ant::render(Kore::Graphics4::TextureUnit tex, Kore::Graphics4::ConstantLocation mLocation, Kore::Graphics4::ConstantLocation mLocationInverse, Kore::Graphics4::ConstantLocation diffuseLocation, Kore::Graphics4::ConstantLocation specularLocation, Kore::Graphics4::ConstantLocation specularPowerLocation) { //Graphics4::ConstantLocation vLocation, Graphics4::TextureUnit tex, mat4 view) {
	for (int i = 0; i < maxAnts; ++i) {
		mat4 bodytrans = mat4::Translation(ants[i].position.x(), ants[i].position.y(), ants[i].position.z());
		mat4 bodyrotation = ants[i].rotation;
		float scale = 0.005f;
		mat4 bodyscale = mat4::Scale(scale, scale, scale);
		body->M = bodytrans * bodyrotation * bodyscale;
		renderMesh(body, tex, mLocation, mLocationInverse, diffuseLocation, specularLocation, specularPowerLocation);

		mat4 legrotation1 = Quaternion(vec3(1, 0, 0), Kore::sin(ants[i].legRotation)).matrix();
		mat4 legrotation2 = Quaternion(vec3(1, 0, 0), -Kore::sin(ants[i].legRotation)).matrix();
		
		leg->M = bodytrans * bodyrotation * bodyscale * mat4::Translation(4.2f, -2.2f, 4.6f) * legrotation1;
		renderMesh(leg, tex, mLocation, mLocationInverse, diffuseLocation, specularLocation, specularPowerLocation);
		leg->M = bodytrans * bodyrotation * bodyscale * mat4::Translation(4.0f, 0.0f, 4.3f) * legrotation2;
		renderMesh(leg, tex, mLocation, mLocationInverse, diffuseLocation, specularLocation, specularPowerLocation);
		leg->M = bodytrans * bodyrotation * bodyscale * mat4::Translation(4.0f, 2.4f, 4.1f) * legrotation1;
		renderMesh(leg, tex, mLocation, mLocationInverse, diffuseLocation, specularLocation, specularPowerLocation);

		mat4 leg2 = mat4::Scale(-1.0f, 1.0f, 1.0f);
		leg->M = bodytrans * bodyrotation * bodyscale * mat4::Translation(-4.2f, -2.2f, 4.6f) * legrotation2 * leg2;
		renderMesh(leg, tex, mLocation, mLocationInverse, diffuseLocation, specularLocation, specularPowerLocation);
		leg->M = bodytrans * bodyrotation * bodyscale * mat4::Translation(-4.0f, 0.0f, 4.3f) * legrotation1 * leg2;
		renderMesh(leg, tex, mLocation, mLocationInverse, diffuseLocation, specularLocation, specularPowerLocation);
		leg->M = bodytrans * bodyrotation * bodyscale * mat4::Translation(-4.0f, 2.4f, 4.1f) * legrotation2 * leg2;
		renderMesh(leg, tex, mLocation, mLocationInverse, diffuseLocation, specularLocation, specularPowerLocation);

		feeler->M = bodytrans * bodyrotation * bodyscale *mat4::Translation(1.0f, -8.0f, 7.0f);
		renderMesh(feeler, tex, mLocation, mLocationInverse, diffuseLocation, specularLocation, specularPowerLocation);
		feeler->M = bodytrans * bodyrotation * bodyscale *mat4::Translation(-1.0f, -8.0f, 7.0f) * mat4::Scale(-1.0f, 1.0f, 1.0f);
		renderMesh(feeler, tex, mLocation, mLocationInverse, diffuseLocation, specularLocation, specularPowerLocation);
	}

#ifdef DEBUG_COLLISIONS
	for (int i = 0; i < collisionObjects; ++i) {
		Box* box = &boxes[i];
		mat4 modelMatrix =  mat4::Translation(box->transform.data[3], box->transform.data[7], box->transform.data[11]) * mat4::Scale(box->halfSize.x(), box->halfSize.y(), box->halfSize.z());
		mat4 modelMatrixInverse = modelMatrix.Invert();

		Graphics4::setMatrix(mLocation, modelMatrix);
		Graphics4::setMatrix(mLocationInverse, modelMatrixInverse);

		Graphics4::setFloat3(diffuseLocation, vec3(1.0, 0.0, 1.0));
		Graphics4::setFloat3(specularLocation, vec3(1.0, 1.0, 1.0));
		Graphics4::setFloat(specularPowerLocation, 1.0);

		Graphics4::setTexture(tex, boxTexture);

		Graphics4::setVertexBuffer(*boxVertexBuffer);
		Graphics4::setIndexBuffer(*boxIndexBuffer);
		Graphics4::drawIndexedVertices();
	}
#endif
	/*int c = 0;
	{
		float* data = vertexBuffers[1]->lock();
		c = 0;
		for (int i = 0; i < maxAnts; i++) {
			const float scale = 0.02f;
			mat4 M = mat4::Translation(ants[i].position.x(), ants[i].position.y(), ants[i].position.z()) * ants[i].rotation * mat4::RotationY(pi) * mat4::Scale(scale, scale, scale);
			setMatrix(data, i, 0, 36, M);
			setMatrix(data, i, 16, 36, calculateN(M));
			setVec4(data, i, 32, 36, vec4(1, 1, 1, 1));
			c++;
		}
		vertexBuffers[1]->unlock();
	}

	Graphics4::setTexture(tex, body->image);
	Graphics4::VertexBuffer* vertexBuffers[2];
	vertexBuffers[0] = body->vertexBuffers[0];
	vertexBuffers[1] = ::vertexBuffers[1];
	Graphics4::setVertexBuffers(vertexBuffers, 2);
	Graphics4::setIndexBuffer(*body->indexBuffer);
	Graphics4::drawIndexedVerticesInstanced(c);

	vec3 legsOffset = vec3(0.044f, 0.035f, 0.0f);

	{
		float* data = vertexBuffers[1]->lock();
		c = 0;
		for (int i = 0; i < maxAnts; i++) {
			const float scale = 0.02f;
			//x = 0.461 , y = 0.461, z = 0.213
			mat4 M = mat4::Translation(ants[i].position.x(), ants[i].position.y(), ants[i].position.z()) * ants[i].rotation * mat4::RotationY(pi) * mat4::Translation(0.0461f + legsOffset.x(), 0.0461f + legsOffset.y(), 0.0213f + 0.023f + legsOffset.z()) * mat4::RotationX(ants[i].legRotation) * mat4::Scale(scale, scale, scale);
			setMatrix(data, i, 0, 36, M);
			setMatrix(data, i, 16, 36, calculateN(M));
			setVec4(data, i, 32, 36, vec4(1, 1, 1, 1));
			c++;
		}
		vertexBuffers[1]->unlock();
	}

	vertexBuffers[0] = leg->vertexBuffers[0];
	Graphics4::setVertexBuffers(vertexBuffers, 2);
	Graphics4::setIndexBuffer(*leg->indexBuffer);
	Graphics4::drawIndexedVerticesInstanced(c);

	{
		float* data = vertexBuffers[1]->lock();
		c = 0;
		// x = 0.422 , y = 0.414, z = -0.01
		for (int i = 0; i < maxAnts; i++) {
			const float scale = 0.02f;
			mat4 M = mat4::Translation(ants[i].position.x(), ants[i].position.y(), ants[i].position.z()) * ants[i].rotation * mat4::RotationY(pi) * mat4::Translation(0.0422f + legsOffset.x(), 0.0414f + legsOffset.y(), -0.001f + legsOffset.z()) * mat4::RotationX(-ants[i].legRotation) * mat4::Scale(scale, scale, scale);
			setMatrix(data, i, 0, 36, M);
			setMatrix(data, i, 16, 36, calculateN(M));
			setVec4(data, i, 32, 36, vec4(1, 1, 1, 1));
			c++;
		}
		vertexBuffers[1]->unlock();
	}

	vertexBuffers[0] = leg->vertexBuffers[0];
	Graphics4::setVertexBuffers(vertexBuffers, 2);
	Graphics4::setIndexBuffer(*leg->indexBuffer);
	Graphics4::drawIndexedVerticesInstanced(c);
	
	{
		float* data = vertexBuffers[1]->lock();
		c = 0;
		// x = 0.407, y = 0.381 , z = -0.244
		for (int i = 0; i < maxAnts; i++) {
			const float scale = 0.02f;
			mat4 M = mat4::Translation(ants[i].position.x(), ants[i].position.y(), ants[i].position.z()) * ants[i].rotation * mat4::RotationY(pi) * mat4::Translation(0.0407 + legsOffset.x(), 0.0381f + legsOffset.y(), -0.0244f - 0.028f + legsOffset.z()) * mat4::RotationX(ants[i].legRotation) * mat4::Scale(scale, scale, scale);
			setMatrix(data, i, 0, 36, M);
			setMatrix(data, i, 16, 36, calculateN(M));
			setVec4(data, i, 32, 36, vec4(1, 1, 1, 1));
			c++;
		}
		vertexBuffers[1]->unlock();
	}

	legsOffset.x() *= -1.0f;

	vertexBuffers[0] = leg->vertexBuffers[0];
	Graphics4::setVertexBuffers(vertexBuffers, 2);
	Graphics4::setIndexBuffer(*leg->indexBuffer);
	Graphics4::drawIndexedVerticesInstanced(c);

	{
		float* data = vertexBuffers[1]->lock();
		c = 0;
		// x = -0.461 , y = 0.461, z = 0.213
		for (int i = 0; i < maxAnts; i++) {
			const float scale = 0.02f;
			mat4 M = mat4::Translation(ants[i].position.x(), ants[i].position.y(), ants[i].position.z()) * ants[i].rotation * mat4::RotationY(pi) * mat4::Translation(-0.0461f + legsOffset.x(), 0.0461f + legsOffset.y(), 0.0213f + 0.023f + legsOffset.z()) * mat4::RotationX(-ants[i].legRotation) * mat4::RotationY(pi) * mat4::Scale(scale, scale, scale);
			setMatrix(data, i, 0, 36, M);
			setMatrix(data, i, 16, 36, calculateN(M));
			setVec4(data, i, 32, 36, vec4(1, 1, 1, 1));
			c++;
		}
		vertexBuffers[1]->unlock();
	}

	vertexBuffers[0] = leg->vertexBuffers[0];
	Graphics4::setVertexBuffers(vertexBuffers, 2);
	Graphics4::setIndexBuffer(*leg->indexBuffer);
	Graphics4::drawIndexedVerticesInstanced(c);

	{
		float* data = vertexBuffers[1]->lock();
		c = 0;
		// x = -0.422 , y = 0.414, z = -0.01
		for (int i = 0; i < maxAnts; i++) {
			const float scale = 0.02f;
			mat4 M = mat4::Translation(ants[i].position.x(), ants[i].position.y(), ants[i].position.z()) * ants[i].rotation * mat4::RotationY(pi) * mat4::Translation(-0.0422f + legsOffset.x(), 0.0414f + legsOffset.y(), -0.001f + legsOffset.z()) * mat4::RotationX(ants[i].legRotation) * mat4::RotationY(pi) * mat4::Scale(scale, scale, scale);
			setMatrix(data, i, 0, 36, M);
			setMatrix(data, i, 16, 36, calculateN(M));
			setVec4(data, i, 32, 36, vec4(1, 1, 1, 1));
			c++;
		}
		vertexBuffers[1]->unlock();
	}

	vertexBuffers[0] = leg->vertexBuffers[0];
	Graphics4::setVertexBuffers(vertexBuffers, 2);
	Graphics4::setIndexBuffer(*leg->indexBuffer);
	Graphics4::drawIndexedVerticesInstanced(c);
	
	{
		float* data = vertexBuffers[1]->lock();
		c = 0;
		// x = -0.407, y = 0.381 , z = -0.244
		for (int i = 0; i < maxAnts; i++) {
			const float scale = 0.02f;
			mat4 M = mat4::Translation(ants[i].position.x(), ants[i].position.y(), ants[i].position.z()) * ants[i].rotation * mat4::RotationY(pi) * mat4::Translation(-0.0407 + legsOffset.x(), 0.0381f + legsOffset.y(), -0.0244f - 0.028f + legsOffset.z()) * mat4::RotationX(-ants[i].legRotation) * mat4::RotationY(pi) * mat4::Scale(scale, scale, scale);
			setMatrix(data, i, 0, 36, M);
			setMatrix(data, i, 16, 36, calculateN(M));
			setVec4(data, i, 32, 36, vec4(1, 1, 1, 1));
			c++;
		}
		vertexBuffers[1]->unlock();
	}

	vertexBuffers[0] = leg->vertexBuffers[0];
	Graphics4::setVertexBuffers(vertexBuffers, 2);
	Graphics4::setIndexBuffer(*leg->indexBuffer);
	Graphics4::drawIndexedVerticesInstanced(c);*/
}

void Ant::setLights(Kore::Graphics4::ConstantLocation lightCountLocation, Kore::Graphics4::ConstantLocation lightPosLocation, MeshObject* room) {
	static const int maxLightCount = 10;
	Kore::vec4 lightPositions[maxLightCount];

	const int lightCount = (int)room->lights.size();
	for (int i = 0; i < lightCount; ++i) {
		Light* light = room->lights[i];
		lightPositions[i] = room->M * light->position;

		if (light->type == 0) {
			lightPositions[i].w() = 0;
		}
		else if (light->type == 1) {
			lightPositions[i].w() = 1;
		}
	}

	Graphics4::setInt(lightCountLocation, lightCount);
	Graphics4::setFloats(lightPosLocation, (float*)lightPositions, lightCount * 4);
}
