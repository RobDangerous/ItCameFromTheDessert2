#pragma once

#include "pch.h"
#include "Engine/MeshObject.h"

using namespace Kore;

class KitchenObject {
    
public:
    KitchenObject(MeshObject* body, MeshObject* door, vec3 position, vec3 rotation, vec3 offset = vec3(0.0f, 0.0f, 0.0f));

    void render(TextureUnit tex, ConstantLocation mLocation);
    void open();
    void close();
    
    MeshObject* body;
    MeshObject* door;
    
private:
    vec3 position;
    vec3 rotation;
    
    bool closed;
    vec3 offset;
    
};
