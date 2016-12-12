#include "KitchenObject.h"
#include <Kore/Math/Quaternion.h>

namespace {
	void setM(MeshObject* mesh, mat4 M) {
		if (mesh != nullptr) {
			for (int k = 0; k < mesh->colliderCount; ++k) {
				float distance;
				if (mesh->collider[k] != nullptr) {
					mesh->collider[k]-> trans(M);
				}
			}
		}
	}
}

KitchenObject::KitchenObject(MeshObject* body, MeshObject* door_closed, MeshObject* door_open, vec3 position, vec3 rotation) : body(body), door_closed(door_closed), door_open(door_open), closed(true) {
	M = mat4::Translation(position.x(), position.y(), position.z());
    M *= mat4::Rotation(rotation.x(), rotation.y(), rotation.z());

	setM(body, M * mat4::RotationY(pi * 0.5f));
	setM(door_closed, M * mat4::RotationY(pi * 0.5f));
}

void KitchenObject::render(TextureUnit tex, ConstantLocation mLocation) {
    Kore::Graphics::setMatrix(mLocation, M);
    if (body != nullptr) {
        body->render(tex, mLocation);
    }
    
    if (closed && door_closed != nullptr) {
        door_closed->render(tex, mLocation);
    } else if (!closed && door_open != nullptr) {
        door_open->render(tex, mLocation);
    }
}

void KitchenObject::openOrClose() {
    if (closed && door_open != nullptr) {
        /*mat4 T = mat4::Translation(position.x() - off.get(0,3), position.y() - off.get(1,3), position.z() - off.get(2,3));
        mat4 T_inv = mat4::Translation(off.get(0,3), off.get(1,3), off.get(2,3));
        mat4 R = mat4::Rotation(rotation.x() + pi/4.0f, rotation.y(), rotation.z());
        mat4 M =  T * R * T_inv;*/
        closed = false;
    } else if (!closed && door_closed != nullptr) {
        closed = true;
    }
}

mat4 KitchenObject::getM() {
    return M;
}
