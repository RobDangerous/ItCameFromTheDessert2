#pragma once

#include <Kore/Graphics4/Graphics.h>
#include "Ground.h"

#include "Engine/InstancedMeshObject.h"

const int MAP_SIZE_INNER = 200;
const int MAP_SIZE_OUTER = 300;
const int STONE_COUNT = 64;

extern Kore::Graphics4::VertexBuffer** landscapeVertices;
extern Kore::Graphics4::IndexBuffer* landscapeIndices;
extern Kore::Graphics4::Texture* landscapeTexture;

void createLandscape(Kore::Graphics4::VertexStructure** structures, float size, InstancedMeshObject* sMesh, int sCount, Ground*&);
void renderLandscape(Kore::Graphics4::TextureUnit tex);
vec3 getLandscapeNormal(float x, float y);
