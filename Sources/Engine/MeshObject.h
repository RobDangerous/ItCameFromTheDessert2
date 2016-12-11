#pragma once

#include <Kore/IO/FileReader.h>
#include <Kore/Math/Core.h>
#include <Kore/Math/Random.h>
#include <Kore/System.h>
#include <Kore/Input/Keyboard.h>
#include <Kore/Input/Mouse.h>
#include <Kore/Audio/Mixer.h>
#include <Kore/Graphics/Image.h>
#include <Kore/Graphics/Graphics.h>
#include <Kore/Log.h>

#include "Collision.h"
#include "ObjLoader.h"
#include "Rendering.h"
#include "CollLoader.h"
#include "assert.h"
#include "string.h"

class SphereCollider;

class MeshObject {
public:
	MeshObject(const char* meshFile, const char* textureFile, Kore::VertexStructure** structures, float scale = 1.0f) {
		for (int i = 0; i < colliderCount; ++i) collider[i] = nullptr;
		mesh = loadObj(meshFile);
		image = new Kore::Texture(textureFile, true);
		
		vertexBuffers = new Kore::VertexBuffer*[2];
		vertexBuffers[0] = new Kore::VertexBuffer(mesh->numVertices, *structures[0], 0);
		float* vertices = vertexBuffers[0]->lock();
		for (int i = 0; i < mesh->numVertices; ++i) {
			vertices[i * 8 + 0] = mesh->vertices[i * 8 + 0] * scale;
			vertices[i * 8 + 1] = mesh->vertices[i * 8 + 1] * scale;
			vertices[i * 8 + 2] = mesh->vertices[i * 8 + 2] * scale;
			vertices[i * 8 + 3] = mesh->vertices[i * 8 + 3];
			vertices[i * 8 + 4] = 1.0f - mesh->vertices[i * 8 + 4];
			vertices[i * 8 + 5] = mesh->vertices[i * 8 + 5];
			vertices[i * 8 + 6] = mesh->vertices[i * 8 + 6];
			vertices[i * 8 + 7] = mesh->vertices[i * 8 + 7];
		}
		vertexBuffers[0]->unlock();
		
		vertexBuffers[1] = new Kore::VertexBuffer(1, *structures[1], 1);

		indexBuffer = new Kore::IndexBuffer(mesh->numFaces * 3);
		int* indices = indexBuffer->lock();
		for (int i = 0; i < mesh->numFaces * 3; i++) {
			indices[i] = mesh->indices[i];
		}
		indexBuffer->unlock();

		Collider.center = Kore::vec3(0, 0, 0);
		Collider.radius = 1;
	}
    
    MeshObject(const char* meshFile, const char* colliderFile, const char* textureFile, const Kore::VertexStructure& structure, float scale) {
		for (int i = 0; i < colliderCount; ++i) collider[i] = nullptr;
        mesh = loadObj(meshFile);
        image = new Kore::Texture(textureFile, true);
		vertexBuffers = nullptr;
        
        // Mesh Vertex Buffer
        vertexBuffer = new Kore::VertexBuffer(mesh->numVertices, structure, 0);
        float* vertices = vertexBuffer->lock();
        
        for (int i = 0; i < mesh->numVertices; ++i) {
            vertices[i * 8 + 0] = mesh->vertices[i * 8 + 0] * scale;
            vertices[i * 8 + 1] = mesh->vertices[i * 8 + 1] * scale;
            vertices[i * 8 + 2] = mesh->vertices[i * 8 + 2] * scale;
            vertices[i * 8 + 3] = mesh->vertices[i * 8 + 3];
            vertices[i * 8 + 4] = 1.0f - mesh->vertices[i * 8 + 4];
            vertices[i * 8 + 5] = mesh->vertices[i * 8 + 5];
            vertices[i * 8 + 6] = mesh->vertices[i * 8 + 6];
            vertices[i * 8 + 7] = mesh->vertices[i * 8 + 7];
        }
        vertexBuffer->unlock();
        
        indexBuffer = new Kore::IndexBuffer(mesh->numFaces * 3);
        int* indices = indexBuffer->lock();
        for (int i = 0; i < mesh->numFaces * 3; ++i) {
            indices[i] = mesh->indices[i];
        }
        indexBuffer->unlock();
        
        // BB import testcode remove later
        {
            int index = 0;
            int count = 0;
            
            while (index >= 0 && strcmp(colliderFile, "") != 0) {
				Kore::vec3 min(std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity());
				Kore::vec3 max(-std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity());
				loadColl(colliderFile, min, max, index);
                
				if (min.x() != std::numeric_limits<double>::infinity()) {
					Kore::vec3 center = min + (max - min) / 2;
					Kore::vec3 extends = max - min;
					collider[count] = new BoxCollider(center, extends);

					assert(colliderCount > count);
					++count;
				}
            }
            Kore::log(Kore::Info, "Object has %i collider", count);
        }
        
        Kore::vec3 position = Kore::vec3(0,0,0); // TODO
		Collider.center = Kore::vec3(position.x(), position.y(), position.z());
		Collider.radius = 1;
    }

	void render(Kore::TextureUnit tex, int instances) {
		Kore::Graphics::setTexture(tex, image);
		Kore::Graphics::setVertexBuffers(vertexBuffers, 2);
		Kore::Graphics::setIndexBuffer(*indexBuffer);
		Kore::Graphics::drawIndexedVerticesInstanced(instances);
	}
    
    void render(Kore::TextureUnit tex, Kore::ConstantLocation mLocation) {
        Kore::Graphics::setTexture(tex, image);
        Kore::Graphics::setVertexBuffer(*vertexBuffer);
        Kore::Graphics::setIndexBuffer(*indexBuffer);
        Kore::Graphics::drawIndexedVertices();
    }

	Kore::VertexBuffer** vertexBuffers;
    Kore::VertexBuffer* vertexBuffer;
	Kore::IndexBuffer* indexBuffer;
    
    static const int colliderCount = 10;
    BoxCollider* collider[colliderCount];

	Mesh* mesh;
	Kore::Texture* image;

	SphereCollider Collider;
};
