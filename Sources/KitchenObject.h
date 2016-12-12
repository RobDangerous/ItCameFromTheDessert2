#pragma once

#include "pch.h"
#include "Engine/MeshObject.h"

using namespace Kore;

class KitchenObject {
    
public:
    KitchenObject(MeshObject* body, MeshObject* door_closed, MeshObject* door_open, vec3 position, vec3 rotation, bool pizza = false);

	bool visible;
	bool pizza;
    void render(TextureUnit tex, ConstantLocation mLocation);
    void openOrClose();
    mat4 getM();
    
    MeshObject* body;
    MeshObject* door_closed;
    MeshObject* door_open;
    
    bool closed;
    mat4 M;
};
