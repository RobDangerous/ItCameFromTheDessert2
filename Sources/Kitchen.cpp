#include "pch.h"
#include "Kitchen.h"

#include <Kore/Log.h>

using namespace Kore;

KitchenObject* objects[maxObjects];


Kitchen::Kitchen() {
	init();
}

void Kitchen::init() {
	closestObj = nullptr;
	
	objects[0] = new KitchenObject("floor.ogex", nullptr, nullptr);
	objects[1] = new KitchenObject("walls.ogex", nullptr, nullptr);
	
	objects[2] = new KitchenObject("fridge.ogex", "fridge_door.ogex", "fridge_door_open.ogex");
	
	objects[3] = new KitchenObject("lower_cupboard.ogex", nullptr, nullptr);
	objects[4] = new KitchenObject("upper_cupboard.ogex", nullptr, nullptr);
	objects[5] = new KitchenObject("table_chairs.ogex", nullptr, nullptr);
	
	objects[6] = new KitchenObject("broken_egg.ogex", nullptr, nullptr, 4, vec3(3.0, 0.0, 1.2));
	
	objects[7] = new KitchenObject("oven.ogex", "oven_door.ogex", "oven_door_open.ogex");
	objects[8] = new KitchenObject("drawer.ogex", "drawer_door.ogex", "drawer_door_open.ogex");
	objects[9] = new KitchenObject("sink.ogex", nullptr, nullptr);
	objects[10] = new KitchenObject("cake.ogex", nullptr, nullptr);
	objects[11] = new KitchenObject("eggs.ogex", nullptr, nullptr);
	objects[12] = new KitchenObject("pizza.ogex", nullptr, nullptr);
	objects[13] = new KitchenObject("door.ogex", nullptr, nullptr);
	

	objects[10]->dynamic = true;
}

void Kitchen::render(Kore::Graphics4::TextureUnit tex, Kore::Graphics4::ConstantLocation mLocation, Kore::Graphics4::ConstantLocation mLocationInverse, Kore::Graphics4::ConstantLocation diffuseLocation, Kore::Graphics4::ConstantLocation specularLocation, Kore::Graphics4::ConstantLocation specularPowerLocation) {
	for (int i = 0; i < maxObjects; ++i) {
		KitchenObject* kitchenObj = objects[i];

		kitchenObj->update();
		
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
			break;
		}
	}
}

#ifndef MAXFLOAT
#define MAXFLOAT 9999999999999999
#endif

void Kitchen::highlightTheClosestObject(Kore::vec4 playerPosition) {
	float minDist = MAXFLOAT;
	
	closestObj = nullptr;
	
	for (int i = 0; i < maxObjects; ++i) {
		KitchenObject* kitchenObj = objects[i];
		
		if (kitchenObj != nullptr) {
			float dist = kitchenObj->checkDistance(playerPosition);
			
			if (dist < minDist) {
				minDist = dist;
				closestObj = kitchenObj;
			}
		}
	}
	
	for (int i = 0; i < maxObjects; ++i) {
		KitchenObject* kitchenObj = objects[i];
		
		if (kitchenObj == closestObj) {
			kitchenObj->highlightKitchenObj(true);
		} else {
			kitchenObj->highlightKitchenObj(false);
		}
	}
	//log(Info, "Closest Obj %s", closestObj->name);
}

bool Kitchen::canOpen() const {
	if (closestObj != nullptr && closestObj->isHighlighted())
		return true;
	else
		return false;
}

void Kitchen::openTheDoor() {
	for (int i = 0; i < maxObjects; ++i) {
		KitchenObject* kitchenObj = objects[i];
		
		if (kitchenObj == closestObj && kitchenObj->isHighlighted()) {
			kitchenObj->openDoor();
		}
	}
}
