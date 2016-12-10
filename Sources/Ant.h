#pragma once

#include <Kore/Math/Vector.h>
#include <Kore/Math/Quaternion.h>
#include <Kore/Graphics/Graphics.h>

class InstancedMeshObject;

class Ant {
public:
	Ant();
	void move();
	void render(Kore::ConstantLocation vLocation, Kore::TextureUnit tex, Kore::mat4 view);

	Kore::vec3 position;
	Kore::vec4 forward, up, right;
	Kore::mat4 rotation;

	Kore::VertexBuffer** vertexBuffers;
	InstancedMeshObject* body;
	int maxAnts;
};
