#include "pch.h"
#include "Collision.h"
#include "MeshObject.h"

bool SphereCollider::IntersectsWith(TriangleMeshCollider& other) {
	TriangleCollider coll;
	int* current = other.mesh->mesh->indices;
	float* currentVertex = other.mesh->mesh->vertices;
	for (int i = 0; i < other.mesh->mesh->numFaces; i++) {
		coll.LoadFromBuffers(i, current, currentVertex);
		if (coll.Area() < 0.1f) continue;
		if (IntersectsWith(coll)) {
			other.lastCollision = i;
			Kore::vec3 normal;
			if (coll.GetNormal().x() < -0.8f)
				normal = coll.GetNormal();
			// Kore::log(Warning, "Intersected with triangle: %f, %f, %f", coll.GetNormal().x(), coll.GetNormal().y(), coll.GetNormal().z());
			return true;
		}
	}
	return false;
}

Kore::vec3 SphereCollider::GetCollisionNormal(const TriangleMeshCollider& other) {
	TriangleCollider coll;
	coll.LoadFromBuffers(other.lastCollision, other.mesh->mesh->indices, other.mesh->mesh->vertices);
	return coll.GetNormal();
}

float SphereCollider::PenetrationDepth(const TriangleMeshCollider& other) {
	// Get a collider for the plane of the triangle
	TriangleCollider coll;
	coll.LoadFromBuffers(other.lastCollision, other.mesh->mesh->indices, other.mesh->mesh->vertices);
	PlaneCollider plane = coll.GetPlane();


	return PenetrationDepth(plane);
}
