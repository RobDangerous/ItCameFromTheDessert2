#pragma once

#include <Kore/Math/Vector.h>
#include <Kore/Math/Quaternion.h>
#include <Kore/Graphics4/Graphics.h>

#include "MeshObject.h"
//#include "Engine/TriggerCollider.h"

class InstancedMeshObject;

enum AntMode { Floor, LeftWall, RightWall, FrontWall, BackWall, Ceiling };

class Ant {
public:
	static void init();
	Ant();
	void chooseScent(bool force);
	static void moveEverybody(float deltaTime);
	void move(float deltaTime);
	static void render(Kore::Graphics4::TextureUnit tex, Kore::Graphics4::ConstantLocation mLocation, Kore::Graphics4::ConstantLocation mLocationInverse, Kore::Graphics4::ConstantLocation diffuseLocation, Kore::Graphics4::ConstantLocation specularLocation, Kore::Graphics4::ConstantLocation specularPowerLocation);// Kore::Graphics4::ConstantLocation vLocation, Kore::Graphics4::TextureUnit tex, Kore::mat4 view);
	static void setLights(Kore::Graphics4::ConstantLocation lightCountLocation, Kore::Graphics4::ConstantLocation lightPosLocation, MeshObject* room);

	static void morePizze(Kore::vec3 position);
	static void lessPizza(Kore::vec3 position);

	Kore::vec3 position;
	Kore::vec4 forward, up, right;
	Kore::vec3 dir;
	Kore::mat4 rotation;
	Kore::vec3 lastNormal;
    
    float energy;
    bool dead;
	bool active;
    
	Kore::vec3i lastGrid;
	float legRotation;
	bool legRotationUp;
	AntMode mode;
private:
    //bool intersectsWith(MeshObject* obj, Kore::vec3 dir);
	bool intersects(Kore::vec3 dir);
    
    bool isDying();
};

const int maxAnts = 500;
extern int currentAnts;
extern Ant ants[maxAnts];
