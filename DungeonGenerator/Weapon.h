/*
 * Author      : Matthew Johnson
 * Date        : 31/Jan/2014
 * Description :
 *   
 */
 
#pragma once

#include "Entity.h"
#include "glm/glm.hpp"
#include "Texture.h"
#include <vector>

class Window;
class Player;
class Mesh;
class WeaponFactory;
class WeaponTemplate;

class Weapon
	: public Entity
{
	friend class Entity;
	friend class WeaponFactory;
public:
	static void LoadWeaponTempaltesFromXml( const XmlReader::XmlReaderIterator& xmlItr );
	static Weapon* CreateWeapon( const std::string& templateName );

	Weapon();
	~Weapon();

	void LoadAssets();
	void Update( float dt );
	void Draw( Window* window );
	void StartFire();
	void StopFire();
	void DoFire();
	void SetOwner( Player* owner ) { mOwner = owner; }

private:
	const WeaponTemplate* mTempalte;
	float mFireTimer;
	bool mIsFiring;
	bool mStopFire;
	Player* mOwner;
	Mesh* mWeaponMesh;
	/*struct Decal
	{
		glm::vec3 pos;
		glm::vec3 norm;
		glm::mat4 m;
		float life;
	};
	std::vector< Decal > mDecals;*/

private:
	static WeaponFactory* GetFactory();
};