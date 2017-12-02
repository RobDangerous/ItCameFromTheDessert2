#include "pch.h"

#include "Particles.h"

#include <Kore/Graphics4/Graphics.h>
#include <Kore/Math/Random.h>

#include "Rendering.h"

using namespace Kore;

ParticleSystem::ParticleSystem(vec3 pos, vec3 dir, float size, float timeToLive, vec4 colorS, vec4 colorE, float grav, int maxParticles, Graphics4::VertexStructure** structures, Graphics4::Texture* image) :
	colorStart(colorS),
	colorEnd(colorE),
	gravity(grav),
	totalTimeToLive(timeToLive),
	numParticles(maxParticles),
	texture (image) {

	particlePos = new vec3[maxParticles];
	particleVel = new vec3[maxParticles];
	particleTTL = new float[maxParticles];
	
	spawnRate = 0.05f;
	nextSpawn = spawnRate;
	spawnArea = 0.1;

	init(size / 2, maxParticles, structures);
	setPosition(pos);
	setDirection(dir);
}

void ParticleSystem::init(float halfSize, int maxParticles, Graphics4::VertexStructure** structures) {
	vbs = new Graphics4::VertexBuffer*[2];
	vbs[0] = new Graphics4::VertexBuffer(4, *structures[0], 0);
	float* vertices = vbs[0]->lock();
	setVertex(vertices, 0, -1 * halfSize, -1 * halfSize, 0, 0, 0);
	setVertex(vertices, 1, -1 * halfSize, 1 * halfSize, 0, 0, 1);
	setVertex(vertices, 2, 1 * halfSize, 1 * halfSize, 0, 1, 1);
	setVertex(vertices, 3, 1 * halfSize, -1 * halfSize, 0, 1, 0);
	vbs[0]->unlock();
	
	vbs[1] = new Graphics4::VertexBuffer(maxParticles, *structures[1], 1);

	// Set index buffer
	ib = new Graphics4::IndexBuffer(6);
	int* indices = ib->lock();
	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;
	indices[3] = 0;
	indices[4] = 2;
	indices[5] = 3;
	ib->unlock();
}

ParticleSystem::~ParticleSystem()
{
    delete vbs;
    delete ib;
    delete particlePos;
    delete particleVel;
    delete particleTTL;
}

void ParticleSystem::setPosition(vec3 position) {
	float b = spawnArea;
	emitMin = position + vec3(-b, -b, -b);
	emitMax = position + vec3(b, b, b);
}

void ParticleSystem::setDirection(vec3 direction) {
	emitDir = direction;
}

void ParticleSystem::update(float deltaTime) {
	// Do we need to spawn a particle?
	nextSpawn -= deltaTime;
	bool spawnParticle = false;
	if (nextSpawn < 0) {
		spawnParticle = true;
		nextSpawn = spawnRate;
	}

	for (int i = 0; i < numParticles; i++) {

		if (particleTTL[i] < 0.0f) {
			if (spawnParticle) {
				emitParticle(i);
				spawnParticle = false;
			}
		}

		particleTTL[i] -= deltaTime;
		particleVel[i] += vec3(0, -gravity * deltaTime, 0);
		particlePos[i] += particleVel[i] * deltaTime;
	}
}

void ParticleSystem::render(Graphics4::TextureUnit tex, Graphics4::ConstantLocation vLocation, mat4 V) {
	//**Graphics4::setRenderState(Graphics4::RenderState::DepthWrite, false);

	mat4 view = V.Invert();
	view.Set(0, 3, 0.0f);
	view.Set(1, 3, 0.0f);
	view.Set(2, 3, 0.0f);

	int alive = 0;
	float* data = vbs[1]->lock();
	for (int i = 0; i < numParticles; i++) {
		// Skip dead particles
		if (particleTTL[i] <= 0.0f) continue;
			
		mat4 M = mat4::Translation(particlePos[i].x(), particlePos[i].y(), particlePos[i].z()) * mat4::Scale(0.2f, 0.2f, 0.2f);
		
		setMatrix(data, alive, 0, 36, M * view);
		setMatrix(data, alive, 16, 36, calculateN(M * view));
		
		// Interpolate linearly between the two colors
		float interpolation = particleTTL[i] / totalTimeToLive;
		vec4 col = colorStart * interpolation + colorEnd * (1.0f - interpolation);
		setVec4(data, alive, 32, 36, col);

		++alive;
	}
	vbs[1]->unlock();
	
	Graphics4::setTexture(tex, texture);
	Graphics4::setVertexBuffers(vbs, 2);
	Graphics4::setIndexBuffer(*ib);
	Graphics4::drawIndexedVerticesInstanced(alive);

	//**Graphics4::setRenderState(RenderState::DepthWrite, true);
	//**Graphics4::setRenderState(RenderState::DepthTest, true);
}

void ParticleSystem::emitParticle(int index) {
	// Calculate a random position inside the box
	float x = getRandom(emitMin.x(), emitMax.x());
	float y = getRandom(emitMin.y(), emitMax.y());
	float z = getRandom(emitMin.z(), emitMax.z());

	particlePos[index].set(x, y, z);
	particleVel[index] = emitDir;
	particleTTL[index] = totalTimeToLive;
}

float ParticleSystem::getRandom(float minValue, float maxValue) {
	int randMax = 1000000;
	int randInt = Random::get(0, randMax);
	float r = (float)randInt / (float)randMax;
	return minValue + r * (maxValue - minValue);
}