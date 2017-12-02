#include "KitchenObject.h"
#include <Kore/Math/Quaternion.h>

namespace {
	void setM(MeshObject* mesh, mat4 M) {
		if (mesh != nullptr) {
			for (int k = 0; k < mesh->colliderCount; ++k) {
				if (mesh->collider[k] != nullptr) {
					mesh->collider[k]-> trans(M);
				}
			}
		}
	}
}

KitchenObject::KitchenObject(MeshObject* body, MeshObject* door_closed, MeshObject* door_open, vec3 position, vec3 rotation, bool pizza) : body(body), door_closed(door_closed), door_open(door_open), pizza(pizza), readOnlyPos(position), visible(true), closed(true) {
	M = mat4::Translation(position.x(), position.y(), position.z());
    M *= mat4::Rotation(rotation.x(), rotation.y(), rotation.z());

	setM(body, M);
	setM(door_closed, M);

    triggerCollider = nullptr;
}

void KitchenObject::render(Graphics4::TextureUnit tex, Graphics4::ConstantLocation mLocation) {
	if (!visible) return;

    Kore::Graphics4::setMatrix(mLocation, M);
    if (body != nullptr) {
        body->render(tex, mLocation);
    }
    
    if (closed && door_closed != nullptr) {
        door_closed->render(tex, mLocation);
    } else if (!closed && door_open != nullptr) {
        door_open->render(tex, mLocation);
    }
}

void KitchenObject::openOrClose(float time) {
    float deltaT = time - lastTime;
    if (deltaT < 3) { // you can open the door only every x seconds
        log(Info, "Cool down: %f", deltaT);
        return;
    }
    
    if (closed && door_open != nullptr) {
        /*mat4 T = mat4::Translation(position.x() - off.get(0,3), position.y() - off.get(1,3), position.z() - off.get(2,3));
        mat4 T_inv = mat4::Translation(off.get(0,3), off.get(1,3), off.get(2,3));
        mat4 R = mat4::Rotation(rotation.x() + pi/4.0f, rotation.y(), rotation.z());
        mat4 M =  T * R * T_inv;*/
        closed = false;
    } else if (!closed && door_closed != nullptr) {
        closed = true;
    }
    
    lastTime = time;
}

void KitchenObject::setTriggerCollider(TriggerCollider* triggerCollider) {
    this->triggerCollider = triggerCollider;
}
