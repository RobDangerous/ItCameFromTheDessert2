#include "KitchenObject.h"

KitchenObject::KitchenObject(MeshObject** objects, int count, vec3 position, vec3 rotation) : objects(objects), count(count) {
    
    M = Kore::mat4::Translation(position.x(), position.y(), position.z());
    M *= Kore::mat4::Rotation(rotation.x(), rotation.y(), rotation.z());

}

void KitchenObject::render(TextureUnit tex, ConstantLocation mLocation) {
    for (int i = 0; i < count; ++i) {
        // set the model matrix
        Graphics::setMatrix(mLocation, M);
        objects[i]->render(tex);
    }


}
