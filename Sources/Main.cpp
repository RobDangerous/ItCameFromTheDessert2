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
#include <Kore/Audio1/Audio.h>
#include <Kore/Graphics1/Image.h>
#include <Kore/Graphics4/Graphics.h>
#include <Kore/Graphics2/Graphics.h>
#include <Kore/Graphics1/Color.h>
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
MeshObject* roomObjects[8];
//TriggerCollider* triggerCollider[6] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};

namespace {
	const char* title = "It Came from the Dessert";
    const int width = 1024;
    const int height = 768;
    const float CAMERA_ROTATION_SPEED = 0.01f;
	const float CAMERA_ZOOM_SPEED = 1.f;
	const float CAMERA_MOVE_SPEED = 2.f;

	const int PIZZA_OFFSET = 21;
    
    double startTime;
	Graphics4::Shader* vertexShader;
	Graphics4::Shader* fragmentShader;
	Graphics4::PipelineState* program;

	Graphics2::Graphics2* g2;
    
	Graphics4::Shader* instancedVertexShader;
	Graphics4::Shader* instancedFragmentShader;
	Graphics4::PipelineState* instancedProgram;

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
	mat4 rooM;
    
    float horizontalAngle = -1.24f * pi;
    float verticalAngle = -0.5f;
    vec3 cameraPos = vec3(-5.5, 6, 10);
    vec3 cameraDir;
    vec3 cameraUp;
    
	Graphics4::ConstantLocation instancedPLocation;
	Graphics4::ConstantLocation instancedVLocation;
	Graphics4::TextureUnit instancedTex;
	
	PhysicsWorld physics;
    
	Graphics4::TextureUnit tex;
	Graphics4::ConstantLocation pLocation;
	Graphics4::ConstantLocation vLocation;
	Graphics4::ConstantLocation mLocation;
	Graphics4::ConstantLocation lightPosLocation;
	Graphics4::VertexStructure structure;
    
    //BoxCollider boxCollider(vec3(-46.0f, -4.0f, 44.0f), vec3(10.6f, 4.4f, 4.0f));
    
	Graphics4::Texture* particleImage;
    
    double lastTime;
    const int maxPizza = 6;
	int pizzaCount = 0;

    ParticleRenderer* particleRenderer;
    
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

	float rayIntersectsWithMesh(vec3 pos, vec3 dir, MeshObject* obj, vec3 &norm) {
		float distance = std::numeric_limits<float>::infinity();
		vec3 n;
		if (obj == nullptr) return distance;

		for (int k = 0; k < obj->colliderCount; ++k) {
			if (obj->collider[k] != nullptr) {
				float dist;
				if (obj->collider[k]->IntersectsWith(pos, dir, dist, n)) {
					distance = Kore::min(dist, distance);
					norm = n;
				}
			}
		}
		return distance;
	}

	KitchenObject* getIntersectingMesh(vec3 pos, vec3 dir, float &distMin, vec3 &normMin) {
		distMin = std::numeric_limits<float>::infinity();
		vec3 norm;
		KitchenObject* result = nullptr;
		for (unsigned oi = 0; kitchenObjects[oi] != nullptr; ++oi) {
			if (!kitchenObjects[oi]->visible) continue;
			
			float dist = rayIntersectsWithMesh(pos, dir, kitchenObjects[oi]->body, norm);
			if (dist < distMin) {
				distMin = dist;
				normMin = norm;
				result = kitchenObjects[oi];
			}
			if (kitchenObjects[oi]->door_closed != nullptr && kitchenObjects[oi]->closed) {
				dist = rayIntersectsWithMesh(pos, dir, kitchenObjects[oi]->door_closed, norm);
				if (dist < distMin) {
					distMin = dist;
					normMin = norm;
					result = kitchenObjects[oi];
				}
			}
		}

		for (unsigned oi = 0; roomObjects[oi] != nullptr; ++oi) {
			float d = rayIntersectsWithMesh(pos, dir, roomObjects[oi], norm);
			if (d < distMin) {
				distMin = d;
				normMin = norm;
				result = nullptr;
			}
		}
		return result;
	}
    
    void update() {
        double t = System::time() - startTime;
        double deltaT = t - lastTime;
        
        lastTime = t;
        Kore::Audio2::update();
        
        Graphics4::begin();
        Graphics4::clear(Graphics4::ClearColorFlag | Graphics4::ClearDepthFlag | Graphics4::ClearStencilFlag, 0xFF0000FF, 1.0f, 0);
        
        // Important: We need to set the program before we set a uniform
		Graphics4::setPipeline(program);
        
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
		vec3 norm;
		hovered = getIntersectingMesh(cameraPos, cameraDir, distMin, norm);
        
        View = mat4::lookAlong(cameraDir, cameraPos, cameraUp);
        
        Graphics4::setMatrix(pLocation, P);
        Graphics4::setMatrix(vLocation, View);
        
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
        
		Graphics4::setTextureAddressing(tex, Graphics4::U, Graphics4::Repeat);
		Graphics4::setTextureAddressing(tex, Graphics4::V, Graphics4::Repeat);

        // render the room
        Kore::Graphics4::setMatrix(mLocation, rooM);

		for (unsigned oi = 0; roomObjects[oi] != nullptr; ++oi) {
			roomObjects[oi]->render(tex, mLocation);
		}
        
		Graphics4::setPipeline(instancedProgram);
        
        Graphics4::setMatrix(instancedPLocation, P);
        Graphics4::setMatrix(instancedVLocation, View);
        
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
            g2->setColor(Graphics1::Color::White);
        }
		else if (hovered->pizza) {
            g2->setColor(Graphics1::Color::Red);
		}
		else if (hovered->door_closed == nullptr) {
			g2->setColor(Graphics1::Color::Green);
		}
		else {
			g2->setColor(Graphics1::Color::Blue);
		}
		g2->drawRect(width / 2 -  1, height / 2 -  1, 2, 2, 1);
		g2->drawRect(width / 2 +  8, height / 2 -  1, 8, 2, 1);
		g2->drawRect(width / 2 - 16, height / 2 -  1, 8, 2, 1);
		g2->drawRect(width / 2 -  1, height / 2 +  8, 2, 8, 1);
		g2->drawRect(width / 2 -  1, height / 2 - 16, 2, 8, 1);
		g2->end();
        
        g2->setFont(font24);
        g2->setFontColor(Graphics1::Color::Black);
        g2->setFontSize(24);
        char pizza_text[42];
        sprintf(pizza_text, "You have %i pizza", maxPizza-pizzaCount);
        g2->drawString(pizza_text, 10, 10);
        
        Graphics4::end();
		Graphics4::swapBuffers();
    }
    
    void keyDown(KeyCode code) {
        if (code == KeyUp) {
            up_A = true;
        } else if (code == KeyDown) {
            down_A = true;
        } else if (code == KeyLeft) {
            right_A = true;
        } else if (code == KeyRight) {
            left_A = true;
        } else if (code == KeyW) {
			up_C = true;
		} else if (code == KeyS) {
			down_C = true;
		} else if (code == KeyA) {
			right_C = true;
		} else if (code == KeyD) {
			left_C = true;
		} else if (code == KeyControl) {
			crouch = true;
		} else if (code == KeySpace) {
			jump = true;
		} else if (code == KeyR) {
			for (int i = 0; i < maxPizza; ++i) {
				if (kitchenObjects[PIZZA_OFFSET + i] != nullptr) {
					if (kitchenObjects[PIZZA_OFFSET + i] == hovered) hovered = nullptr;

					Ant::lessPizza(kitchenObjects[PIZZA_OFFSET + i]->readOnlyPos);

					delete kitchenObjects[PIZZA_OFFSET + i]->body;
					delete kitchenObjects[PIZZA_OFFSET + i];

					kitchenObjects[PIZZA_OFFSET + i] = nullptr;
				}
			}
			pizzaCount = 0;
		} else if (code == KeyEscape) {
            Kore::System::stop();
        } else if (code == KeyL) {
            Kore::log(Kore::Info, "Camera pos %f %f %f", cameraPos.x(), cameraPos.y(), cameraPos.z());
            Kore::log(Kore::Info, "Camera angle horizontal %f", horizontalAngle);
            Kore::log(Kore::Info, "Camera angle vertical %f", verticalAngle);
        } else if (code == KeyT) {
            int i = 0;
            while (kitchenObjects[i] != nullptr) {
                kitchenObjects[i]->openOrClose(lastTime);
                ++i;
            }
        }
    }
    
    void keyUp(KeyCode code) {
		if (code == KeyUp) {
			up_A = false;
		}
		else if (code == KeyDown) {
			down_A = false;
		}
		else if (code == KeyLeft) {
			right_A = false;
		}
		else if (code == KeyRight) {
			left_A = false;
		}
		else if (code == KeyW) {
			up_C = false;
		}
		else if (code == KeyS) {
			down_C = false;
		}
		else if (code == KeyA) {
			right_C = false;
		}
		else if (code == KeyD) {
			left_C = false;
		}
		else if (code == KeyControl) {
			crouch = false;
		}
		else if (code == KeySpace) {
			jump = false;
		}
    }
    
	double lastMouseTime = 0;

    void mouseMove(int windowId, int x, int y, int movementX, int movementY) {
		double t = System::time() - startTime;
		double deltaT = t - lastMouseTime;
		lastMouseTime = t;
		if (deltaT > 1.0f / 30.0f) return;

        horizontalAngle += CAMERA_ROTATION_SPEED * movementX * deltaT * 7.0f;
        verticalAngle -= CAMERA_ROTATION_SPEED * movementY * deltaT * 7.0f;
		verticalAngle = Kore::min(Kore::max(verticalAngle, -0.49f * pi), 0.49f * pi);
    }
    
    void mousePress(int windowId, int button, int x, int y) {
		if (button == 0) {
			vec3 norm;
			float dist = std::numeric_limits<float>::infinity();
			hovered = getIntersectingMesh(cameraPos, cameraDir, dist, norm);

			if (hovered != nullptr && hovered->pizza) {
				for (int i = 0; i < pizzaCount; ++i) {
					if (kitchenObjects[PIZZA_OFFSET + i] == hovered) {
						Ant::lessPizza(kitchenObjects[PIZZA_OFFSET + i]->readOnlyPos);
						kitchenObjects[PIZZA_OFFSET + i] = kitchenObjects[PIZZA_OFFSET + pizzaCount - 1];
						kitchenObjects[PIZZA_OFFSET + pizzaCount - 1] = nullptr;

						delete hovered->body;
						delete hovered;
						hovered = nullptr;

						--pizzaCount;
					}
				}
			}
			else if (dist < std::numeric_limits<float>::infinity() && pizzaCount < maxPizza) {
				vec3 pos = cameraPos + cameraDir * dist;
				vec3 rot(0.0f, 0.0f, 0.0f);
				if (norm.y() < -0.9f) rot.y() = pi;
				if (norm.x() > 0.9f) rot.y() = -0.5f * pi;
				if (norm.x() < -0.9f) rot.y() = 0.5f * pi;
				if (norm.z() > 0.9f) rot.z() = 0.5f * pi;
				if (norm.z() < -0.9f) rot.z() = -0.5f * pi;

				MeshObject* pizza = new MeshObject("Data/Meshes/pizza.obj", "Data/Meshes/pizza_collider.obj", "Data/Textures/pizza.png", structure, 1.0f);
				kitchenObjects[PIZZA_OFFSET + pizzaCount] = new KitchenObject(pizza, nullptr, nullptr, pos, rot, true);
				Ant::morePizze(pos);

				++pizzaCount;
				kitchenObjects[PIZZA_OFFSET + pizzaCount] = nullptr;
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
        instancedVertexShader = new Graphics4::Shader(vs.readAll(), vs.size(), Graphics4::VertexShader);
        instancedFragmentShader = new Graphics4::Shader(fs.readAll(), fs.size(), Graphics4::FragmentShader);
        
        // This defines the structure of your Vertex Buffer
		Graphics4::VertexStructure** structures = new Graphics4::VertexStructure*[2];
        structures[0] = new Graphics4::VertexStructure();
        structures[0]->add("pos", Graphics4::Float3VertexData);
        structures[0]->add("tex", Graphics4::Float2VertexData);
        structures[0]->add("nor", Graphics4::Float3VertexData);
        
        structures[1] = new Graphics4::VertexStructure();
        structures[1]->add("M", Graphics4::Float4x4VertexData);
        structures[1]->add("N", Graphics4::Float4x4VertexData);
        structures[1]->add("tint", Graphics4::Float4VertexData);
        
        instancedProgram = new Graphics4::PipelineState;
		instancedProgram->inputLayout[0] = structures[0];
		instancedProgram->inputLayout[1] = structures[1];
		instancedProgram->inputLayout[2] = nullptr;
        instancedProgram->vertexShader = instancedVertexShader;
        instancedProgram->fragmentShader = instancedFragmentShader;
		instancedProgram->compile();
        
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
        vertexShader = new Graphics4::Shader(vs2.readAll(), vs2.size(), Graphics4::VertexShader);
        fragmentShader = new Graphics4::Shader(fs2.readAll(), fs2.size(), Graphics4::FragmentShader);
        
        // This defines the structure of your Vertex Buffer
		structure = Graphics4::VertexStructure();
        structure.add("pos", Graphics4::Float3VertexData);
        structure.add("tex", Graphics4::Float2VertexData);
        structure.add("nor", Graphics4::Float3VertexData);
        
        program = new Graphics4::PipelineState;
		program->inputLayout[0] = &structure;
		program->inputLayout[1] = nullptr;
        program->vertexShader = vertexShader;
        program->fragmentShader = fragmentShader;
		program->blendSource = Graphics4::SourceAlpha;
		program->blendDestination = Graphics4::InverseSourceAlpha;
		program->depthMode = Graphics4::ZCompareLess;
		program->depthWrite = true;
		program->compile();
        
        tex = program->getTextureUnit("tex");
        
        pLocation = program->getConstantLocation("P");
        vLocation = program->getConstantLocation("V");
        mLocation = program->getConstantLocation("M");

		rooM = mat4::Translation(0, -1.0f, 6.5f);
        roomObjects[0] = new MeshObject("Data/Meshes/room_floor.obj", "Data/Meshes/room_floor_collider.obj", "Data/Textures/marble_tile.png", structure, 1.0f);
		roomObjects[0]->collider[0]->trans(rooM);
		roomObjects[1] = new MeshObject("Data/Meshes/room_wall1.obj", "Data/Meshes/room_wall1_collider.obj", "Data/Textures/omi_tapete.png", structure, 1.0f);
		roomObjects[1]->collider[0]->trans(rooM);
		roomObjects[2] = new MeshObject("Data/Meshes/room_wall2.obj", "Data/Meshes/room_wall2_collider.obj", "Data/Textures/omi_tapete.png", structure, 1.0f);
		roomObjects[2]->collider[0]->trans(rooM);
		roomObjects[3] = new MeshObject("Data/Meshes/room_wall3.obj", "Data/Meshes/room_wall3_collider.obj", "Data/Textures/omi_tapete.png", structure, 1.0f);
		roomObjects[3]->collider[0]->trans(rooM);
		roomObjects[4] = new MeshObject("Data/Meshes/room_wall4.obj", "Data/Meshes/room_wall4_collider.obj", "Data/Textures/omi_tapete.png", structure, 1.0f);
		roomObjects[4]->collider[0]->trans(rooM);
		roomObjects[5] = new MeshObject("Data/Meshes/room_ceiling.obj", "Data/Meshes/room_ceiling_collider.obj", "Data/Textures/ceilingTexture.png", structure, 1.0f);
		roomObjects[5]->collider[0]->trans(rooM);
		roomObjects[6] = nullptr;

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
		
		MeshObject* img = new MeshObject("Data/Meshes/credits.obj", nullptr, "Data/Textures/creditsTexture.png", structure, 1.0f);
		kitchenObjects[19] = new KitchenObject(img, nullptr, nullptr, vec3(-7.95f, 5.0f, 7.0f), vec3(-pi * 0.5f, 0.0f, 0.0f));
		
		MeshObject* lamp = new MeshObject("Data/Meshes/lamp.obj", nullptr, "Data/Textures/lampTexture.png", structure, 1.0f);
		kitchenObjects[20] = new KitchenObject(lamp, nullptr, nullptr, vec3(0.0f, 9.0f, 7.0f), vec3(0.0f, 0.0f, 0.0f));

		kitchenObjects[21] = nullptr;

		hovered = nullptr;

        Random::init(System::time() * 100);
        
        Ant::init();
        
        Graphics4::setTextureAddressing(tex, Graphics4::U, Graphics4::Repeat);
        Graphics4::setTextureAddressing(tex, Graphics4::V, Graphics4::Repeat);
        
        P = mat4::Perspective(45, (float)width / (float)height, 0.1f, 1000);
        
		g2 = new Graphics2::Graphics2(width, height);

        font14 = Kravur::load("Data/Fonts/arial", FontStyle(), 14);
        font24 = Kravur::load("Data/Fonts/arial", FontStyle(), 24);
        font34 = Kravur::load("Data/Fonts/arial", FontStyle(), 34);
        font44 = Kravur::load("Data/Fonts/arial", FontStyle(), 44);
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

	Kore::Audio1::init();
	Kore::Audio2::init();

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
