#pragma once

#include <Kore/Graphics/Graphics.h>

extern Kore::VertexBuffer* landscapeVertices;
extern Kore::IndexBuffer* landscapeIndices;

void createLandscape();
void renderLandscape(Kore::ConstantLocation mLocation, Kore::ConstantLocation nLocation);