#include "pch.h"

#include <vector>
#include <algorithm>
#include <iostream>

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

#include "Engine/Collision.h"
#include "Engine/InstancedMeshObject.h"
#include "Engine/ObjLoader.h"
#include "Engine/Particles.h"
#include "Engine/PhysicsObject.h"
#include "Engine/PhysicsWorld.h"
#include "Engine/Rendering.h"
#include "Engine/Explosion.h"
#include "Landscape.h"
#include "Text.h"

#include "Projectiles.h"
#include "TankSystem.h"
#include "Tank.h"
#include "KitchenObject.h"

#include "Ant.h"

#include "Engine/CollLoader.h"
#include <limits>

using namespace Kore;

KitchenObject* kitchenObjects[10];

namespace {
	const char* title = "It Came from the Dessert";
    const int width = 1024;
    const int height = 768;
    const float CAMERA_ROTATION_SPEED = 0.01f;
    const float CAMERA_ZOOM_SPEED = 1.f;
    
    double startTime;
    Shader* vertexShader;
    Shader* fragmentShader;
    Program* program;
    
    Shader* instancedVertexShader;
    Shader* instancedFragmentShader;
    Program* instancedProgram;
    
    bool left;
    bool right;
    bool up;
    bool down;
    
    Kravur* font14;
    Kravur* font24;
    Kravur* font34;
    Kravur* font44;
    Text* textRenderer;
    
    mat4 P;
    mat4 View;
    
    float horizontalAngle = 0.0;
    float verticalAngle = 0.0;
    vec3 cameraPos;
    vec3 cameraDir;
    vec3 cameraUp;
    
    float lightPosX;
    float lightPosY;
    float lightPosZ;
    
    InstancedMeshObject* stoneMesh;
    MeshObject* projectileMesh;
    ConstantLocation instancedPLocation;
    ConstantLocation instancedVLocation;
    TextureUnit instancedTex;
    
    Projectiles* projectiles;
    
    PhysicsWorld physics;
    
    TextureUnit tex;
    ConstantLocation pLocation;
    ConstantLocation vLocation;
    ConstantLocation mLocation;
    ConstantLocation lightPosLocation;
    
    BoxCollider boxCollider(vec3(-46.0f, -4.0f, 44.0f), vec3(10.6f, 4.4f, 4.0f));
    
    Texture* particleImage;
    Explosion* explosionSystem;
    
    double lastTime;
    double gameOverTime = 0;
    bool gameOver = false;
    int gameOverKills = 0;
    
    InstancedMeshObject* tankTop;
    InstancedMeshObject* tankBottom;
    InstancedMeshObject* tankFlag;
    TankSystem* tankTics;
    ParticleRenderer* particleRenderer;
    
    Ground* ground;
    
    MeshObject* fridgeObjects[2];
    MeshObject* cakeOnTheCupboard[2];
    MeshObject* cupboard;
    MeshObject* cake;
    MeshObject* chair;
    MeshObject* table;
    MeshObject* oven[2];
    MeshObject* microwave[2];
    
    vec3 screenToWorld(vec2 screenPos) {
        vec4 pos((2 * screenPos.x()) / width - 1.0f, -((2 * screenPos.y()) / height - 1.0f), 0.0f, 1.0f);
        
        mat4 projection = P;
        mat4 view = View;
        
        projection = projection.Invert();
        view = view.Invert();
        
        vec4 worldPos = view * projection * pos;
        return vec3(worldPos.x() / worldPos.w(), worldPos.y() / worldPos.w(), worldPos.z() / worldPos.w());
    }
    
    float inline clamp(float min, float max, float val) {
        return Kore::max(min, Kore::min(max, val));
    }
    
    float inline clamp01(float val) {
        return Kore::max(0.0f, Kore::min(1.0f, val));
    }
    
    void renderShadowText(char* s,  float w, float h) {
        int offset = textRenderer->font->size / 12;
        textRenderer->drawString(s, 0x000000aa, w + offset, h + offset, mat3::Identity());
        textRenderer->drawString(s, 0xffffffff, w, h, mat3::Identity());
    }
    
    void renderCentered(char* s, float h, float w = width / 2) {
        float l = textRenderer->font->stringWidth(s);
        renderShadowText(s, w - l / 2, h);
    }
    
    void update() {
        double t = System::time() - startTime;
        double deltaT = t - lastTime;
        
        lastTime = t;
        Kore::Audio::update();
        
        Graphics::begin();
        Graphics::clear(Graphics::ClearColorFlag | Graphics::ClearDepthFlag | Graphics::ClearStencilFlag, 0xFF000000, 1.0f, 0);
        
        // Important: We need to set the program before we set a uniform
        program->set();
        Graphics::setBlendingMode(SourceAlpha, Kore::BlendingOperation::InverseSourceAlpha);
        Graphics::setRenderState(BlendingState, true);
        Graphics::setRenderState(DepthTest, true);
        
        // Direction: Spherical coordinates to Cartesian coordinates conversion
        cameraDir = vec3(
                         Kore::cos(verticalAngle) * Kore::sin(horizontalAngle),
                         Kore::sin(verticalAngle),
                         Kore::cos(verticalAngle) * Kore::cos(horizontalAngle)
                         );
        vec3 right = vec3(
                          Kore::sin(horizontalAngle - pi / 2.0),
                          0,
                          Kore::cos(horizontalAngle - pi / 2.0)
                          );
        cameraUp = right.cross(cameraDir);
        
        View = mat4::lookAlong(cameraDir, cameraPos, cameraUp);
        
        Graphics::setMatrix(pLocation, P);
        Graphics::setMatrix(vLocation, View);
        
        // update light pos
        /*lightPosX = 100;
         lightPosY = 100;
         lightPosZ = 100;
         Graphics::setFloat3(lightPosLocation, lightPosX, lightPosY, lightPosZ);
         
         if (t >= START_DELAY) {
         projectiles->update(deltaT);
         physics.Update(deltaT);
         tankTics->update(deltaT);
         }
         
         renderLandscape(tex);
         tankTics->render(tex, View, vLocation);*/
        
        // render the kitchen
       /* MeshObject** current = &objects[0];
        while (*current != nullptr) {
            // set the model matrix
            Graphics::setMatrix(mLocation, (*current)->M);
            (*current)->render(tex);
            
            ++current;
        }*/
        int i = 0;
        while (kitchenObjects[i] != nullptr) {
            kitchenObjects[i]->render(tex, mLocation);
            ++i;
        }
        
        instancedProgram->set();
        
        Graphics::setMatrix(instancedPLocation, P);
        Graphics::setMatrix(instancedVLocation, View);
        
        Ant::move();
        Ant::render(instancedVLocation, instancedTex, View);
        
        /*projectiles->render(vLocation, tex, View);
         particleRenderer->render(tex, View, vLocation);
         
         textRenderer->start();
         gameOver = gameOver || tankTics->deserted >= MAX_DESERTED;
         if (t < START_DELAY) {
         textRenderer->setFont(font44);
         renderCentered("Tank You!", height / 2 - 100);
         textRenderer->setFont(font24);
         renderCentered("Make the war last forever. But be warned, experienced", height / 2 + 50);
         renderCentered("soldiers might realize that the bloodshed is pointless.", height / 2 + 100);
         }
         else if (!gameOver) {
         char c[42];
         char d[42];
         char k[42];
         sprintf(c, "Time: %i", (int)t - START_DELAY);
         sprintf(d, "Deserted: %i / %i", tankTics->deserted, MAX_DESERTED);
         sprintf(k, "Destroyed: %i", tankTics->destroyed);
         textRenderer->setFont(font24);
         renderShadowText(c, 15, 15);
         renderShadowText(k, 15, 45);
         renderShadowText(d, 15, 75);
         }
         else {
         textRenderer->setFont(font44);
         renderCentered("Game over!", height / 2 - 220);
         textRenderer->setFont(font34);
         if(gameOverTime == 0.0f)
         gameOverTime = t - START_DELAY;
         if (gameOverKills == 0) gameOverKills = tankTics->destroyed;
         char gameOverText[256];
         sprintf(gameOverText, "The war lasted %i seconds and killed %i...", (int)gameOverTime, gameOverKills);
         renderCentered(gameOverText, height / 2 - 70);
         textRenderer->setFont(font24);
         renderCentered("Tank you for playing our jam game:", height / 2 + 80);
         renderCentered("Polona Caserman", height / 2 + 140, width / 4.0f);
         renderCentered("Robert Konrad", height / 2 + 140);
         renderCentered("Lars Lotter", height / 2 + 140, width * 3.0f / 4);
         renderCentered("Max Maag", height / 2 + 200, width / 3.0f);
         renderCentered("Christian Reuter", height / 2 + 200, width * 2.0f / 3.0f);
         textRenderer->setFont(font14);
         renderCentered("Background music by Hong Linh Thai and Maria Rumjanzewa", height / 2 + 280);
         }
         textRenderer->end();*/
        
        Graphics::end();
        Graphics::swapBuffers();
    }
    
    void keyDown(KeyCode code, wchar_t character) {
        if (code == Key_Up) {
            up = true;
        } else if (code == Key_Down) {
            down = true;
        } else if (code == Key_Left) {
            right = true;
        } else if (code == Key_Right) {
            left = true;
        } else if (code == Key_Escape) {
            Kore::System::stop();
        }
    }
    
    void keyUp(KeyCode code, wchar_t character) {
        if (code == Key_Up) {
            up = false;
        } else if (code == Key_Down) {
            down = false;
        } else if (code == Key_Left) {
            right = false;
        } else if (code == Key_Right) {
            left = false;
        } else if (code == Key_A) {
            tankTics->setMultipleSelect(false);
        }
    }
    
    void mouseMove(int windowId, int x, int y, int movementX, int movementY) {
        horizontalAngle += CAMERA_ROTATION_SPEED * movementX;
        verticalAngle -= CAMERA_ROTATION_SPEED * movementY;
    }
    
    void mousePress(int windowId, int button, int x, int y) {
        
        vec3 position = screenToWorld(vec2(x, y));
        vec3 pickDir = vec3(position.x(), position.y(), position.z()) - cameraPos;
        pickDir.normalize();
        
        if (button == 0) {
            //tankTics->select(cameraPosition, pickDir);
        }
        else if (button == 1) {
            //tankTics->issueCommand(cameraPosition, pickDir);
        }
    }
    
    void mouseScroll(int windowId, int delta) {
        cameraPos -= cameraDir * (CAMERA_ZOOM_SPEED * delta);
    }
    
    void init() {
        FileReader vs("shader.vert");
        FileReader fs("shader.frag");
        instancedVertexShader = new Shader(vs.readAll(), vs.size(), VertexShader);
        instancedFragmentShader = new Shader(fs.readAll(), fs.size(), FragmentShader);
        
        // This defines the structure of your Vertex Buffer
        VertexStructure** structures = new VertexStructure*[2];
        structures[0] = new VertexStructure();
        structures[0]->add("pos", Float3VertexData);
        structures[0]->add("tex", Float2VertexData);
        structures[0]->add("nor", Float3VertexData);
        
        structures[1] = new VertexStructure();
        structures[1]->add("M", Float4x4VertexData);
        structures[1]->add("N", Float4x4VertexData);
        structures[1]->add("tint", Float4VertexData);
        
        instancedProgram = new Program;
        instancedProgram->setVertexShader(instancedVertexShader);
        instancedProgram->setFragmentShader(instancedFragmentShader);
        instancedProgram->link(structures, 2);
        
        instancedTex = instancedProgram->getTextureUnit("tex");
        instancedPLocation = instancedProgram->getConstantLocation("P");
        instancedVLocation = instancedProgram->getConstantLocation("V");
        lightPosLocation = instancedProgram->getConstantLocation("lightPos");
        
        /*stoneMesh = new InstancedMeshObject("Data/Meshes/stone.obj", "Data/Textures/stone.png", structures, STONE_COUNT);
         projectileMesh = new MeshObject("Data/Meshes/projectile.obj", "Data/Textures/projectile.png", structures, PROJECTILE_SIZE);
         
         particleImage = new Texture("Data/Textures/particle.png", true);
         particleRenderer = new ParticleRenderer(structures);
         projectiles = new Projectiles(1000, 20, particleImage, projectileMesh, structures, &physics);*/
        
        // New shaders
        FileReader vs2("shader2.vert");
        FileReader fs2("shader2.frag");
        vertexShader = new Shader(vs2.readAll(), vs2.size(), VertexShader);
        fragmentShader = new Shader(fs2.readAll(), fs2.size(), FragmentShader);
        
        // This defines the structure of your Vertex Buffer
        VertexStructure structure;
        structure.add("pos", Float3VertexData);
        structure.add("tex", Float2VertexData);
        structure.add("nor", Float3VertexData);
        
        program = new Program;
        program->setVertexShader(vertexShader);
        program->setFragmentShader(fragmentShader);
        program->link(structure);
        
        tex = program->getTextureUnit("tex");
        
        pLocation = program->getConstantLocation("P");
        vLocation = program->getConstantLocation("V");
        mLocation = program->getConstantLocation("M");
        
        log(Info, "Load fridge");
        MeshObject* fridgeBody = new MeshObject("Data/Meshes/fridge_body.obj", "", "Data/Textures/map.png", structure, 1.0f);
        MeshObject* fridgeDoor = new MeshObject("Data/Meshes/fridge_door.obj", "", "Data/Textures/white.png", structure, 1.0f);
        fridgeObjects[0] = fridgeBody;
        fridgeObjects[1] = fridgeDoor;
        kitchenObjects[0] = new KitchenObject(fridgeObjects, 2, vec3(-10.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f));
        
        log(Info, "Load cupboard");
        cupboard = new MeshObject("Data/Meshes/cupboard.obj", "Data/Meshes/cupboard_collider.obj", "Data/Textures/white.png", structure, 1.0f);
        cake = new MeshObject("Data/Meshes/cake.obj", "Data/Meshes/cake_collider.obj", "Data/Textures/white.png", structure, 1.0f);
        cakeOnTheCupboard[0] = cupboard;
        cakeOnTheCupboard[1] = cake;
        kitchenObjects[1] = new KitchenObject(cakeOnTheCupboard, 2, vec3(0.0f, 0.0f, 0.0f), vec3(pi, 0.0f, 0.0f));
        
        log(Info, "Load chair");
        chair = new MeshObject("Data/Meshes/chair.obj", "Data/Meshes/chair_collider.obj", "Data/Textures/map.png", structure, 1.0f);
        kitchenObjects[2] = new KitchenObject(&chair, 1, vec3(5.0f, 0.0f, 5.0f), vec3(0.0f, 0.0f, 0.0f));
        kitchenObjects[3] = new KitchenObject(&chair, 1, vec3(5.0f, 0.0f, 8.0f), vec3(pi, 0.0f, 0.0f));
        kitchenObjects[4] = new KitchenObject(&chair, 1, vec3(7.0f, 0.0f, 6.5f), vec3(-pi/2, 0.0f, 0.0f));
        kitchenObjects[5] = new KitchenObject(&chair, 1, vec3(3.5f, 0.0f, 6.5f), vec3(pi/2, 0.0f, 0.0f));
        
        log(Info, "Load table");
        table = new MeshObject("Data/Meshes/table.obj", "Data/Meshes/table_collider.obj", "Data/Textures/map.png", structure, 1.0f);
        kitchenObjects[6] = new KitchenObject(&table, 1, vec3(5.0f, 0.0f, 6.5f), vec3(0.0f, 0.0f, 0.0f));
        
        log(Info, "Load fridge");
		MeshObject* stove_body = new MeshObject("Data/Meshes/stove_body.obj", "", "Data/Textures/map.png", structure, 1.0f);
		MeshObject* stove_door = new MeshObject("Data/Meshes/stove_door.obj", "", "Data/Textures/map.png", structure, 1.0f);
        oven[0] = stove_body;
        oven[1] = stove_door;
        kitchenObjects[7] = new KitchenObject(oven, 2, vec3(2.0f, 0.0f, 0.0f), vec3(pi, 0.0f, 0.0f));
        
        log(Info, "Load microwave");
        MeshObject* micro_body = new MeshObject("Data/Meshes/microwave_body.obj", "", "Data/Textures/map.png", structure, 1.0f);
        MeshObject* micro_door = new MeshObject("Data/Meshes/microwave_door.obj", "", "Data/Textures/white.png", structure, 1.0f);
        microwave[0] = micro_body;
        microwave[1] = micro_door;
        kitchenObjects[8] = new KitchenObject(microwave, 2, vec3(4.0f, 0.0f, 0.0f), vec3(-pi/2, 0.0f, 0.0f));
        
        Random::init(System::time() * 100);
        
        Ant::init();
        
        Graphics::setRenderState(DepthTest, true);
        Graphics::setRenderState(DepthTestCompare, ZCompareLess);
        
        Graphics::setTextureAddressing(tex, U, Repeat);
        Graphics::setTextureAddressing(tex, V, Repeat);
        
        //explosionSystem = new Explosion(vec3(2,6,0), 2.f, 10.f, 300, structures, particleImage);
        
        cameraPos = vec3(0, 0, 20);
        cameraDir = vec3(0, 0, -20);
        cameraUp = vec3(0, 0, -1);
        P = mat4::Perspective(45, (float)width / (float)height, 0.1f, 1000);
        
        //createLandscape(structures, MAP_SIZE_OUTER, stoneMesh, STONE_COUNT, ground);
        
        font14 = Kravur::load("Data/Fonts/arial", FontStyle(), 14);
        font24 = Kravur::load("Data/Fonts/arial", FontStyle(), 24);
        font34 = Kravur::load("Data/Fonts/arial", FontStyle(), 34);
        font44 = Kravur::load("Data/Fonts/arial", FontStyle(), 44);
        textRenderer = new Text;
        textRenderer->setProjection(width, height);
        textRenderer->setFont(font44);
        
        /*tankTop = new InstancedMeshObject("Data/Meshes/tank_top.obj", "Data/Textures/tank_top.png", structures, MAX_TANKS, 8);
         tankBottom = new InstancedMeshObject("Data/Meshes/tank_bottom.obj", "Data/Textures/tank_bottom.png", structures, MAX_TANKS, 10);
         tankFlag = new InstancedMeshObject("Data/Meshes/flag.obj", "Data/Textures/flag.png", structures, MAX_TANKS, 2);
         
         tankTics = new TankSystem(&physics, particleRenderer, tankBottom, tankTop, tankFlag, vec3(-MAP_SIZE_INNER / 2, 6, -MAP_SIZE_INNER / 2), vec3(-MAP_SIZE_INNER / 2, 6, MAP_SIZE_INNER / 2), vec3(MAP_SIZE_INNER / 2, 6, -MAP_SIZE_INNER / 2), vec3(MAP_SIZE_INNER / 2, 6, MAP_SIZE_INNER / 2), 3, projectiles, structures, ground);
         
         Sound *bgSound = new Sound("Data/Sounds/WarTheme.wav");
         Mixer::play(bgSound);*/
    }
}

int kore(int argc, char** argv) {
    Kore::System::setName(title);
	Kore::System::setup();

	Kore::WindowOptions options;
	options.title = title;
	options.width = width;
	options.height = height;
	options.x = 100;
	options.y = 0;
	options.targetDisplay = -1;
	options.mode = WindowModeWindow;
	options.rendererOptions.depthBufferBits = 16;
	options.rendererOptions.stencilBufferBits = 8;
	options.rendererOptions.textureFormat = 0;
	options.rendererOptions.antialiasing = 0;
	Kore::System::initWindow(options);

	Kore::Mixer::init();
	Kore::Audio::init();

	init();

	Kore::System::setCallback(update);

	startTime = System::time();

	Keyboard::the()->KeyDown = keyDown;
	Keyboard::the()->KeyUp = keyUp;
	Mouse::the()->Move = mouseMove;
	Mouse::the()->Press = mousePress;
	Mouse::the()->Scroll = mouseScroll;
	Mouse::the()->lock(0);

	Kore::System::start();

	return 0;
}
