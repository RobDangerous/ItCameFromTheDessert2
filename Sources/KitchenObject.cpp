#include "KitchenObject.h"

#include <Kore/Log.h>

namespace {
	bool render(MeshObject* mesh, Kore::Graphics4::TextureUnit tex, Kore::Graphics4::ConstantLocation mLocation, Kore::Graphics4::ConstantLocation mLocationInverse, Kore::Graphics4::ConstantLocation diffuseLocation, Kore::Graphics4::ConstantLocation specularLocation, Kore::Graphics4::ConstantLocation specularPowerLocation) {
		
		if (mesh == nullptr) return false;
		
		for (int i = 0; i < mesh->meshesCount; ++i) {
			Geometry* geometry = mesh->geometries[i];
			mat4 modelMatrix = mesh->M * geometry->transform;
			mat4 modelMatrixInverse = modelMatrix.Invert();
			
			Graphics4::setMatrix(mLocation, modelMatrix);
			Graphics4::setMatrix(mLocationInverse, modelMatrixInverse);
			
			unsigned int materialIndex = geometry->materialIndex;
			Material* material = mesh->findMaterialWithIndex(materialIndex);
			if (material != nullptr) {
				Graphics4::setFloat3(diffuseLocation, material->diffuse);
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
}

KitchenObject::KitchenObject(const char* meshBodyFile, const char* meshClosedDoorFile, const char* meshOpenDoorFile, const float scale, const vec3 position, const Quaternion rotation) : closed(true), body(nullptr), door_open(nullptr), door_closed(nullptr) {
	
	M = mat4::Identity();
	M *= mat4::Translation(position.x(), position.y(), position.z());
	M *= mat4::Scale(scale, scale, scale);
	M *= rotation.matrix();
	
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

void KitchenObject::openDoor(bool open) {
	closed = !open;
}

bool KitchenObject::isClosed() const {
	return closed;
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
