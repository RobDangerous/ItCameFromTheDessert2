#include "pch.h"
#include "Kitchen.h"

using namespace Kore;

namespace {
	const int maxObjects = 4;
	MeshObject* objects[maxObjects];
	
	void renderMesh(MeshObject* mesh, Kore::Graphics4::TextureUnit tex, Kore::Graphics4::ConstantLocation mLocation, Kore::Graphics4::ConstantLocation mLocationInverse, Kore::Graphics4::ConstantLocation diffuseLocation, Kore::Graphics4::ConstantLocation specularLocation, Kore::Graphics4::ConstantLocation specularPowerLocation) {
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
	}
}

Kitchen::Kitchen() {
}

void Kitchen::init() {
	Graphics4::VertexStructure* structures = new Graphics4::VertexStructure();
	structures->add("pos", Graphics4::Float3VertexData);
	structures->add("tex", Graphics4::Float2VertexData);
	structures->add("nor", Graphics4::Float3VertexData);
	
	objects[0] = new MeshObject("kitchen/fridge.ogex", "kitchen/", *structures, 1);
	objects[1] = new MeshObject("kitchen/lower_cupboard.ogex", "kitchen/", *structures, 1);
	objects[2] = new MeshObject("kitchen/upper_cupboard.ogex", "kitchen/", *structures, 1);
	
	MeshObject* obj = new MeshObject("kitchen/broken_egg.ogex", "kitchen/", *structures, 5);
	obj->M = mat4::Translation(3.0, 0.0, 0.2);
	objects[3] = obj;
}

void Kitchen::render(Kore::Graphics4::TextureUnit tex, Kore::Graphics4::ConstantLocation mLocation, Kore::Graphics4::ConstantLocation mLocationInverse, Kore::Graphics4::ConstantLocation diffuseLocation, Kore::Graphics4::ConstantLocation specularLocation, Kore::Graphics4::ConstantLocation specularPowerLocation) {
	for (int i = 0; i < maxObjects; ++i) {
		MeshObject* object = objects[i];
		renderMesh(object, tex, mLocation, mLocationInverse, diffuseLocation, specularLocation, specularPowerLocation);
	}
}

void Kitchen::setLights(Kore::Graphics4::ConstantLocation lightCountLocation, Kore::Graphics4::ConstantLocation lightPosLocation) {
	for (int i = 0; i < maxObjects; ++i) {
		MeshObject* object = objects[i];

		static const int maxLightCount = 10;
		Kore::vec4 lightPositions[maxLightCount];
		
		const int lightCount = (int)object->lights.size();
		for (int i = 0; i < lightCount; ++i) {
			Light* light = object->lights[i];
			lightPositions[i] = object->M * light->position;
			
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
}
