#pragma once

#include <vector>

#include "Steering.h"

#include "Engine/Collision.h"
#include "Engine/PhysicsObject.h"
#include "Engine/Explosion.h"
#include "Projectiles.h"
#include "Landscape.h"

enum StateMachineState {
    Wandering,
    Following,
    Attack,
    Move,
    Wait,
    Won
};

class Tank : public PhysicsObject {
public:
	Tank(int frac);
	void rotateTurret(float angle);
	void update(float deltaT);
	vec3 getTurretLookAt();
	vec3 getPosition();
    void MoveWithVelocity(vec3 velocity);
    void MoveToPosition(vec3 position);
    vec3 Velocity;
	
	float getHPPerc();
	float getXPPerc();
	void score();

	mat4 GetBottomM();
	mat4 GetTopM(mat4 bottomM);
	mat4 GetFlagM(mat4 bottomM);
    
    void SetEnemy(std::vector<Tank*>& enemyTanks);
    std::vector<Tank*>* GetEnemy() const;
    
    void setProjectile(Projectiles* projectiles);

	bool selected;
	int hp;
	int kills;
    int mFrac;
	int myProjectileID;
    
    void FollowAndAttack(Tank* tank);
	void onDeath();

private:
	float turretAngle;
    float Orientation;
    void SetOrientationFromVelocity(float deltaT);
    void SetTankOrientation(float deltaT);
    bool SetTurretOrientation(float deltaAngle, float angle);
    
    std::vector<Tank*>* enemyTanks;
    Tank* enemyTank;
    
    Steering* steer;
    vec3 toPosition;
    float maxVelocity;
    
    float yPosition;
    
    float minDistToFollow;
    float minDistToShoot;
    
    void updateStateMachine(float deltaT);
    StateMachineState currentState;

	void onCollision(COLLIDING_OBJECT other, void* collisionData);
    
    Projectiles* mProj;
    
    void StopTheTank();
};
