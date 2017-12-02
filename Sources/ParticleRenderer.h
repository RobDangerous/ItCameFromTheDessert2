//
//  CameraSystemManager.h
//  Korerorinpa
//
//  Created by KOM TU Darmstadt on 25.09.16.
//  Copyright (c) 2016 KTX Software Development. All rights reserved.
//

#pragma once

#include "pch.h"
#include <Kore/Math/Matrix.h>
#include "Engine/Particles.h"
#include <set>

using namespace Kore;

#include <vector>

class ParticleRenderer
{
public:
    ParticleRenderer(Kore::Graphics4::VertexStructure** structures);
    void render(Graphics4::TextureUnit tex, mat4 View, Graphics4::ConstantLocation vLocation);

    void addParticleSystem(ParticleSystem* system);
    void removeParticleSystem(ParticleSystem* system);
    Kore::Graphics4::VertexStructure** getStructures();
private:
    std::set<ParticleSystem*> particlesystems;
    Kore::Graphics4::VertexStructure** structures;
};
