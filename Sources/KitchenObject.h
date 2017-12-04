#pragma once

#include "pch.h"
#include "MeshObject.h"

#include <Kore/Math/Quaternion.h>

using namespace Kore;

class KitchenObject {
    
public:
    KitchenObject(const char* meshBodyFile, const char* meshClosedDoorFile, const char* meshOpenDoorFile, const float scale = 1.0, const vec3 position = vec3(0.0f, 0.0f, 0.0f), const Quaternion rotation = Quaternion(0.0f, 0.0f, 0.0f, 1.0f));
	
	void renderMesh(Kore::Graphics4::TextureUnit tex, Kore::Graphics4::ConstantLocation mLocation, Kore::Graphics4::ConstantLocation mLocationInverse, Kore::Graphics4::ConstantLocation diffuseLocation, Kore::Graphics4::ConstantLocation specularLocation, Kore::Graphics4::ConstantLocation specularPowerLocation);
	void setLightsForMesh(Kore::Graphics4::ConstantLocation lightCountLocation, Kore::Graphics4::ConstantLocation lightPosLocation);
	
	void openDoor(bool open);
	bool isClosed() const;
	
	MeshObject* getBody() const;
	MeshObject* getOpenDoor() const;
	MeshObject* getClosedDoor() const;
    
    mat4 M;
	
private:
	bool closed;
	
	MeshObject* body;
	MeshObject* door_closed;
	MeshObject* door_open;
	
};
