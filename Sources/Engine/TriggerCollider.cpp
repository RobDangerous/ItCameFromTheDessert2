#include "pch.h"

#include "TriggerCollider.h"
#include "Collision.h"

TriggerCollider::TriggerCollider(const char* meshFile, const char* textureFile, const Kore::Graphics4::VertexStructure& structure, mat4 M, float scale) {
    mesh = loadObj(meshFile);
    image = new Kore::Graphics4::Texture(textureFile, true);
    
    // Mesh Vertex Buffer
    vertexBuffer = new Kore::Graphics4::VertexBuffer(mesh->numVertices, structure, 0);
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
    
    indexBuffer = new Kore::Graphics4::IndexBuffer(mesh->numFaces * 3);
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

void TriggerCollider::renderTest(Kore::Graphics4::TextureUnit tex, Kore::Graphics4::ConstantLocation mLocation) {
    Kore::Graphics4::setMatrix(mLocation, collider->M);
    Kore::Graphics4::setTexture(tex, image);
    Kore::Graphics4::setVertexBuffer(*vertexBuffer);
    Kore::Graphics4::setIndexBuffer(*indexBuffer);
    Kore::Graphics4::drawIndexedVertices();
}
