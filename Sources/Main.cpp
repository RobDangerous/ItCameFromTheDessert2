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
#include <Kore/Graphics/Graphics2.h>
#include <Kore/Graphics/Color.h>
#include <Kore/Log.h>

#include "Engine/Collision.h"
#include "Engine/InstancedMeshObject.h"
#include "Engine/TriggerCollider.h"
#include "Engine/ObjLoader.h"
#include "Engine/Particles.h"
#include "Engine/PhysicsObject.h"
#include "Engine/PhysicsWorld.h"
#include "Engine/Rendering.h"
#include "Engine/Explosion.h"
#include "Landscape.h"

#include "Projectiles.h"
#include "TankSystem.h"
#include "Tank.h"
#include "KitchenObject.h"

#include "Ant.h"

#include "Engine/CollLoader.h"
#include <limits>

using namespace Kore;

KitchenObject* kitchenObjects[30];
//TriggerCollider* triggerCollider[6] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};

namespace {
	const char* title = "It Came from the Dessert";
    const int width = 1024;
    const int height = 768;
    const float CAMERA_ROTATION_SPEED = 0.01f;
	const float CAMERA_ZOOM_SPEED = 1.f;
	const float CAMERA_MOVE_SPEED = 2.f;
    
    double startTime;
    Shader* vertexShader;
    Shader* fragmentShader;
    Program* program;

	Graphics2* g2;
    
    Shader* instancedVertexShader;
    Shader* instancedFragmentShader;
    Program* instancedProgram;

	bool left_A;
	bool left_C;
	bool right_A;
	bool right_C;
    bool up_A;
	bool up_C;
	bool down_A;
    bool down_C;
	bool jump;
	bool crouch;
    
    Kravur* font14;
    Kravur* font24;
    Kravur* font34;
    Kravur* font44;
    
    mat4 P;
    mat4 View;
    
    float horizontalAngle = -1.24f * pi;
    float verticalAngle = -0.5f;
    vec3 cameraPos = vec3(-5.5, 6, 10);
    vec3 cameraDir;
    vec3 cameraUp;
    
    ConstantLocation instancedPLocation;
    ConstantLocation instancedVLocation;
    TextureUnit instancedTex;
	
	PhysicsWorld physics;
    
    TextureUnit tex;
    ConstantLocation pLocation;
    ConstantLocation vLocation;
    ConstantLocation mLocation;
    ConstantLocation lightPosLocation;
	VertexStructure structure;
    
    //BoxCollider boxCollider(vec3(-46.0f, -4.0f, 44.0f), vec3(10.6f, 4.4f, 4.0f));
    
    Texture* particleImage;
    
    double lastTime;
    const int maxPizza = 6;
	int pizzaCount = 0;

    ParticleRenderer* particleRenderer;
    
    MeshObject* room_floor;
    MeshObject* room_wall1;
    MeshObject* room_wall2;
    MeshObject* room_wall3;
    MeshObject* room_wall4;
    
    MeshObject* fridgeBody;
    MeshObject* fridgeDoorOpen;
    MeshObject* fridgeDoorClosed;
    MeshObject* cake;
	MeshObject* cupboard1;
	MeshObject* cupboard2;
	MeshObject* cupboard3;
    MeshObject* cupboard4;
    MeshObject* cupboard5;
    MeshObject* cupboard6;
    MeshObject* cupboard7;
    MeshObject* cupboard8;
	MeshObject* chair1;
	MeshObject* chair2;
	MeshObject* chair3;
	MeshObject* chair4;
    MeshObject* table;
    MeshObject* microwaveBody;
    MeshObject* microwaveDoorOpen;
    MeshObject* microwaveDoorClosed;
    MeshObject* ovenBody;
    MeshObject* ovenDoorOpen;
    MeshObject* ovenDoorClosed;
    MeshObject* stove;
    MeshObject* wash;
	KitchenObject* hovered;
    
    // Death collider
    TriggerCollider* microwaveTrigger;
    TriggerCollider* fridgeTrigger;
    TriggerCollider* ovenTrigger;
    TriggerCollider* stoveTrigger;
    TriggerCollider* washTrigger;
    
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
        /*int offset = textRenderer->font->size / 12;
        textRenderer->drawString(s, 0x000000aa, w + offset, h + offset, mat3::Identity());
        textRenderer->drawString(s, 0xffffffff, w, h, mat3::Identity());*/
    }
    
    void renderCentered(char* s, float h, float w = width / 2) {
        /*float l = textRenderer->font->stringWidth(s);
        renderShadowText(s, w - l / 2, h);*/
    }

	float rayIntersectsWithMesh(vec3 pos, vec3 dir, MeshObject* obj) {
		float distance = std::numeric_limits<float>::infinity();

		if (obj == nullptr) return distance;

		for (int k = 0; k < obj->colliderCount; ++k) {
			if (obj->collider[k] != nullptr) {
				float dist;
				if (obj->collider[k]->IntersectsWith(pos, dir, dist)) {
					distance = Kore::min(dist, distance);
				}
			}
		}
		return distance;
	}

	KitchenObject* getIntersectingMesh(vec3 pos, vec3 dir, float &distMin) {
		distMin = std::numeric_limits<float>::infinity();
		KitchenObject* result = nullptr;
		for (unsigned oi = 0; kitchenObjects[oi] != nullptr; ++oi) {
			if (!kitchenObjects[oi]->visible) continue;

			float dist = rayIntersectsWithMesh(pos, dir, kitchenObjects[oi]->body);
			if (dist < distMin) {
				distMin = dist;
				result = kitchenObjects[oi];
			}
			if (kitchenObjects[oi]->door_closed != nullptr && kitchenObjects[oi]->closed) {
				dist = rayIntersectsWithMesh(pos, dir, kitchenObjects[oi]->door_closed);
				if (dist < distMin) {
					distMin = dist;
					result = kitchenObjects[oi];
				}
			}
		}
		return result;
	}
    
    void update() {
        double t = System::time() - startTime;
        double deltaT = t - lastTime;
        
        lastTime = t;
        Kore::Audio::update();
        
        Graphics::begin();
        Graphics::clear(Graphics::ClearColorFlag | Graphics::ClearDepthFlag | Graphics::ClearStencilFlag, 0xFF0000FF, 1.0f, 0);
        
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
		if (left_A || left_C) {
			cameraPos -= right * (float) deltaT * CAMERA_MOVE_SPEED;
		}
		if (right_A || right_C) {
			cameraPos += right * (float)deltaT * CAMERA_MOVE_SPEED;
		}
		if (down_A || down_C) {
			cameraPos -= cameraDir * (float)deltaT * CAMERA_MOVE_SPEED;
		}
		if (up_A || up_C) {
			cameraPos += cameraDir * (float)deltaT * CAMERA_MOVE_SPEED;
		}
		if (crouch) {
			cameraPos -= cameraUp * (float)deltaT * CAMERA_MOVE_SPEED;
		}
		if (jump) {
			cameraPos += cameraUp * (float)deltaT * CAMERA_MOVE_SPEED;
		}

		hovered = nullptr;
		float distMin = std::numeric_limits<float>::infinity();
		hovered = getIntersectingMesh(cameraPos, cameraDir, distMin);
        
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
        int i = 0;
        while (kitchenObjects[i] != nullptr) {
            kitchenObjects[i]->render(tex, mLocation);
            
            // test: render trigger collider
            /*if (kitchenObjects[i]->triggerCollider != nullptr) {
                kitchenObjects[i]->triggerCollider->renderTest(tex, mLocation);
            }*/
            ++i;
        }
        
        // render the room
        vec3 pos = vec3(0, -1.0f, 6.5f);
        mat4 M = mat4::Translation(pos.x(), pos.y(), pos.z());
        Kore::Graphics::setMatrix(mLocation, M);
        room_floor->render(tex, mLocation);
        room_wall1->render(tex, mLocation);
        room_wall2->render(tex, mLocation);
        room_wall3->render(tex, mLocation);
        room_wall4->render(tex, mLocation);
        
        instancedProgram->set();
        
        Graphics::setMatrix(instancedPLocation, P);
        Graphics::setMatrix(instancedVLocation, View);
        
        Ant::moveEverybody(deltaT);
        Ant::render(instancedVLocation, instancedTex, View);
        
        
        /*
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

        
		g2->begin(false);
        
		if (hovered == nullptr) {
            g2->setColor(Color::White);
        }
		else if (hovered->pizza) {
            g2->setColor(Color::Red);
		}
		else if (hovered->door_closed == nullptr) {
			g2->setColor(Color::Green);
		}
		else {
			g2->setColor(Color::Blue);
		}
		g2->drawRect(width / 2 -  1, height / 2 -  1, 2, 2, 1);
		g2->drawRect(width / 2 +  8, height / 2 -  1, 8, 2, 1);
		g2->drawRect(width / 2 - 16, height / 2 -  1, 8, 2, 1);
		g2->drawRect(width / 2 -  1, height / 2 +  8, 2, 8, 1);
		g2->drawRect(width / 2 -  1, height / 2 - 16, 2, 8, 1);
		g2->end();
        
        g2->setFont(font24);
        g2->setFontColor(Color::Black);
        g2->setFontSize(24);
        char pizza_text[42];
        sprintf(pizza_text, "You have %i pizza", maxPizza-pizzaCount);
        g2->drawString(pizza_text, 10, 10);
        
        Graphics::end();
		Graphics::swapBuffers();
    }
    
    void keyDown(KeyCode code, wchar_t character) {
        if (code == Key_Up) {
            up_A = true;
        } else if (code == Key_Down) {
            down_A = true;
        } else if (code == Key_Left) {
            right_A = true;
        } else if (code == Key_Right) {
            left_A = true;
        } else if (code == Key_W) {
			up_C = true;
		} else if (code == Key_S) {
			down_C = true;
		} else if (code == Key_A) {
			right_C = true;
		} else if (code == Key_D) {
			left_C = true;
		} else if (code == Key_Control) {
			crouch = true;
		} else if (code == Key_Space) {
			jump = true;
		} else if (code == Key_Escape) {
            Kore::System::stop();
        } else if (code == Key_L) {
            Kore::log(Kore::Info, "Camera pos %f %f %f", cameraPos.x(), cameraPos.y(), cameraPos.z());
            Kore::log(Kore::Info, "Camera angle horizontal %f", horizontalAngle);
            Kore::log(Kore::Info, "Camera angle vertical %f", verticalAngle);
        } else if (code == Key_T) {
            int i = 0;
            while (kitchenObjects[i] != nullptr) {
                kitchenObjects[i]->openOrClose(lastTime);
                ++i;
            }
        }
    }
    
    void keyUp(KeyCode code, wchar_t character) {
		if (code == Key_Up) {
			up_A = false;
		}
		else if (code == Key_Down) {
			down_A = false;
		}
		else if (code == Key_Left) {
			right_A = false;
		}
		else if (code == Key_Right) {
			left_A = false;
		}
		else if (code == Key_W) {
			up_C = false;
		}
		else if (code == Key_S) {
			down_C = false;
		}
		else if (code == Key_A) {
			right_C = false;
		}
		else if (code == Key_D) {
			left_C = false;
		}
		else if (code == Key_Control) {
			crouch = false;
		}
		else if (code == Key_Space) {
			jump = false;
		}
    }
    
    void mouseMove(int windowId, int x, int y, int movementX, int movementY) {
        horizontalAngle += CAMERA_ROTATION_SPEED * movementX;
        verticalAngle -= CAMERA_ROTATION_SPEED * movementY;
		verticalAngle = Kore::min(Kore::max(verticalAngle, -0.49f * pi), 0.49f * pi);
    }
    
    void mousePress(int windowId, int button, int x, int y) {
		if (button == 0) {
			float dist = std::numeric_limits<float>::infinity();
			hovered = getIntersectingMesh(cameraPos, cameraDir, dist);

			if (hovered != nullptr && hovered->pizza) {
				for (int i = 0; i < pizzaCount; ++i) {
					if (kitchenObjects[19 + i] == hovered) {
						kitchenObjects[19 + i] = kitchenObjects[19 + pizzaCount - 1];
						kitchenObjects[19 + pizzaCount - 1] = nullptr;

						delete hovered->body;
						delete hovered;
						hovered = nullptr;

						--pizzaCount;
					}
				}
			}
			else if (dist < std::numeric_limits<float>::infinity() && pizzaCount < maxPizza) {
				vec3 pos = cameraPos + cameraDir * dist;

				MeshObject* pizza = new MeshObject("Data/Meshes/pizza.obj", "Data/Meshes/pizza_collider.obj", "Data/Textures/pizza.png", structure, 1.0f);
				kitchenObjects[19 + pizzaCount] = new KitchenObject(pizza, nullptr, nullptr, pos, vec3(0.0f, 0.0f, 0.0f), true);

				++pizzaCount;
			}
		}
		else if (button == 1) {
			if (hovered != nullptr) {
				hovered->openOrClose(lastTime);
			}
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
		structure = VertexStructure();
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
        
        room_floor = new MeshObject("Data/Meshes/room_floor.obj", "Data/Meshes/room_floor_collider.obj", "Data/Textures/marble_tile.png", structure, 1.0f);
        room_wall1 = new MeshObject("Data/Meshes/room_wall1.obj", "Data/Meshes/room_wall1_collider.obj", "Data/Textures/marble_tile.png", structure, 1.0f);
        room_wall2 = new MeshObject("Data/Meshes/room_wall2.obj", "Data/Meshes/room_wall2_collider.obj", "Data/Textures/marble_tile.png", structure, 1.0f);
        room_wall3 = new MeshObject("Data/Meshes/room_wall3.obj", "Data/Meshes/room_wall3_collider.obj", "Data/Textures/marble_tile.png", structure, 1.0f);
        room_wall4 = new MeshObject("Data/Meshes/room_wall4.obj", "Data/Meshes/room_wall4_collider.obj", "Data/Textures/marble_tile.png", structure, 1.0f);
        
        log(Info, "Load fridge");
        fridgeBody = new MeshObject("Data/Meshes/fridge_body.obj", "Data/Meshes/fridge_body_collider.obj", "Data/Textures/fridgeAndCupboardTexture.png", structure, 1.0f);
        fridgeDoorClosed = new MeshObject("Data/Meshes/fridge_door.obj", "Data/Meshes/fridge_door_collider.obj", "Data/Textures/fridgeAndCupboardTexture.png", structure, 1.0f);
        fridgeDoorOpen = new MeshObject("Data/Meshes/fridge_door_open.obj", nullptr, "Data/Textures/fridgeAndCupboardTexture.png", structure, 1.0f);
        kitchenObjects[0] = new KitchenObject(fridgeBody, fridgeDoorClosed, fridgeDoorOpen, vec3(6.0f, 0.0f, 0.0f), vec3(-pi/2, 0.0f, 0.0f));
        
        fridgeTrigger = new TriggerCollider("Data/Meshes/fridge_trigger.obj", "Data/Textures/black.png", structure, kitchenObjects[0]->M);
        kitchenObjects[0]->setTriggerCollider(fridgeTrigger);
        //triggerCollider[0] = fridgeTrigger;
        
        log(Info, "Load cupboard and cake");
        cupboard1 = new MeshObject("Data/Meshes/cupboard.obj", "Data/Meshes/cupboard_collider.obj", "Data/Textures/fridgeAndCupboardTexture.png", structure, 1.0f);
        cake = new MeshObject("Data/Meshes/cake.obj", "Data/Meshes/cake_collider.obj", "Data/Textures/CakeTexture.png", structure, 1.0f);
        kitchenObjects[1] = new KitchenObject(cupboard1, nullptr, nullptr, vec3(0.0f, 0.0f, 0.0f), vec3(pi, 0.0f, 0.0f));
        kitchenObjects[2] = new KitchenObject(cake, nullptr, nullptr, vec3(0.0f, 0.0f, 0.0f), vec3(pi, 0.0f, 0.0f));
        
        log(Info, "Load chair");
		chair1 = new MeshObject("Data/Meshes/chair.obj", "Data/Meshes/chair_collider.obj", "Data/Textures/LightFurnitureTexture.png", structure, 1.0f);
		chair2 = new MeshObject("Data/Meshes/chair.obj", "Data/Meshes/chair_collider.obj", "Data/Textures/LightFurnitureTexture.png", structure, 1.0f);
		chair3 = new MeshObject("Data/Meshes/chair.obj", "Data/Meshes/chair_collider.obj", "Data/Textures/LightFurnitureTexture.png", structure, 1.0f);
		chair4 = new MeshObject("Data/Meshes/chair.obj", "Data/Meshes/chair_collider.obj", "Data/Textures/LightFurnitureTexture.png", structure, 1.0f);
		kitchenObjects[3] = new KitchenObject(chair1, nullptr, nullptr, vec3(5.0f, 0.0f, 5.0f), vec3(0.0f, 0.0f, 0.0f));
        kitchenObjects[4] = new KitchenObject(chair2, nullptr, nullptr, vec3(5.0f, 0.0f, 8.0f), vec3(pi, 0.0f, 0.0f));
        kitchenObjects[5] = new KitchenObject(chair3, nullptr, nullptr, vec3(6.5f, 0.0f, 6.5f), vec3(-pi/2, 0.0f, 0.0f));
        kitchenObjects[6] = new KitchenObject(chair4, nullptr, nullptr, vec3(3.5f, 0.0f, 6.5f), vec3(pi/2, 0.0f, 0.0f));
        
        log(Info, "Load table");
        table = new MeshObject("Data/Meshes/table.obj", "Data/Meshes/table_collider.obj", "Data/Textures/LightFurnitureTexture.png", structure, 1.0f);
        kitchenObjects[7] = new KitchenObject(table, nullptr, nullptr, vec3(5.0f, 0.0f, 6.5f), vec3(0.0f, 0.0f, 0.0f));
        
        log(Info, "Load oven");
		ovenBody = new MeshObject("Data/Meshes/oven_body.obj", "Data/Meshes/oven_body_collider.obj", "Data/Textures/ovenTexture.png", structure, 1.0f);
		ovenDoorClosed = new MeshObject("Data/Meshes/oven_door.obj", "Data/Meshes/oven_door_collider.obj", "Data/Textures/ovenTexture.png", structure, 1.0f);
        ovenDoorOpen = new MeshObject("Data/Meshes/oven_door_open.obj", nullptr, "Data/Textures/ovenTexture.png", structure, 1.0f);
        stove = new MeshObject("Data/Meshes/stove.obj", "Data/Meshes/stove_collider.obj", "Data/Textures/stoveTexture_off.png", structure, 1.0f);
        kitchenObjects[8] = new KitchenObject(ovenBody, ovenDoorClosed, ovenDoorOpen, vec3(2.0f, 0.0f, 0.0f), vec3(pi, 0.0f, 0.0f));
		kitchenObjects[9] = new KitchenObject(stove, nullptr, nullptr, vec3(2.0f, 0.0f, 0.0f), vec3(pi, 0.0f, 0.0f));
        
        ovenTrigger = new TriggerCollider("Data/Meshes/oven_trigger.obj", "Data/Textures/black.png", structure, kitchenObjects[8]->M);
        kitchenObjects[8]->setTriggerCollider(ovenTrigger);
        //triggerCollider[1] = ovenTrigger;
        
        stoveTrigger = new TriggerCollider("Data/Meshes/stove_trigger.obj", "Data/Textures/black.png", structure, kitchenObjects[9]->M);
        kitchenObjects[9]->setTriggerCollider(stoveTrigger);
        //triggerCollider[2] = stoveTrigger;

        log(Info, "Load microwave");
        microwaveBody = new MeshObject("Data/Meshes/microwave_body.obj", "Data/Meshes/microwave_body_collider.obj", "Data/Textures/microwaveTexture.png", structure, 1.0f);
        microwaveDoorClosed = new MeshObject("Data/Meshes/microwave_door.obj", "Data/Meshes/microwave_door_collider.obj", "Data/Textures/microwaveTexture.png", structure, 1.0f);
        microwaveDoorOpen = new MeshObject("Data/Meshes/microwave_door_open.obj", nullptr, "Data/Textures/microwaveTexture.png", structure, 1.0f);
		cupboard2 = new MeshObject("Data/Meshes/cupboard.obj", "Data/Meshes/cupboard_collider.obj", "Data/Textures/fridgeAndCupboardTexture.png", structure, 1.0f);
		kitchenObjects[10] = new KitchenObject(microwaveBody, microwaveDoorClosed, microwaveDoorOpen, vec3(4.0f, 1.4f, 0.0f), vec3(-pi/2, 0.0f, 0.0f));
        kitchenObjects[11] = new KitchenObject(cupboard2, nullptr, nullptr, vec3(4.0f, 0.0f, 0.0f), vec3(pi, 0.0f, 0.0f));
        
        microwaveTrigger = new TriggerCollider("Data/Meshes/microwave_trigger.obj", "Data/Textures/black.png", structure, kitchenObjects[10]->M);
        kitchenObjects[10]->setTriggerCollider(microwaveTrigger);
        //triggerCollider[3] = microwaveTrigger;
        
        log(Info, "Load wash");
        wash = new MeshObject("Data/Meshes/wash.obj", "Data/Meshes/wash_collider.obj", "Data/Textures/white.png", structure, 1.0f);
        kitchenObjects[12] = new KitchenObject(wash, nullptr, nullptr, vec3(-2.0f, 0.0f, 0.0f), vec3(pi, 0.0f, 0.0f));
        
        washTrigger = new TriggerCollider("Data/Meshes/wash_trigger.obj", "Data/Textures/black.png", structure, kitchenObjects[12]->M);
        kitchenObjects[12]->setTriggerCollider(washTrigger);
        //triggerCollider[4] = washTrigger;
        
        log(Info, "Load cupboard");
		cupboard3 = new MeshObject("Data/Meshes/cupboard.obj", "Data/Meshes/cupboard_collider.obj", "Data/Textures/fridgeAndCupboardTexture.png", structure, 1.0f);
		kitchenObjects[13] = new KitchenObject(cupboard3, nullptr, nullptr, vec3(-4.0f, 0.0f, 0.0f), vec3(pi, 0.0f, 0.0f));
        
        
        cupboard4 = new MeshObject("Data/Meshes/cupboard.obj", "Data/Meshes/cupboard_collider.obj", "Data/Textures/fridgeAndCupboardTexture.png", structure, 1.0f);
        cupboard5 = new MeshObject("Data/Meshes/cupboard.obj", "Data/Meshes/cupboard_collider.obj", "Data/Textures/fridgeAndCupboardTexture.png", structure, 1.0f);
        cupboard6 = new MeshObject("Data/Meshes/cupboard.obj", "Data/Meshes/cupboard_collider.obj", "Data/Textures/fridgeAndCupboardTexture.png", structure, 1.0f);
        cupboard7 = new MeshObject("Data/Meshes/cupboard.obj", "Data/Meshes/cupboard_collider.obj", "Data/Textures/fridgeAndCupboardTexture.png", structure, 1.0f);
        cupboard8 = new MeshObject("Data/Meshes/cupboard.obj", "Data/Meshes/cupboard_collider.obj", "Data/Textures/fridgeAndCupboardTexture.png", structure, 1.0f);
        kitchenObjects[14] = new KitchenObject(cupboard4, nullptr, nullptr, vec3(4.0f, 0.0f, 13.5f), vec3(0.0f, 0.0f, 0.0f));
        kitchenObjects[15] = new KitchenObject(cupboard5, nullptr, nullptr, vec3(2.0f, 0.0f, 13.5f), vec3(0.0f, 0.0f, 0.0f));
        kitchenObjects[16] = new KitchenObject(cupboard6, nullptr, nullptr, vec3(0.0f, 0.0f, 13.5f), vec3(0.0f, 0.0f, 0.0f));
        kitchenObjects[17] = new KitchenObject(cupboard7, nullptr, nullptr, vec3(-2.0f, 0.0f, 13.5f), vec3(0.0f, 0.0f, 0.0f));
        kitchenObjects[18] = new KitchenObject(cupboard8, nullptr, nullptr, vec3(-4.0f, 0.0f, 13.5f), vec3(0.0f, 0.0f, 0.0f));
        
		hovered = nullptr;

        Random::init(System::time() * 100);
        
        Ant::init();
        
        Graphics::setRenderState(DepthTest, true);
        Graphics::setRenderState(DepthTestCompare, ZCompareLess);
        
        Graphics::setTextureAddressing(tex, U, Repeat);
        Graphics::setTextureAddressing(tex, V, Repeat);
        
        P = mat4::Perspective(45, (float)width / (float)height, 0.1f, 1000);
        
		g2 = new Graphics2(width, height);

        font14 = Kravur::load("Data/Fonts/arial", FontStyle(), 14);
        font24 = Kravur::load("Data/Fonts/arial", FontStyle(), 24);
        font34 = Kravur::load("Data/Fonts/arial", FontStyle(), 34);
        font44 = Kravur::load("Data/Fonts/arial", FontStyle(), 44);
    }
}

#ifdef SYS_WINDOWS
#include <Windows.h>
#endif

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
#ifdef SYS_WINDOWS
	char name[256];
	DWORD size = 255;
	GetUserNameA(name, &size);
	if (strcmp(name, "Robert") != 0) {
		Mouse::the()->lock(0);
	}
#else
	Mouse::the()->lock(0);
#endif

	Kore::System::start();

	return 0;
}
