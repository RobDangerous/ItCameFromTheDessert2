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

#include "Projectiles.h"
//#include "Steering.h"
#include "TankSystem.h"

#include "Tank.h"

using namespace Kore;

const int MAP_SIZE_INNER = 200;
const int MAP_SIZE_OUTER = 300;

namespace {
	const int width = 800;
	const int height = 600;

	int mouseX = width / 2;
	int mouseY = height / 2;

	double startTime;
	Shader* vertexShader;
	Shader* fragmentShader;
	Program* program;

	float cameraAngle = 0.0f;
	float cameraZoom = 0.5f;

	bool left;
	bool right;
	bool up;
	bool down;
	
	mat4 P;
	mat4 View;
	//mat4 PV;

	vec3 cameraPosition;
	//vec3 targetCameraPosition;
	//vec3 oldCameraPosition;

	vec3 lookAt;
	//vec3 targetLookAt;
	//vec3 oldLookAt;

	float lightPosX;
	float lightPosY;
	float lightPosZ;

	MeshObject* sphereMesh;
	MeshObject* projectileMesh;
	PhysicsObject* spherePO;

	Projectiles* projectiles;

	PhysicsWorld physics;
	
	TextureUnit tex;
	ConstantLocation pLocation;
	ConstantLocation vLocation;
	ConstantLocation lightPosLocation;

	BoxCollider boxCollider(vec3(-46.0f, -4.0f, 44.0f), vec3(10.6f, 4.4f, 4.0f));

	Texture* particleImage;
	ParticleSystem* particleSystem;
    Explosion* explosionSystem;
    
//    Steering* steer;

	double lastTime;

	InstancedMeshObject* tankTop;
	InstancedMeshObject* tankBottom;
    InstancedMeshObject* tankFlag;
	TankSystem* tankTics;
    ParticleRenderer* particleRenderer;

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

	void update() {
		double t = System::time() - startTime;
		double deltaT = t - lastTime;

		lastTime = t;
		Kore::Audio::update();
		
		Graphics::begin();
		Graphics::clear(Graphics::ClearColorFlag | Graphics::ClearDepthFlag | Graphics::ClearStencilFlag, 0xff9999FF, 1.0f, 0);

		// Important: We need to set the program before we set a uniform
		program->set();
		Graphics::setBlendingMode(SourceAlpha, Kore::BlendingOperation::InverseSourceAlpha);
		Graphics::setRenderState(BlendingState, true);
		Graphics::setRenderState(BackfaceCulling, NoCulling);

		// set the camera
		cameraAngle += 0.3f * deltaT;

		float x = 0 + 10 * Kore::cos(cameraAngle);
		float z = 0 + 10 * Kore::sin(cameraAngle);
		
		// Interpolate the camera to not follow small physics movements
		float alpha = 0.3f;

		const float cameraSpeed = 1.5f;
		if (mouseY < 50) {
			cameraPosition.z() += cameraSpeed * clamp01(1 - mouseY / 50.0f);
		}
		if (mouseY > height - 50) {
			cameraPosition.z() -= cameraSpeed * clamp01((mouseY - height + 50) / 50.0f);
		}
		if (mouseX < 50) {
			cameraPosition.x() -= cameraSpeed * clamp01(1 - mouseX / 50.0f);
		}
		if (mouseX > width - 50) {
			cameraPosition.x() += cameraSpeed * clamp01((mouseX - width + 50) / 50.0f);
		}
		
		cameraPosition.y() = cameraZoom * 150 + (1 - cameraZoom) * 10;
		vec3 off = vec3(0, -1, 0) * cameraZoom + (1 - cameraZoom) * vec3(0, -1, 1);
		lookAt = cameraPosition + off;
		
		// Follow the ball with the camera
		P = mat4::Perspective(0.5f * pi, (float)width / (float)height, 0.1f, 1000);
		View = mat4::lookAt(cameraPosition, lookAt, vec3(0, 0, 1));

		Graphics::setMatrix(pLocation, P);
		Graphics::setMatrix(vLocation, View);

		// update light pos
		lightPosX = 100; //20 * Kore::sin(2 * t);
		lightPosY = 100; // 10;
		lightPosZ = 100; //20 * Kore::cos(2 * t);
		Graphics::setFloat3(lightPosLocation, lightPosX, lightPosY, lightPosZ);

		// Handle inputs
		float forceX = 0.0f;
		float forceZ = 0.0f;
		if (up) forceX += 1.0f;
		if (down) forceX -= 1.0f;
		if (left) forceZ -= 1.0f;
		if (right) forceZ += 1.0f;

		// Apply inputs
		vec3 force(forceX, 0.0f, forceZ);
		force = force * 20.0f;
		spherePO->ApplyForceToCenter(force);

        // Update physics
        physics.Update(deltaT);
    
		tankTics->update(deltaT);

		Graphics::setStencilParameters(Kore::ZCompareAlways, Replace, Keep, Keep, 1, 0xff, 0xff);
		tankTics->render(tex, View, vLocation);
		
        // Update physics
        physics.Update(deltaT);

		// Check for game over
		bool result = spherePO->Collider.IntersectsWith(boxCollider);
		if (result) {
			// ...
		}
        
        // Render dynamic objects
        /*for (int i = 0; i < physics.currentDynamicObjects; i++) {
            PhysicsObject** currentP = &physics.dynamicObjects[i];
            (*currentP)->UpdateMatrix();
            (*currentP)->Mesh->render(tex, View);
        }

		// Render static objects
		for (int i = 0; i < physics.currentStaticColliders; i++) {
			TriangleMeshCollider** current = &physics.staticColliders[i];
			(*current)->mesh->render(tex, View);
		}*/

		Graphics::setStencilParameters(ZCompareEqual, Keep, Keep, Keep, 0, 0xff, 0);
		renderLandscape(tex);

		Graphics::setStencilParameters(ZCompareAlways, Keep, Keep, Keep, 0, 0xff, 0xff);
		// Update and render particles
		particleSystem->setPosition(spherePO->GetPosition());
		particleSystem->setDirection(vec3(-spherePO->Velocity.x(), 3, -spherePO->Velocity.z()));
		particleSystem->update(deltaT);
		particleSystem->render(tex, vLocation, View);

		projectiles->update(deltaT);
		projectiles->render(vLocation, tex, View);
        
        particleRenderer->render(tex, View, vLocation);

		Graphics::end();
		Graphics::swapBuffers();
	}

	void ResetSphere(vec3 Position, vec3 Velocity) {
		spherePO->SetPosition(Position);
		spherePO->Velocity = Velocity;
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
		}
	}

	void mouseMove(int windowId, int x, int y, int movementX, int movementY) {
		mouseX = x;
		mouseY = y;

		vec3 position = screenToWorld(vec2(mouseX, mouseY));
		vec3 pickDir = vec3(position.x(), position.y(), position.z()) - cameraPosition;
		pickDir.normalize();
				
		static int pick = 0;
		/*for (unsigned i = 0; i < tanks.size(); ++i) {
			if (tanks[i]->Collider.IntersectsWith(cameraPosition, pickDir)) {
				log(Info, "Picky %i", pick++);
			}
		}*/
	}
	
	void mousePress(int windowId, int button, int x, int y) {
		//projectiles->fire(cameraPosition, lookAt - cameraPosition, 10, 1);
		/*if(!tanks.empty()) {
			vec3 p = tanks.front()->getPosition();
			vec3 l = tanks.front()->getTurretLookAt();
			projectiles->fire(p, l, 10);
			log(Info, "Boom! (%f, %f, %f) -> (%f, %f, %f)", p.x(), p.y(), p.z(), l.x(), l.y(), l.z());
		}*/
	}

	void mouseRelease(int windowId, int button, int x, int y) {
		
	}

	void mouseScroll(int windowId, int delta) {
		cameraZoom = clamp(0.0f, 1.0f, cameraZoom + delta * 0.05f);
	}
	
	void init() {
		FileReader vs("shader.vert");
		FileReader fs("shader.frag");
		vertexShader = new Shader(vs.readAll(), vs.size(), VertexShader);
		fragmentShader = new Shader(fs.readAll(), fs.size(), FragmentShader);

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

		program = new Program;
		program->setVertexShader(vertexShader);
		program->setFragmentShader(fragmentShader);
		program->link(structures, 2);

		tex = program->getTextureUnit("tex");
		pLocation = program->getConstantLocation("P");
		vLocation = program->getConstantLocation("V");
		lightPosLocation = program->getConstantLocation("lightPos");
		
		sphereMesh = new MeshObject("cube.obj", "cube.png", structures);
		projectileMesh = new MeshObject("projectile.obj", "projectile.png", structures, PROJECTILE_SIZE);

		spherePO = new PhysicsObject(TANK, 5, true, false);
		spherePO->Collider.radius = 0.5f;
		spherePO->Mesh = sphereMesh;
		physics.AddDynamicObject(spherePO);

		ResetSphere(vec3(-10, 5.5f, 10), vec3(0, 0, 0));
        
        particleImage = new Texture("particle.png", true);
        particleSystem = new ParticleSystem(spherePO->GetPosition(), vec3(0, 10, 0), 1.0f, 3.0f, vec4(2.5f, 0, 0, 1), vec4(0, 0, 0, 0), 10, 100, structures, particleImage);
        particleRenderer = new ParticleRenderer(structures);
        projectiles = new Projectiles(1000, 20, particleImage, projectileMesh, structures, &physics);
        
		TriangleMeshCollider* tmc = new TriangleMeshCollider();
		tmc->mesh = new MeshObject("level.obj", "level.png", structures);
		//physics.AddStaticCollider(tmc);

		tankTop = new InstancedMeshObject("tank_top.obj", "tank_top_uv.png", structures, MAX_TANKS, 8);
		tankBottom = new InstancedMeshObject("tank_bottom.obj", "tank_bottom_uv.png", structures, MAX_TANKS, 10);
		tankFlag = new InstancedMeshObject("flag.obj", "flag_uv.png", structures, MAX_TANKS, 2);

        tankTics = new TankSystem(particleRenderer, tankBottom, tankTop, tankFlag, vec3(-MAP_SIZE_INNER / 2, 6, -MAP_SIZE_INNER / 2), vec3(-MAP_SIZE_INNER / 2, 6, MAP_SIZE_INNER / 2), vec3(MAP_SIZE_INNER / 2, 6, -MAP_SIZE_INNER / 2), vec3(MAP_SIZE_INNER / 2, 6, MAP_SIZE_INNER / 2), 3, projectiles);

		Graphics::setRenderState(DepthTest, true);
		Graphics::setRenderState(DepthTestCompare, ZCompareLess);

		Graphics::setTextureAddressing(tex, U, Repeat);
		Graphics::setTextureAddressing(tex, V, Repeat);

        
        explosionSystem = new Explosion(vec3(2,6,0), 2.f, 10.f, 300, structures, particleImage);

		cameraPosition = vec3(0, 0.5f, 0);
		cameraZoom = 0.5f;
		lookAt = spherePO->GetPosition();
        
//        steer = new Steering;
        
        Random::init(123);

		createLandscape(structures, MAP_SIZE_OUTER);
	}
}

int kore(int argc, char** argv) {
	Kore::System::setName("Korerorinpa");
	Kore::System::setup();

	Kore::WindowOptions options;
	options.title = "Korerorinpa";
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
	Mouse::the()->Release = mouseRelease;
	Mouse::the()->Scroll = mouseScroll;

	Kore::System::start();

	return 0;
}
