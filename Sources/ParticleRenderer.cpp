//
//  CameraSystemManager.cpp
//  Korerorinpa
//
//  Created by KOM TU Darmstadt on 25.09.16.
//  Copyright (c) 2016 KTX Software Development. All rights reserved.
//

#include "ParticleRenderer.h"

ParticleRenderer::ParticleRenderer(Kore::Graphics4::VertexStructure** structures) :
    structures(structures)
{
}

void ParticleRenderer::render(Graphics4::TextureUnit tex, mat4 View, Graphics4::ConstantLocation vLocation)
{
    std::set<ParticleSystem*>::iterator it;
    for( it = particlesystems.begin(); it != particlesystems.end(); ++it)
    {
        (*it)->render(tex, vLocation, View);
    }
}

void ParticleRenderer::addParticleSystem( ParticleSystem* system )
{
    particlesystems.insert(system);
}

void ParticleRenderer::removeParticleSystem( ParticleSystem* system )
{
    particlesystems.erase(system);
}

Kore::Graphics4::VertexStructure** ParticleRenderer::getStructures()
{
    return structures;
}