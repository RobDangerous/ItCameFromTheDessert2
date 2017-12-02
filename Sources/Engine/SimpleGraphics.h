#pragma once

void initGraphics();
void startFrame();
void endFrame();
void clear(float red, float green, float blue);
void setPixel(int x, int y, float red, float green, float blue);
void getPixel(Kore::Graphics1::Image* image, int x, int y, float& red, float& green, float& blue);
Kore::Graphics1::Image* loadImage(const char* filename);
void destroyImage(Kore::Graphics1::Image* image);
void drawImage(Kore::Graphics1::Image* image, int x, int y);
void drawTriangle(float x1, float y1, float z1, float u1, float v1, float x2, float y2, float z2, float u2, float v2, float x3, float y3, float z3, float u3, float v3);
const int width = 1024;
const int height = 768;
