#pragma once

#include <Kore/IO/FileReader.h>
#include <Kore/Math/Core.h>
#include <Kore/System.h>
#include <Kore/Graphics/Image.h>
#include <Kore/Graphics/Graphics.h>

#include "PhysicsObject.h"

class TriggerCollider {
    public:
    TriggerCollider(const char* meshFile, const char* textureFile, const Kore::VertexStructure& structure, mat4 M, float scale = 1.0f);
    
    void renderTest(Kore::TextureUnit tex, Kore::ConstantLocation mLocation);
    
    Kore::VertexBuffer* vertexBuffer;
    Kore::IndexBuffer* indexBuffer;
    
    Mesh* mesh;
    Kore::Texture* image;

    mat4 M;
    
    BoxCollider* collider;
    
};
