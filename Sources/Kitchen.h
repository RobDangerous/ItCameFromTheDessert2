#pragma once

#include <Kore/Graphics4/Graphics.h>

#include "MeshObject.h"

class Kitchen {
public:
	Kitchen();
	
	void render(Kore::Graphics4::TextureUnit tex, Kore::Graphics4::ConstantLocation mLocation, Kore::Graphics4::ConstantLocation mLocationInverse, Kore::Graphics4::ConstantLocation diffuseLocation, Kore::Graphics4::ConstantLocation specularLocation, Kore::Graphics4::ConstantLocation specularPowerLocation);// Kore::Graphics4::ConstantLocation vLocation, Kore::Graphics4::TextureUnit tex, Kore::mat4 view);
	void setLights(Kore::Graphics4::ConstantLocation lightCountLocation, Kore::Graphics4::ConstantLocation lightPosLocation/*, MeshObject* room*/);
	
private:
	void init();
	
	bool openFridge;

};

const int maxObjects = 8;
extern MeshObject* objects[maxObjects];
