#pragma once

#include "pch.h"
#include "Engine/MeshObject.h"

using namespace Kore;

class KitchenObject {
    
public:
    KitchenObject(MeshObject* body, MeshObject* door_closed, MeshObject* door_open, vec3 position, vec3 rotation);

    void render(TextureUnit tex, ConstantLocation mLocation);
    void open();
    void close();
    
    MeshObject* body;
    MeshObject* door_closed;
    MeshObject* door_open;
    
private:
    //vec3 position;
    //vec3 rotation;
    
    bool closed;
    vec3 offset;
    
    mat4 M;
    
};
