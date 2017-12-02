#pragma once

#include <Kore/IO/FileReader.h>
#include <Kore/Math/Core.h>
#include <Kore/System.h>
#include <Kore/Graphics1/Image.h>
#include <Kore/Graphics4/Graphics.h>

#include "PhysicsObject.h"

class TriggerCollider {
    public:
    TriggerCollider(const char* meshFile, const char* textureFile, const Kore::Graphics4::VertexStructure& structure, mat4 M, float scale = 1.0f);
    
    void renderTest(Kore::Graphics4::TextureUnit tex, Kore::Graphics4::ConstantLocation mLocation);
    
    Kore::Graphics4::VertexBuffer* vertexBuffer;
    Kore::Graphics4::IndexBuffer* indexBuffer;
    
    Mesh* mesh;
    Kore::Graphics4::Texture* image;
    
    BoxCollider* collider;
    
};
