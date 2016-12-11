#pragma once

#include "pch.h"
#include "Engine/MeshObject.h"

using namespace Kore;

class KitchenObject {
    
public:
    //KitchenObject(MeshObject** objects, int count, vec3 position, vec3 rotation);
    KitchenObject(MeshObject* body, MeshObject* door, vec3 position, vec3 rotation);

    void render(TextureUnit tex, ConstantLocation mLocation);
    void open();
    
    //MeshObject** objects;
    //int count;
    
    MeshObject* body;
    MeshObject* door;
    
    mat4 M;
    
};
