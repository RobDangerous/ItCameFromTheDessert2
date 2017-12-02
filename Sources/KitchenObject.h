#pragma once

#include "pch.h"
#include "Engine/MeshObject.h"
#include "Engine/TriggerCollider.h"

using namespace Kore;

class KitchenObject {
    
public:
    KitchenObject(MeshObject* body, MeshObject* door_closed, MeshObject* door_open, vec3 position, vec3 rotation, bool pizza = false);

	bool visible;
	bool pizza;
	Kore::vec3 readOnlyPos;
    void render(Graphics4::TextureUnit tex, Graphics4::ConstantLocation mLocation);
    void openOrClose(float time);
    void setTriggerCollider(TriggerCollider* triggerCollider);
    
    MeshObject* body;
    MeshObject* door_closed;
    MeshObject* door_open;
    
    bool closed;
    float lastTime;
    mat4 M;
    
    TriggerCollider* triggerCollider;
};
