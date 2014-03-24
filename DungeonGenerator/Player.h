/*
 * Author      : Matthew Johnson
 * Date        : 31/Jan/2014
 * Description :
 *   
 */
 
#pragma once

#include "Actor.h"
#include <set>

class Camera;
class Weapon;
class Game;

class Player
	: public Actor
{
public:
	Player();
	virtual ~Player();

	void Update( float dt );
	void Draw( Window* window );

	void SetForward( int i ) { mForward = i; }
	void SetBackward( int i ) { mBackward = i; }
	void SetStrafeLeft( int i ) { mStrafeLeft = i; }
	void SetStrafeRight( int i ) { mStrafeRight = i; }
	void Rotate( float radians );
	void SetCamera( Camera* cam ) { mCamera = cam; }
	bool IsMoving() const;

	// Returns false if already has weapon
	bool GiveWeapon( Weapon* weapon );
	void NextWeapon();
	void PrevWeapon();
	void InitializeWeapons();

	void StartFire();
	void StopFire();

	void HideHUD( bool hide );

	// Calls Use() on the Entity we are looking at
	void UseAction();
	Entity* Raycast( glm::vec3& hitNormal, glm::vec3& hitLocation );

	bool OnCollision( Entity* other );

private:
	void EquipWeapon();

	int mForward, mBackward, mStrafeLeft, mStrafeRight;
	float mRotation;
	Camera* mCamera;
	std::vector< Weapon* > mWeapons;
	unsigned mActiveWeaponIndex;
	std::set< std::string > mWeaponsByName;
};