#pragma once

#include "pch.h"
#include "Engine/MeshObject.h"

using namespace Kore;

class KitchenObject {
    
public:
    KitchenObject(MeshObject** objects, int count, vec3 position, vec3 rotation);

    void render(TextureUnit tex, ConstantLocation mLocation);
    
    MeshObject** objects;
    int count;
    
    mat4 M;
};
