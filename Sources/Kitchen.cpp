#include "pch.h"

#include "Kitchen.h"
#include "Ant.h"

#include <Kore/Audio1/Audio.h>
#include <Kore/Audio1/Sound.h>
#include <Kore/Log.h>
#include <Kore/Math/Random.h>

using namespace Kore;

KitchenObject* objects[maxObjects];

namespace {
	KitchenObject* fridge;
	KitchenObject* egg;
	KitchenObject* brokenEgg;
	KitchenObject* pizza;
	KitchenObject* pizzaDrawer;
	KitchenObject* cake;
	KitchenObject* roomfloor;
	KitchenObject* oven;
	KitchenObject* donut;
	KitchenObject* donutDrawer;
	bool egged = false;
	Sound* sound;
}

Kitchen::Kitchen() {
	init();
	sound = new Sound("ants.wav");
}

void Kitchen::init() {
	closestObj = nullptr;
	
	objects[0] = new KitchenObject("floor.ogex", nullptr, nullptr);
	objects[1] = new KitchenObject("wall0.ogex", nullptr, nullptr);
	objects[2] = new KitchenObject("wall1.ogex", nullptr, nullptr);
	objects[3] = new KitchenObject("wall2.ogex", nullptr, nullptr);
	objects[4] = new KitchenObject("wall3.ogex", nullptr, nullptr);
	objects[5] = new KitchenObject("wall4.ogex", nullptr, nullptr);
	
	objects[6] = new KitchenObject("fridge.ogex", "fridge_door.ogex", "fridge_door_open.ogex");
	
	objects[7] = new KitchenObject("lower_cupboard.ogex", nullptr, nullptr);
	objects[8] = new KitchenObject("upper_cupboard.ogex", "upper_cupboard_door.ogex", "upper_cupboard_door_open.ogex");
	objects[9] = new KitchenObject("table_chairs.ogex", nullptr, nullptr);
	
	objects[10] = new KitchenObject("broken_egg.ogex", nullptr, nullptr, 4, vec3(3.0, 0.0, 1.2));
	
	objects[11] = new KitchenObject("oven.ogex", "oven_door.ogex", "oven_door_open.ogex");
	objects[12] = new KitchenObject("drawer.ogex", "drawer_door.ogex", "drawer_door_open.ogex");
	objects[13] = new KitchenObject("sink.ogex", nullptr, nullptr);
	objects[14] = new KitchenObject("cake.ogex", nullptr, nullptr);
	objects[15] = new KitchenObject("eggs.ogex", nullptr, nullptr);
	objects[16] = new KitchenObject("pizza.ogex", nullptr, nullptr);
	objects[17] = new KitchenObject("door.ogex", nullptr, nullptr);
	objects[18] = new KitchenObject("window.ogex", nullptr, nullptr);
	objects[19] = new KitchenObject("lamp.ogex", nullptr, nullptr);
	objects[20] = new KitchenObject("credits.ogex", nullptr, nullptr);
	objects[21] = new KitchenObject("donut.ogex", nullptr, nullptr);

	fridge = objects[6];
	egg = objects[15];
	brokenEgg = objects[10];
	pizza = objects[16];
	pizza->visible = false;
	pizzaDrawer = objects[8];
	cake = objects[14];
	roomfloor = objects[0];
	oven = objects[11];
	donut = objects[21];
	donut->visible = false;
	donutDrawer = objects[12];
}

void Kitchen::render(Kore::Graphics4::TextureUnit tex, Kore::Graphics4::ConstantLocation mLocation, Kore::Graphics4::ConstantLocation mLocationInverse, Kore::Graphics4::ConstantLocation diffuseLocation, Kore::Graphics4::ConstantLocation specularLocation, Kore::Graphics4::ConstantLocation specularPowerLocation) {
	for (int i = 0; i < maxObjects; ++i) {
		KitchenObject* kitchenObj = objects[i];

		kitchenObj->update();
		if (kitchenObj == egg) {
			if (egg->getBody()->M.get(1, 3) < -1.8f) {
				egged = true;
			}
		}
		if (kitchenObj == cake) {
			//cake->R = mat4::RotationZ(0.01f) * cake->R; // deactivate because of weird lighting/normals
		}
		if (kitchenObj == pizza && pizza->dynamic) {
			if (pizza->getBody()->M.get(1, 3) < -0.9f) {
				pizza->dynamic = false;
				for (int j = currentAnts; j < currentAnts + 25; ++j) {
					ants[j].position = vec3(1.15f /*+ Random::get(0, 1000) / 1000.0f * 1.5f*/, 1.42f /*- Random::get(0, 1000) / 1000.0f*/, 0.665f + 0.3f);
					ants[j].energy = 0;
					ants[j].dead = false;
					ants[j].active = true;
					float value = Random::get(-100.0f, 100.0f) / 10.0f;
					ants[j].forward = vec4(Kore::sin(value), 0.0f, Kore::cos(value), 1.0f);
					ants[j].rotation = Quaternion(vec3(0, 1, 0), pi / -2.0f + Kore::atan2(ants[j].forward.z(), ants[j].forward.x())).matrix() * Quaternion(vec3(1, 0, 0), pi / 2.0f).matrix();
					ants[j].up = vec4(0, 1, 0, 1);
					ants[j].right = ants[j].forward.cross(ants[j].up);
				}
				Audio1::play(sound, true, 1.4f);
				currentAnts += 25;
			}
		}
		
		if (kitchenObj != nullptr && (kitchenObj != brokenEgg || egged) && (kitchenObj != egg || !egged) && kitchenObj->visible) {
			Graphics4::setTextureAddressing(tex, Graphics4::U, Graphics4::TextureAddressing::Repeat);
			Graphics4::setTextureAddressing(tex, Graphics4::V, Graphics4::TextureAddressing::Repeat);
			
			kitchenObj->renderMesh(tex, mLocation, mLocationInverse, diffuseLocation, specularLocation, specularPowerLocation);
		}
		
		if (kitchenObj == donut && donut->dynamic) {
			if (donut->getBody()->M.get(1, 3) < -0.85) {
				donut->dynamic = false;
				for (int j = currentAnts; j < currentAnts + 25; ++j) {
					vec4 pos = donut->getBody()->M * vec4(0, 0, 0, 1);
					ants[j].position = vec3(-2.25f, 0.0f, 1.85f);//vec3(1.15f /*+ Random::get(0, 1000) / 1000.0f * 1.5f*/, 1.42f /*- Random::get(0, 1000) / 1000.0f*/, 0.665f + 0.3f);
					ants[j].energy = 0;
					ants[j].dead = false;
					ants[j].active = true;
					float value = Random::get(-100.0f, 100.0f) / 10.0f;
					ants[j].forward = vec4(Kore::sin(value), 0.0f, Kore::cos(value), 1.0f);
					ants[j].rotation = Quaternion(vec3(0, 1, 0), pi / -2.0f + Kore::atan2(ants[j].forward.z(), ants[j].forward.x())).matrix() * Quaternion(vec3(1, 0, 0), pi / 2.0f).matrix();
					ants[j].up = vec4(0, 1, 0, 1);
					ants[j].right = ants[j].forward.cross(ants[j].up);
				}
				Audio1::play(sound, true, 0.8f);
				currentAnts += 25;
			}
		
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
	float minDist = 4.0f;
	
	closestObj = nullptr;
	
	for (int i = 0; i < maxObjects; ++i) {
		KitchenObject* kitchenObj = objects[i];
		
		if (kitchenObj != nullptr) {
			if (kitchenObj->getClosedDoor() == nullptr) continue;
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

extern vec3 cameraPos;

void Kitchen::openTheDoor() {
	for (int i = 0; i < maxObjects; ++i) {
		KitchenObject* kitchenObj = objects[i];
		
		if (kitchenObj == closestObj && kitchenObj->isHighlighted()) {
			kitchenObj->openDoor();
			if (kitchenObj == fridge && !fridge->activated) {
				fridge->activated = true;
				egg->speed = vec3(0.001f, 0.03f, 0.015f);
				egg->acc = vec3(0, -0.002f, 0);
				egg->dynamic = true;
				
				for (int i = currentAnts; i < currentAnts + 100; ++i) {
					ants[i].position = vec3(2.5f + 0.6f * Random::get(0, 1000) / 1000.0f, 2.5f * Random::get(0, 1000) / 1000.0f, 0.665f + 0.4f);
					ants[i].energy = 0;
					ants[i].dead = false;
					ants[i].active = true;
					float value = Random::get(-100.0f, 100.0f) / 10.0f;
					ants[i].forward = vec4(Kore::sin(value), Kore::cos(value), 0.0f, 1.0f);
					ants[i].rotation = Quaternion(vec3(0, 0, 1), pi / 2.0f - Kore::atan2(ants[i].forward.y(), ants[i].forward.x())).matrix() * Quaternion(vec3(0, 0, 1), pi).matrix();
					ants[i].up = vec4(0, 0, 1, 1);
					ants[i].right = ants[i].forward.cross(ants[i].up);
				}
				Audio1::play(sound, true);
				currentAnts += 100;
			}
			if (kitchenObj == pizzaDrawer && !pizzaDrawer->activated) {
				pizzaDrawer->activated = true;
				pizza->visible = true;
				pizza->speed = vec3(0.001f, 0.03f, 0.01f);
				pizza->acc = vec3(0, -0.002f, 0);
				pizza->dynamic = true;
			}
			if (kitchenObj == donutDrawer && !donutDrawer->activated) {
				donutDrawer->activated = true;
				donut->visible = true;
				donut->speed = vec3(0.001f, 0.05f, 0.01f);
				donut->acc = vec3(0, -0.002f, 0);
				donut->dynamic = true;
			}
			if (kitchenObj == oven && !oven->activated) {
				oven->activated = true;
				for (int i = currentAnts; i < currentAnts + 100; ++i) {
					ants[i].position = vec3(-1.9f + Random::get(0, 1000) / 1000.0f * 1.5f, 1.3f - Random::get(0, 1000) / 1000.0f, 0.665f + 0.3f);
					ants[i].energy = 0;
					ants[i].dead = false;
					ants[i].active = true;
					float value = Random::get(-100.0f, 100.0f) / 10.0f;
					ants[i].forward = vec4(Kore::sin(value), Kore::cos(value), 0.0f, 1.0f);
					ants[i].rotation = Quaternion(vec3(0, 0, 1), pi / 2.0f - Kore::atan2(ants[i].forward.y(), ants[i].forward.x())).matrix() * Quaternion(vec3(0, 0, 1), pi).matrix();
					ants[i].up = vec4(0, 0, 1, 1);
					ants[i].right = ants[i].forward.cross(ants[i].up);
				}
				Audio1::play(sound, true, 1.2f);
				currentAnts += 100;
			}
		}
	}
}

void Kitchen::getRoomSize(float& xMin, float& xMax, float& yMin, float& yMax) {
	vec4 pos = roomfloor->getBody()->geometries[0]->transform * vec4(0, 0, 0, 1);
	float scaleFac = 5;
	vec4 scale = vec4(scaleFac, 0, scaleFac, 1.0);
	
	vec4 max = pos + scale;
	xMax = max.x();
	yMax = max.z();
	
	vec4 min = pos - scale;
	xMin = min.x();
	yMin = min.z();
}
