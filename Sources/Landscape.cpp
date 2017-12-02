#include "pch.h"

#include <vector>

#include "Landscape.h"
#include <Kore/Math/Vector.h>
#include <Kore/Math/Random.h>

#include "Engine/Rendering.h"
#include "Engine/InstancedMeshObject.h"

using namespace Kore;

Kore::Graphics4::VertexBuffer** landscapeVertices;
Kore::Graphics4::IndexBuffer* landscapeIndices;
Kore::Graphics4::Texture* landscapeTexture;

namespace {
	int stoneCount;
	InstancedMeshObject* stoneMesh;
	Kore::Graphics1::Image* normalmap;
}

vec3 getLandscapeNormal(float x, float y) {
	int normal = normalmap->at(static_cast<int>((x / MAP_SIZE_OUTER + 1.0f) / 2.0f * normalmap->width), static_cast<int>((y / MAP_SIZE_OUTER + 1.0f) / 2.0f * normalmap->height));
	int nxi = (normal & 0xff000000) >> 24;
	int nyi = (normal & 0xff0000) >> 16;
	int nzi = (normal & 0xff00) >> 8;
	float nx = (nxi / 255.0f - 0.5f) * 2.0f;
	float ny = (nyi / 255.0f - 0.5f) * 2.0f;
	float nz = (nzi / 255.0f - 0.5f) * 2.0f;
	return vec3(nx, ny, nz);
}


void createLandscape(Graphics4::VertexStructure** structures, float size, InstancedMeshObject* sMesh, int sCount, Ground*& ground) {
	Kore::Graphics1::Image* map = new Kore::Graphics1::Image("Data/Textures/map.png", true);
	normalmap = new Kore::Graphics1::Image("Data/Textures/mapnormals.png", true);
	landscapeTexture = new Graphics4::Texture("Data/Textures/sand.png", true);

	const int w = 250;
	const int h = 250;
	
	landscapeVertices = new Graphics4::VertexBuffer*[2];
	landscapeVertices[0] = new Graphics4::VertexBuffer((w + 1) * (h + 1), *structures[0], 0);
	float* vertices = landscapeVertices[0]->lock();
	int i = 0;
	
	float* height = new float[(w+1)*(h+1)];
	vec3* normals = new vec3[(w+1)*(h+1)];

	for (int y = 0; y <= h; ++y) {
		for (int x = 0; x <= w; ++x) {
			int color = 0xff00 & map->at(static_cast<int>(x / (float)(w + 1) * map->width), static_cast<int>(y / (float)(h + 1) * map->height));
			color >>= 8;
			int normal = normalmap->at(static_cast<int>(x / (float)(w + 1) * map->width), static_cast<int>(y / (float)(h + 1) * map->height));
			int nxi = (normal & 0xff000000) >> 24;
			int nyi = (normal & 0xff0000) >> 16;
			int nzi = (normal & 0xff00) >> 8;
			float nx = (nxi / 255.0f - 0.5f) * 2.0f;
			float ny = (nyi / 255.0f - 0.5f) * 2.0f;
			float nz = (nzi / 255.0f - 0.5f) * 2.0f;
			float hght = color / 255.0f * 10.0f;
			vertices[i++] = -size / 2 + size / w * x;
			vertices[i++] = hght;
			vertices[i++] = -size / 2 + size / h * y;
			vertices[i++] = x % 2;
			vertices[i++] = y % 2;
			vertices[i++] = nx;
			vertices[i++] = ny;
			vertices[i++] = nz;
			height[y * w + x] = hght;
			normals[y * w + x] = vec3(nx, ny, nz);
		}
	}

	ground = new Ground(height, normals, w, h, size, size);

	stoneCount = sCount;
	stoneMesh = sMesh;
	float* data = stoneMesh->vertexBuffers[1]->lock();
    for (int i = 0; i < stoneCount; i++) {
		int xr = Random::get(0, w);
		int yr = Random::get(0, h);
		mat4 M = mat4::Translation(vertices[(xr * (w + 1) + yr) * 8 + 0], vertices[(xr * (w + 1) + yr) * 8 + 1], vertices[(xr * (w + 1) + yr) * 8 + 2]) * mat4::Rotation(2 * pi * (Random::get(0, 1000) * 1.0f) / 1000, 2 * pi * (Random::get(0, 1000) * 1.0f) / 1000, 2 * pi * (Random::get(0, 1000) * 1.0f) / 1000);
		setMatrix(data, i, 0, 36, M);
		setMatrix(data, i, 16, 36, calculateN(M));
		setVec4(data, i, 32, 36, vec4(1, 1, 1, 1));
	}
	stoneMesh->vertexBuffers[1]->unlock();

	landscapeVertices[0]->unlock();
	
	landscapeVertices[1] = new Graphics4::VertexBuffer(1, *structures[1], 1);
	landscapeIndices = new Graphics4::IndexBuffer(w * h * 6);
	int* indices = landscapeIndices->lock();
	i = 0;
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			int baseindex = y * (w + 1) + x;
			indices[i++] = baseindex;
			indices[i++] = baseindex + 1;
			indices[i++] = baseindex + (w + 1);

			indices[i++] = baseindex + 1;
			indices[i++] = baseindex + (w + 1);
			indices[i++] = baseindex + (w + 1) + 1;
		}
	}
	landscapeIndices->unlock();
}

void renderLandscape(Kore::Graphics4::TextureUnit tex) {
	Graphics4::setTexture(tex, landscapeTexture);

	float* data = landscapeVertices[1]->lock();
	setMatrix(data, 0, 0, 36, mat4::Identity());
	setMatrix(data, 0, 16, 36, mat4::Identity());
	setVec4(data, 0, 32, 36, vec4(1, 1, 1, 1));
	landscapeVertices[1]->unlock();
	
	Graphics4::setVertexBuffers(landscapeVertices, 2);
	Graphics4::setIndexBuffer(*landscapeIndices);
	Graphics4::drawIndexedVerticesInstanced(1);
	
	stoneMesh->render(tex, stoneCount);
}
