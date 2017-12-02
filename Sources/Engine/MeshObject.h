#pragma once

#include <Kore/IO/FileReader.h>
#include <Kore/Math/Core.h>
#include <Kore/Math/Random.h>
#include <Kore/System.h>
#include <Kore/Input/Keyboard.h>
#include <Kore/Input/Mouse.h>
#include <Kore/Audio1/Audio.h>
#include <Kore/Graphics1/Image.h>
#include <Kore/Graphics4/Graphics.h>
#include <Kore/Log.h>

#include "Collision.h"
#include "ObjLoader.h"
#include "Rendering.h"
#include "CollLoader.h"
#include "assert.h"
#include "string.h"

class MeshObject {
public:
	MeshObject(const char* meshFile, const char* textureFile, Kore::Graphics4::VertexStructure** structures, float scale = 1.0f) {
		for (int i = 0; i < colliderCount; ++i) collider[i] = nullptr;
		mesh = loadObj(meshFile);
		image = new Kore::Graphics4::Texture(textureFile, true);
		
		vertexBuffers = new Kore::Graphics4::VertexBuffer*[2];
		vertexBuffers[0] = new Kore::Graphics4::VertexBuffer(mesh->numVertices, *structures[0], 0);
		float* vertices = vertexBuffers[0]->lock();
		for (int i = 0; i < mesh->numVertices; ++i) {
			vertices[i * 8 + 0] = mesh->vertices[i * 8 + 0];
			vertices[i * 8 + 1] = mesh->vertices[i * 8 + 1];
			vertices[i * 8 + 2] = mesh->vertices[i * 8 + 2];
			vertices[i * 8 + 3] = mesh->vertices[i * 8 + 3];
			vertices[i * 8 + 4] = 1.0f - mesh->vertices[i * 8 + 4];
			vertices[i * 8 + 5] = mesh->vertices[i * 8 + 5];
			vertices[i * 8 + 6] = mesh->vertices[i * 8 + 6];
			vertices[i * 8 + 7] = mesh->vertices[i * 8 + 7];
		}
		vertexBuffers[0]->unlock();
		
		vertexBuffers[1] = new Kore::Graphics4::VertexBuffer(1, *structures[1], 1);

		indexBuffer = new Kore::Graphics4::IndexBuffer(mesh->numFaces * 3);
		int* indices = indexBuffer->lock();
		for (int i = 0; i < mesh->numFaces * 3; i++) {
			indices[i] = mesh->indices[i];
		}
		indexBuffer->unlock();
	}
    
    MeshObject(const char* meshFile, const char* colliderFile, const char* textureFile, const Kore::Graphics4::VertexStructure& structure, float scale) {
		for (int i = 0; i < colliderCount; ++i) collider[i] = nullptr;
        mesh = loadObj(meshFile);
        image = new Kore::Graphics4::Texture(textureFile, true);
		vertexBuffers = nullptr;
        
        // Mesh Vertex Buffer
        vertexBuffer = new Kore::Graphics4::VertexBuffer(mesh->numVertices, structure, 0);
        float* vertices = vertexBuffer->lock();

		Kore::vec4 min1(10000, 100000, 1999909, 1);
		Kore::vec4 max1(-10000, -100000, -1999909, 1);
        for (int i = 0; i < mesh->numVertices; ++i) {
			min1.x() = Kore::min(min1.x(), mesh->vertices[i * 8 + 0]);
			max1.x() = Kore::max(max1.x(), mesh->vertices[i * 8 + 0]);
			min1.y() = Kore::min(min1.y(), mesh->vertices[i * 8 + 1]);
			max1.y() = Kore::max(max1.y(), mesh->vertices[i * 8 + 1]);
			min1.z() = Kore::min(min1.z(), mesh->vertices[i * 8 + 2]);
			max1.z() = Kore::max(max1.z(), mesh->vertices[i * 8 + 2]);

            vertices[i * 8 + 0] = mesh->vertices[i * 8 + 0];
            vertices[i * 8 + 1] = mesh->vertices[i * 8 + 1];
            vertices[i * 8 + 2] = mesh->vertices[i * 8 + 2];
            vertices[i * 8 + 3] = mesh->vertices[i * 8 + 3];
            vertices[i * 8 + 4] = 1.0f - mesh->vertices[i * 8 + 4];
            vertices[i * 8 + 5] = mesh->vertices[i * 8 + 5];
            vertices[i * 8 + 6] = mesh->vertices[i * 8 + 6];
            vertices[i * 8 + 7] = mesh->vertices[i * 8 + 7];
        }
        vertexBuffer->unlock();
        
        indexBuffer = new Kore::Graphics4::IndexBuffer(mesh->numFaces * 3);
        int* indices = indexBuffer->lock();
        for (int i = 0; i < mesh->numFaces * 3; ++i) {
            indices[i] = mesh->indices[i];
        }
        indexBuffer->unlock();
        
        // BB import testcode remove later
        if (colliderFile != nullptr) {
            int index = 0;
            int count = 0;

			//collider[count] = new BoxCollider(min, max);

			Kore::vec4 min2(10000, 100000, 1999909, 1);
			Kore::vec4 max2(-10000, -100000, -1999909, 1);
            while (index >= 0 && strcmp(colliderFile, "") != 0) {
				Kore::vec4 min(std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity(), 1);
				Kore::vec4 max(-std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity(), 1);
				loadColl(colliderFile, min, max, index);
                
				if (min.x() != std::numeric_limits<double>::infinity()) {
					collider[count] = new BoxCollider(min, max);


					min2.x() = Kore::min(min2.x(), min.x());
					max2.x() = Kore::max(max2.x(), max.x());
					min2.y() = Kore::min(min2.y(), min.y());
					max2.y() = Kore::max(max2.y(), max.y());
					min2.z() = Kore::min(min2.z(), min.z());
					max2.z() = Kore::max(max2.z(), max.z());


					assert(colliderCount > count);
					++count;
				}
            }
            Kore::log(Kore::Info, "Object has %i collider, diff %f / %f", count, (max1 - max2).getLength(), (min1 - min2).getLength());
        }
    }

	void render(Kore::Graphics4::TextureUnit tex, int instances) {
		Kore::Graphics4::setTexture(tex, image);
		Kore::Graphics4::setVertexBuffers(vertexBuffers, 2);
		Kore::Graphics4::setIndexBuffer(*indexBuffer);
		Kore::Graphics4::drawIndexedVerticesInstanced(instances);
	}
    
    void render(Kore::Graphics4::TextureUnit tex, Kore::Graphics4::ConstantLocation mLocation) {
        Kore::Graphics4::setTexture(tex, image);
        Kore::Graphics4::setVertexBuffer(*vertexBuffer);
        Kore::Graphics4::setIndexBuffer(*indexBuffer);
        Kore::Graphics4::drawIndexedVertices();
    }

	Kore::Graphics4::VertexBuffer** vertexBuffers;
    Kore::Graphics4::VertexBuffer* vertexBuffer;
	Kore::Graphics4::IndexBuffer* indexBuffer;
    
    static const int colliderCount = 15;
    BoxCollider* collider[colliderCount];

	Mesh* mesh;
	Kore::Graphics4::Texture* image;
};
