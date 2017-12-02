#pragma once
#include "pch.h"

#include "Particles.h"

class Explosion : public ParticleSystem
{
public:
    Explosion(Kore::vec3 pos, float spawnArea, float grav, int maxParticles, Kore::Graphics4::VertexStructure** structures, Kore::Graphics4::Texture* image);
    bool isReady();
    void update(float deltaTime);
    void emitParticle(int index);
    void explode();
    void pulse();
private:
    bool exploded;
    bool ready;
};
