#include "KitchenObject.h"

#include <Kore/Log.h>


KitchenObject::KitchenObject(const char* meshBodyFile, const char* meshClosedDoorFile, const char* meshOpenDoorFile, const float scale, const vec3 position, const Quaternion rotation) : visible(true), activated(false), dynamic(false), speed(0, 0, 0), acc(0, -0.00001f, 0), closed(true), highlight(false), body(nullptr), door_open(nullptr), door_closed(nullptr) {
	
	M = mat4::Identity();
	M *= mat4::Translation(position.x(), position.y(), position.z());
	M *= mat4::Scale(scale, scale, scale);
	M *= rotation.matrix();

	R = mat4::Identity();
	
	Graphics4::VertexStructure* structures = new Graphics4::VertexStructure();
	structures->add("pos", Graphics4::Float3VertexData);
	structures->add("tex", Graphics4::Float2VertexData);
	structures->add("nor", Graphics4::Float3VertexData);
	
	char kitchenDir[100];
	std::strcpy(kitchenDir, "kitchen/");
	
	char bodyMeshDir[100];
	std::strcpy(bodyMeshDir, kitchenDir);
	std::strcat(bodyMeshDir, meshBodyFile);
	body = new MeshObject(bodyMeshDir, kitchenDir, *structures, 1);
	body->M = M;
	
	name = meshBodyFile;
	
	if (meshOpenDoorFile != nullptr) {
		char openDoorMeshDir[100];
		std::strcpy(openDoorMeshDir, kitchenDir);
		std::strcat(openDoorMeshDir, meshOpenDoorFile);
		door_open = new MeshObject(openDoorMeshDir, kitchenDir, *structures, 1);
		door_open->M = M;
	}
	
	if (meshClosedDoorFile != nullptr) {
		char closedDoorMeshDir[100];
		std::strcpy(closedDoorMeshDir, kitchenDir);
		std::strcat(closedDoorMeshDir, meshClosedDoorFile);
		door_closed = new MeshObject(closedDoorMeshDir, kitchenDir, *structures, 1);
		door_closed->M = M;
	}
}

void KitchenObject::renderMesh(Kore::Graphics4::TextureUnit tex, Kore::Graphics4::ConstantLocation mLocation, Kore::Graphics4::ConstantLocation mLocationInverse, Kore::Graphics4::ConstantLocation diffuseLocation, Kore::Graphics4::ConstantLocation specularLocation, Kore::Graphics4::ConstantLocation specularPowerLocation) {
	
	render(body, tex, mLocation, mLocationInverse, diffuseLocation, specularLocation, specularPowerLocation);
	
	if (closed) {
		render(door_closed, tex, mLocation, mLocationInverse, diffuseLocation, specularLocation, specularPowerLocation);
	} else {
		render(door_open, tex, mLocation, mLocationInverse, diffuseLocation, specularLocation, specularPowerLocation);
	}
	
	
}

bool KitchenObject::render(MeshObject* mesh, Kore::Graphics4::TextureUnit tex, Kore::Graphics4::ConstantLocation mLocation, Kore::Graphics4::ConstantLocation mLocationInverse, Kore::Graphics4::ConstantLocation diffuseLocation, Kore::Graphics4::ConstantLocation specularLocation, Kore::Graphics4::ConstantLocation specularPowerLocation) {
	
	if (mesh == nullptr) return false;
	
	for (int i = 0; i < mesh->meshesCount; ++i) {
		Geometry* geometry = mesh->geometries[i];
		mat4 modelMatrix = mesh->M * geometry->transform * R;
		mat4 modelMatrixInverse = modelMatrix.Invert();
		
		Graphics4::setMatrix(mLocation, modelMatrix);
		Graphics4::setMatrix(mLocationInverse, modelMatrixInverse);
		
		unsigned int materialIndex = geometry->materialIndex;
		Material* material = mesh->findMaterialWithIndex(materialIndex);
		if (material != nullptr) {
			if(highlight) Graphics4::setFloat3(diffuseLocation, material->diffuse + vec3(1, 0, 0));
			else Graphics4::setFloat3(diffuseLocation, material->diffuse);
			Graphics4::setFloat3(specularLocation, material->specular);
			Graphics4::setFloat(specularPowerLocation, material->specular_power);
		}
		else {
			Graphics4::setFloat3(diffuseLocation, vec3(1.0, 1.0, 1.0));
			Graphics4::setFloat3(specularLocation, vec3(1.0, 1.0, 1.0));
			Graphics4::setFloat(specularPowerLocation, 1.0);
		}
		
		Graphics4::Texture* image = mesh->images[i];
		Graphics4::setTexture(tex, image);
		
		Graphics4::setVertexBuffer(*mesh->vertexBuffers[i]);
		Graphics4::setIndexBuffer(*mesh->indexBuffers[i]);
		Graphics4::drawIndexedVertices();
	}
	
	return true;
}

void KitchenObject::setLightsForMesh(Kore::Graphics4::ConstantLocation lightCountLocation, Kore::Graphics4::ConstantLocation lightPosLocation) {
	static const int maxLightCount = 10;
	Kore::vec4 lightPositions[maxLightCount];
	
	const int lightCount = (int)body->lights.size();
	for (int i = 0; i < lightCount; ++i) {
		Light* light = body->lights[i];
		lightPositions[i] = body->M * light->position;
	 
		if (light->type == 0) {
			lightPositions[i].w() = 0;
		}
		else if (light->type == 1) {
			lightPositions[i].w() = 1;
		}
	}
	
	Graphics4::setInt(lightCountLocation, lightCount);
	Graphics4::setFloats(lightPosLocation, (float*)lightPositions, lightCount * 4);
}

void KitchenObject::openDoor() {
	closed = !closed;
}

bool KitchenObject::isClosed() const {
	return closed;
}

bool KitchenObject::isHighlighted() const {
	return highlight;
}

MeshObject* KitchenObject::getBody() const {
	return body;
}

MeshObject* KitchenObject::getOpenDoor() const {
	return door_open;
}

MeshObject* KitchenObject::getClosedDoor() const {
	return door_closed;
}

float KitchenObject::checkDistance(vec4 playerPosition) {
	vec4 meshPos = body->geometries[0]->transform * vec4(0, 0, 0, 1);
	//log(Info, "pos %f %f %f", meshPos.x(), meshPos.y(), meshPos.z());
	
	float dist = (meshPos - playerPosition).getLength();
	//log(Info, "dist %s %f", name, dist);
	
	return dist;
}

void KitchenObject::highlightKitchenObj(bool highlightObj) {
	// Highlight only objects that have doors, which can be opened
	if (door_open != nullptr)
		highlight = highlightObj;
}

void KitchenObject::update() {
	if (!dynamic) return;
	
	float x = body->M.get(0, 3);
	float y = body->M.get(1, 3);
	float z = body->M.get(2, 3);

	x += speed.x();
	y += speed.y();
	z += speed.z();

	speed.x() += acc.x();
	speed.y() += acc.y();
	speed.z() += acc.z();

	body->M.Set(0, 3, x);
	body->M.Set(1, 3, y);
	body->M.Set(2, 3, z);

	body->M = body->M;
	
	//log(Info, "Animate %s", name);
}
