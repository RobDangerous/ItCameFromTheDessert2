#include "pch.h"

#include <Kore/IO/FileReader.h>
#include <Kore/Graphics2/Graphics.h>
#include <Kore/Graphics4/Graphics.h>
#include <Kore/Graphics4/PipelineState.h>
#include <Kore/Graphics1/Color.h>
#include <Kore/Input/Keyboard.h>
#include <Kore/Input/Mouse.h>
#include <Kore/System.h>
#include <Kore/Log.h>

#include "MeshObject.h"
#include "Ant.h"
#include "Kitchen.h"

using namespace Kore;
using namespace Kore::Graphics4;

vec3 cameraPos = vec3(0, 0, 0);					// x - left, right, y - up, down, z - forward, backward

namespace {
	const int width = 1024;
	const int height = 768;
	
	const float CAMERA_ROTATION_SPEED = 0.05f;
	const float CAMERA_MOVE_SPEED = 4.f;

	double startTime;
	double lastTime;

	// Avatar shader
	VertexStructure structure;
	Shader* vertexShader;
	Shader* fragmentShader;
	PipelineState* pipeline;

	TextureUnit tex;
	ConstantLocation pLocation;
	ConstantLocation vLocation;
	ConstantLocation mLocation;

	// Living room shader
	VertexStructure structure_living_room;
	Shader* vertexShader_living_room;
	Shader* fragmentShader_living_room;
	PipelineState* pipeline_living_room;

	TextureUnit tex_living_room;
	ConstantLocation pLocation_living_room;
	ConstantLocation vLocation_living_room;
	ConstantLocation mLocation_living_room;
	ConstantLocation mLocation_living_room_inverse;
	ConstantLocation diffuse_living_room;
	ConstantLocation specular_living_room;
	ConstantLocation specular_power_living_room;
	ConstantLocation lightPosLocation_living_room;
	ConstantLocation lightCount_living_room;
	
	Kore::Graphics2::Graphics2* g2;
	Kravur* font14;
	Kravur* font24;
	Kravur* font34;
	Kravur* font44;

	// Keyboard controls
	bool rotate = false;
	bool W, A, S, D = false;
	bool F, L, B, R = false;
	//bool Z, X = false;

	vec3 cameraUp;
	vec3 right;
	vec3 forward;
	float horizontalAngle = -1.24f * pi;
	float verticalAngle = -0.5f;

	Ant* ant;
	Kitchen* kitchen;

	void renderAnt(mat4 V, mat4 P) {
		Graphics4::setPipeline(pipeline_living_room);

		//Ant::setLights(lightCount_living_room, lightPosLocation_living_room, livingRoom);
		Graphics4::setMatrix(vLocation_living_room, V);
		Graphics4::setMatrix(pLocation_living_room, P);
		Ant::render(tex_living_room, mLocation_living_room, mLocation_living_room_inverse, diffuse_living_room, specular_living_room, specular_power_living_room);
	}
	
	void renderKitchen(mat4 V, mat4 P) {
		Graphics4::setPipeline(pipeline_living_room);
		
		kitchen->setLights(lightCount_living_room, lightPosLocation_living_room);
		Graphics4::setMatrix(vLocation_living_room, V);
		Graphics4::setMatrix(pLocation_living_room, P);
		kitchen->render(tex_living_room, mLocation_living_room, mLocation_living_room_inverse, diffuse_living_room, specular_living_room, specular_power_living_room);
	}

	Kore::mat4 getProjectionMatrix() {
		mat4 P = mat4::Perspective(45, (float)width / (float)height, 0.01f, 1000);
		P.Set(0, 0, -P.get(0, 0));
		return P;
	}

	Kore::mat4 getViewMatrix() {
		// Calculate camera direction
		vec3 cameraDir = vec3(Kore::cos(verticalAngle) * Kore::sin(horizontalAngle),
							  Kore::sin(verticalAngle),
							  Kore::cos(verticalAngle) * Kore::cos(horizontalAngle));
		
		// Re-calculate the orthonormal up vector
		cameraUp = right.cross(forward);  // cross product
		cameraUp.normalize();
		
		mat4 V = mat4::lookAlong(cameraDir, cameraPos, cameraUp);
		return V;
	}

	void update() {
		float t = (float)(System::time() - startTime);
		double deltaT = t - lastTime;
		lastTime = t;

		cameraUp = vec3(0, 1, 0);
		right = vec3(Kore::sin(horizontalAngle - pi / 2.0),
					 0,
					 Kore::cos(horizontalAngle - pi / 2.0));
		
		forward = cameraUp.cross(right);  // cross product
		
		// Move position of camera based on WASD keys
		if (S || B) {
			cameraPos -= forward * (float) deltaT * CAMERA_MOVE_SPEED;
		}
		if (W || F) {
			cameraPos += forward * (float) deltaT * CAMERA_MOVE_SPEED;
		}
		if (A || L) {
			cameraPos -= right * (float)deltaT * CAMERA_MOVE_SPEED;
		}
		if (D || R) {
			cameraPos += right * (float)deltaT * CAMERA_MOVE_SPEED;
		}

		Graphics4::begin();
		Graphics4::clear(Graphics4::ClearColorFlag | Graphics4::ClearDepthFlag, Graphics1::Color::Green, 1.0f, 0);

		Graphics4::setPipeline(pipeline);
		
		// Get projection and view matrix
		mat4 P = getProjectionMatrix();
		mat4 V = getViewMatrix();

		Graphics4::setMatrix(vLocation, V);
		Graphics4::setMatrix(pLocation, P);

		// Render kitchen
		renderKitchen(V, P);

		Ant::moveEverybody(1.0f / 60.0f);
		renderAnt(V, P);
		
		kitchen->highlightTheClosestObject(vec4(cameraPos.x(), cameraPos.y(), cameraPos.z(), 1.0));
		if(kitchen->canOpen()) {
			const char* text = "Press Space to open or close the door";
			//log(Info, "%s", text);
			
			g2->begin(false, width, height, false);
			g2->setFont(font44);
			g2->drawString(text, 10, 10);
			g2->end();
		}
		
		Graphics4::end();
		Graphics4::swapBuffers();
	}

	void keyDown(KeyCode code) {
		switch (code) {
		case Kore::KeyW:
			W = true;
			break;
		case Kore::KeyA:
			A = true;
			break;
		case Kore::KeyS:
			S = true;
			break;
		case Kore::KeyD:
			D = true;
			break;
		case Kore::KeyLeft:
			L = true;
			break;
		case Kore::KeyRight:
			R = true;
			break;
		case Kore::KeyUp:
			F = true;
			break;
		case Kore::KeyDown:
			B = true;
			break;
		case Kore::KeySpace:
			kitchen->openTheDoor();
			break;
		case Kore::KeyR:
#ifdef KORE_STEAMVR
			VrInterface::resetHmdPose();
#endif
			break;
		case KeyL:
			Kore::log(Kore::LogLevel::Info, "Position: (%f, %f, %f)", cameraPos.x(), cameraPos.y(), cameraPos.z());
			Kore::log(Kore::LogLevel::Info, "Rotation: (%f, %f)", verticalAngle, horizontalAngle);
			break;
		case Kore::KeyEscape:
		case KeyQ:
			System::stop();
			break;
		default:
			break;
		}
	}

	void keyUp(KeyCode code) {
		switch (code) {
		case Kore::KeyW:
			W = false;
			break;
		case Kore::KeyA:
			A = false;
			break;
		case Kore::KeyS:
			S = false;
			break;
		case Kore::KeyD:
			D = false;
			break;
		case Kore::KeyLeft:
			L = false;
			break;
		case Kore::KeyRight:
			R = false;
			break;
		case Kore::KeyUp:
			F = false;
			break;
		case Kore::KeyDown:
			B = false;
			break;
		default:
			break;
		}
	}

	double lastMouseTime = 0;
	void mouseMove(int windowId, int x, int y, int movementX, int movementY) {
		double t = System::time() - startTime;
		double deltaT = t - lastMouseTime;
		lastMouseTime = t;
		if (deltaT > 1.0f / 30.0f) return;
		
		horizontalAngle -= CAMERA_ROTATION_SPEED * movementX * deltaT * 7.0f;
		verticalAngle -= CAMERA_ROTATION_SPEED * movementY * deltaT * 7.0f;
		verticalAngle = Kore::min(Kore::max(verticalAngle, -0.49f * pi), 0.49f * pi);
		
	}

	void mousePress(int windowId, int button, int x, int y) {
		rotate = true;
	}

	void mouseRelease(int windowId, int button, int x, int y) {
		rotate = false;
	}

	void loadShader() {
		FileReader vs("shader.vert");
		FileReader fs("shader.frag");
		vertexShader = new Shader(vs.readAll(), vs.size(), VertexShader);
		fragmentShader = new Shader(fs.readAll(), fs.size(), FragmentShader);

		// This defines the structure of your Vertex Buffer
		structure.add("pos", Float3VertexData);
		structure.add("tex", Float2VertexData);
		structure.add("nor", Float3VertexData);

		pipeline = new PipelineState;
		pipeline->inputLayout[0] = &structure;
		pipeline->inputLayout[1] = nullptr;
		pipeline->vertexShader = vertexShader;
		pipeline->fragmentShader = fragmentShader;
		pipeline->depthMode = ZCompareLess;
		pipeline->depthWrite = true;
		pipeline->blendSource = Graphics4::SourceAlpha;
		pipeline->blendDestination = Graphics4::InverseSourceAlpha;
		pipeline->alphaBlendSource = Graphics4::SourceAlpha;
		pipeline->alphaBlendDestination = Graphics4::InverseSourceAlpha;
		pipeline->compile();

		tex = pipeline->getTextureUnit("tex");
		Graphics4::setTextureAddressing(tex, Graphics4::U, Repeat);
		Graphics4::setTextureAddressing(tex, Graphics4::V, Repeat);

		pLocation = pipeline->getConstantLocation("P");
		vLocation = pipeline->getConstantLocation("V");
		mLocation = pipeline->getConstantLocation("M");
	}

	void loadLivingRoomShader() {
		// Load shader for living room
		FileReader vs("shader_living_room.vert");
		FileReader fs("shader_living_room.frag");
		vertexShader_living_room = new Shader(vs.readAll(), vs.size(), VertexShader);
		fragmentShader_living_room = new Shader(fs.readAll(), fs.size(), FragmentShader);

		structure_living_room.add("pos", Float3VertexData);
		structure_living_room.add("tex", Float2VertexData);
		structure_living_room.add("nor", Float3VertexData);

		pipeline_living_room = new PipelineState;
		pipeline_living_room->inputLayout[0] = &structure_living_room;
		pipeline_living_room->inputLayout[1] = nullptr;
		pipeline_living_room->vertexShader = vertexShader_living_room;
		pipeline_living_room->fragmentShader = fragmentShader_living_room;
		pipeline_living_room->depthMode = ZCompareLess;
		pipeline_living_room->depthWrite = true;
		pipeline_living_room->blendSource = Graphics4::SourceAlpha;
		pipeline_living_room->blendDestination = Graphics4::InverseSourceAlpha;
		pipeline_living_room->alphaBlendSource = Graphics4::SourceAlpha;
		pipeline_living_room->alphaBlendDestination = Graphics4::InverseSourceAlpha;
		pipeline_living_room->compile();

		tex_living_room = pipeline_living_room->getTextureUnit("tex");
		Graphics4::setTextureAddressing(tex_living_room, Graphics4::U, Repeat);
		Graphics4::setTextureAddressing(tex_living_room, Graphics4::V, Repeat);

		pLocation_living_room = pipeline_living_room->getConstantLocation("P");
		vLocation_living_room = pipeline_living_room->getConstantLocation("V");
		mLocation_living_room = pipeline_living_room->getConstantLocation("M");
		mLocation_living_room_inverse = pipeline_living_room->getConstantLocation("MInverse");
		diffuse_living_room = pipeline_living_room->getConstantLocation("diffuseCol");
		specular_living_room = pipeline_living_room->getConstantLocation("specularCol");
		specular_power_living_room = pipeline_living_room->getConstantLocation("specularPow");
		lightPosLocation_living_room = pipeline_living_room->getConstantLocation("lightPos");
		lightCount_living_room = pipeline_living_room->getConstantLocation("numLights");
	}

	void init() {
		loadShader();
		loadLivingRoomShader();
		
		cameraPos = vec3(-1.5, 2.5, 5.5);
		verticalAngle = 0;
		horizontalAngle = Kore::pi/1.1;

		kitchen = new Kitchen();
		
		Ant::init();
		ant = new Ant;
		
		font14 = Kravur::load("Data/Fonts/arial", FontStyle(), 14);
		font24 = Kravur::load("Data/Fonts/arial", FontStyle(), 24);
		font34 = Kravur::load("Data/Fonts/arial", FontStyle(), 34);
		font44 = Kravur::load("Data/Fonts/arial", FontStyle(), 44);
		g2 = new Graphics2::Graphics2(width, height);
		g2->setFont(font44);
	}

}

int kore(int argc, char** argv) {
	System::init("ItCameFromTheDessert2", width, height, 4);
	
	init();

	System::setCallback(update);

	startTime = System::time();

	Keyboard::the()->KeyDown = keyDown;
	Keyboard::the()->KeyUp = keyUp;
	Mouse::the()->Move = mouseMove;
	Mouse::the()->Press = mousePress;
	Mouse::the()->Release = mouseRelease;
#ifdef NDEBUG
	Mouse::the()->lock(0);
#endif
	System::start();

	return 0;
}
