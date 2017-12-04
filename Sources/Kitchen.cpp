#include "pch.h"
#include "Kitchen.h"

using namespace Kore;

KitchenObject* objects[maxObjects];


Kitchen::Kitchen() {
	init();
}

void Kitchen::init() {
	
	
	// Walls
	objects[0] = new KitchenObject("floor.ogex", nullptr, nullptr);
	objects[1] = new KitchenObject("walls.ogex", nullptr, nullptr);
	
	// Doors
	objects[2] = new KitchenObject("fridge.ogex", "fridge_door.ogex", "fridge_door_open.ogex");
	
	objects[3] = new KitchenObject("lower_cupboard.ogex", nullptr, nullptr);
	objects[4] = new KitchenObject("upper_cupboard.ogex", nullptr, nullptr);
	objects[5] = new KitchenObject("table_chairs.ogex", nullptr, nullptr);
	
	objects[6] = new KitchenObject("broken_egg.ogex", nullptr, nullptr, 4, vec3(3.0, 0.0, 1.2));
}

void Kitchen::render(Kore::Graphics4::TextureUnit tex, Kore::Graphics4::ConstantLocation mLocation, Kore::Graphics4::ConstantLocation mLocationInverse, Kore::Graphics4::ConstantLocation diffuseLocation, Kore::Graphics4::ConstantLocation specularLocation, Kore::Graphics4::ConstantLocation specularPowerLocation) {
	for (int i = 0; i < maxObjects; ++i) {
		KitchenObject* kitchenObj = objects[i];
		
		if (kitchenObj != nullptr) {
			Graphics4::setTextureAddressing(tex, Graphics4::U, Graphics4::TextureAddressing::Repeat);
			Graphics4::setTextureAddressing(tex, Graphics4::V, Graphics4::TextureAddressing::Repeat);
			
			kitchenObj->renderMesh(tex, mLocation, mLocationInverse, diffuseLocation, specularLocation, specularPowerLocation);
		}
	}
}

void Kitchen::setLights(Kore::Graphics4::ConstantLocation lightCountLocation, Kore::Graphics4::ConstantLocation lightPosLocation) {
	for (int i = 0; i < maxObjects; ++i) {
		KitchenObject* kitchenObj = objects[i];
		
		if (kitchenObj != nullptr) {
			
			kitchenObj->setLightsForMesh(lightCountLocation, lightPosLocation);
		}
	}
}
