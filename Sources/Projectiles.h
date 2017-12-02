#pragma once

#include <set>

#include "Engine/Particles.h"
#include "Engine/PhysicsObject.h"
#include "Engine/PhysicsWorld.h"

#define PROJECTILE_SIZE 1
#define PROJECTILE_LIFETIME 3;

class Tank;

typedef struct
{
    int damage;
    Tank* source;
    PhysicsObject* dest;
} projectile_collision_data;

class Projectiles {
public:
	Projectiles(int maxProjectiles, float hitDistance, Kore::Graphics4::Texture* particleTex, MeshObject* mesh, Kore::Graphics4::VertexStructure** structures, PhysicsWorld* physics);
	int fire(vec3 pos, PhysicsObject* target, float s, int dmg, Tank* shooter);
	void update(float deltaT);
	void render(Kore::Graphics4::ConstantLocation vLocation, Kore::Graphics4::TextureUnit tex, Kore::mat4 view);
	void onShooterDeath(int projectileID);
	void remove(Tank* tank);
private:
	int maxProj;
	float hitDist;
	MeshObject* sharedMesh;
	Kore::Graphics4::VertexBuffer** vertexBuffers;

	// Projectiles
	float* timeToLife;
	projectile_collision_data* collision_data;
	Tank** shooters;
	PhysicsObject** physicsObject;
	ParticleSystem** particles;
	PhysicsObject** targets;
    std::set<int> inactiveProjectiles;

	void kill(int projectile, bool score);
};
