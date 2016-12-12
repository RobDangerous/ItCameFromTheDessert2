#include "pch.h"

#include "TriggerCollider.h"
#include "Collision.h"

TriggerCollider::TriggerCollider(const char* meshFile, const char* textureFile, const Kore::VertexStructure& structure, mat4 M, float scale) {
    mesh = loadObj(meshFile);
    image = new Kore::Texture(textureFile, true);
    
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
    
    Kore::vec4 min(std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity(), 1);
    Kore::vec4 max(-std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity(), 1);
    int index = 0;
    loadColl(meshFile, min, max, index);
    collider = new BoxCollider(min, max);
    collider->trans(M);
}

void TriggerCollider::renderTest(Kore::TextureUnit tex, Kore::ConstantLocation mLocation) {
    Kore::Graphics::setMatrix(mLocation, collider->M);
    Kore::Graphics::setTexture(tex, image);
    Kore::Graphics::setVertexBuffer(*vertexBuffer);
    Kore::Graphics::setIndexBuffer(*indexBuffer);
    Kore::Graphics::drawIndexedVertices();
}
