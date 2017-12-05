#pragma once

#include <Kore/Graphics4/Graphics.h>

#include "MeshObject.h"
#include "KitchenObject.h"

class Kitchen {
public:
	Kitchen();
	
	void render(Kore::Graphics4::TextureUnit tex, Kore::Graphics4::ConstantLocation mLocation, Kore::Graphics4::ConstantLocation mLocationInverse, Kore::Graphics4::ConstantLocation diffuseLocation, Kore::Graphics4::ConstantLocation specularLocation, Kore::Graphics4::ConstantLocation specularPowerLocation);
	void setLights(Kore::Graphics4::ConstantLocation lightCountLocation, Kore::Graphics4::ConstantLocation lightPosLocation);
	
	void highlightTheClosestObject(Kore::vec4 playerPosition);
	
	bool canOpen() const;
	void openTheDoor();
	
	void getRoomSize(float& xMin, float& xMax, float& yMin, float& yMax);
	
private:
	void init();
	
	KitchenObject* closestObj;
};

const int maxObjects = 20;
extern KitchenObject* objects[maxObjects];
