#pragma once

#include <Kore/Graphics4/Graphics.h>

#include "MeshObject.h"

class Kitchen {
public:
	static void init();
	Kitchen();
	
	static void render(Kore::Graphics4::TextureUnit tex, Kore::Graphics4::ConstantLocation mLocation, Kore::Graphics4::ConstantLocation mLocationInverse, Kore::Graphics4::ConstantLocation diffuseLocation, Kore::Graphics4::ConstantLocation specularLocation, Kore::Graphics4::ConstantLocation specularPowerLocation);// Kore::Graphics4::ConstantLocation vLocation, Kore::Graphics4::TextureUnit tex, Kore::mat4 view);
	static void setLights(Kore::Graphics4::ConstantLocation lightCountLocation, Kore::Graphics4::ConstantLocation lightPosLocation/*, MeshObject* room*/);
	
private:

};

const int maxObjects = 7;
extern MeshObject* objects[maxObjects];
