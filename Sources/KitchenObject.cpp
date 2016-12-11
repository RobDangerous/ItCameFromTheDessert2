#include "KitchenObject.h"
#include <Kore/Math/Quaternion.h>

namespace {
	void setM(MeshObject* mesh, mat4 M) {
		if (mesh != nullptr) {
			mesh->M = M;
			for (int k = 0; k < mesh->colliderCount; ++k) {
				float distance;
				if (mesh->collider[k] != nullptr) {
					mesh->collider[k]->setPos(M * vec4(0, 0, 0, 1));
				}
			}
		}
	}
}

KitchenObject::KitchenObject(MeshObject* body, MeshObject* door, vec3 position, vec3 rotation, vec3 offset) : body(body), door(door), position(position), rotation(rotation), closed(true), offset(offset) {
    
    mat4 M = mat4::Translation(position.x(), position.y(), position.z());
    M *= mat4::Rotation(rotation.x(), rotation.y(), rotation.z());

	setM(body, M);
	setM(door, M);
}

void KitchenObject::render(TextureUnit tex, ConstantLocation mLocation) {
    
    if (body != nullptr) {
        body->render(tex, mLocation);
    }
    if (door != nullptr) {
        door->render(tex, mLocation);
    }
}

void KitchenObject::open() {
    if (door != nullptr && closed) {
        mat4 off = mat4::Translation(offset.x(), offset.y(), offset.z());
        off *= mat4::Rotation(rotation.x(), rotation.y(), rotation.z());
        
        mat4 T = mat4::Translation(position.x() - off.get(0,3), position.y() - off.get(1,3), position.z() - off.get(2,3));
        mat4 T_inv = mat4::Translation(off.get(0,3), off.get(1,3), off.get(2,3));
        //mat4 T_inv = mat4::Translation(0, 0, 1);
        mat4 R = mat4::Rotation(rotation.x() + pi/4.0f, rotation.y(), rotation.z());
        mat4 M =  T*R*T_inv;
        
        door->M = M;
        
        closed = false;
    }
}

void KitchenObject::close() {
    if (door != nullptr && !closed) {
        mat4 M = mat4::Translation(position.x(), position.y(), position.z());
        M *= mat4::Rotation(rotation.x(), rotation.y(), rotation.z());
        
        door->M = M;
        
        closed = true;
    }
}
