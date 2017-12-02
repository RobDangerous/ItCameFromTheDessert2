#pragma once

#include <vector>

#include <Kore/Math/Vector.h>
#include <Kore/Audio1/Audio.h>

#include "Tank.h"
#include "Engine/InstancedMeshObject.h"
#include "Engine/Explosion.h"
#include "ParticleRenderer.h"
#include "Ground.h"

using namespace Kore;

#define MAX_TANKS 20//100

class TankSystem {
public:
	TankSystem(PhysicsWorld* world, ParticleRenderer* particleRenderer, InstancedMeshObject* meshB, InstancedMeshObject* meshT, InstancedMeshObject* meshF, vec3 spawn1a, vec3 spawn1b, vec3 spawn2a, vec3 spawn2b, float delay, Projectiles* projectiles, Graphics4::VertexStructure** structures, Ground* grnd);
	void initBars(vec2 halfSize, Graphics4::VertexStructure** structures);
	void update(float dt);
	void render(Graphics4::TextureUnit tex, mat4 View, Graphics4::ConstantLocation vLocation);
	void hover(vec3 cameraPosition, vec3 pickDir);
	void select(vec3 cameraPosition, vec3 pickDir);
	void issueCommand(vec3 cameraPosition, vec3 pickDir);
    void setMultipleSelect(bool val);
    void unselectTanks();
	
	int destroyed;
	int deserted;

private:
    bool multipleSelect;
    std::vector<Tank*> selectedTanks;
	Tank* hoveredTank;
    ParticleRenderer* particleRenderer;
    PhysicsWorld* world;
	float spawnDelay;
	float spawnTimer;
	vec3 spawnPos1a;
	vec3 spawnPos1b;
	vec3 spawnPos2a;
	vec3 spawnPos2b;
	InstancedMeshObject* meshBottom;
	InstancedMeshObject* meshTop;
	InstancedMeshObject* meshFlag;
	Graphics4::Texture* particleTexture;
	std::vector<Tank*> tanks;
    std::vector<Explosion*> explosions;
    std::vector<int> emptyIndices;
    Projectiles* mProjectiles;
	bool kill(int i);
	Tank* getHitTank(vec3 cameraPosition, vec3 pickDir);
	Ground* ground;

	Kore::Graphics4::VertexBuffer** vbs;
	Kore::Graphics4::IndexBuffer* ib;
	Kore::Graphics4::Texture* texture;
};
