#pragma once

#include <Kore/Math/Vector.h>
#include <Kore/Math/Quaternion.h>
#include <Kore/Graphics4/Graphics.h>

//#include "Engine/MeshObject.h"
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
	static void render(Kore::Graphics4::ConstantLocation vLocation, Kore::Graphics4::TextureUnit tex, Kore::mat4 view);

	static void morePizze(Kore::vec3 position);
	static void lessPizza(Kore::vec3 position);

	Kore::vec3 position;
	Kore::vec4 forward, up, right;
	Kore::vec3 dir;
	Kore::mat4 rotation;
    
    float energy;
    bool dead;
    
	Kore::vec3i lastGrid;
	float legRotation;
	bool legRotationUp;
	AntMode mode;
private:
    //bool intersectsWith(MeshObject* obj, Kore::vec3 dir);
	bool intersects(Kore::vec3 dir);
    
    bool isDying();
};
