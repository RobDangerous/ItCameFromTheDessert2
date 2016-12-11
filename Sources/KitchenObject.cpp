#include "KitchenObject.h"

//KitchenObject::KitchenObject(MeshObject** objects, int count, vec3 position, vec3 rotation) : objects(objects), count(count) {
KitchenObject::KitchenObject(MeshObject* body, MeshObject* door, vec3 position, vec3 rotation) : body(body), door(door) {
    
    M = Kore::mat4::Translation(position.x(), position.y(), position.z());
    M *= Kore::mat4::Rotation(rotation.x(), rotation.y(), rotation.z());
}

void KitchenObject::render(TextureUnit tex, ConstantLocation mLocation) {
    /*for (int i = 0; i < count; ++i) {
        // set the model matrix
        Graphics::setMatrix(mLocation, M);
        objects[i]->render(tex);
    }*/
    
    Graphics::setMatrix(mLocation, M);
    if (body != nullptr) body->render(tex);
    if (door != nullptr) door->render(tex);

}

void KitchenObject::open() {
    
}
