/*
 * Author      : Matthew Johnson
 * Date        : 2/Feb/2014
 * Description :
 *   
 */
 
#pragma once

#include "Actor.h"
#include "MD2Animation.h"
#include "XmlReader.h"

class EnemyFactory;
class EnemyTemplate;

class Enemy
	: public Actor
	, public MD2Animation::OnAnimationCompleteListener
{
	friend class Entity;
	friend class EnemyFactory;
public:
	static void LoadEnemyTempaltesFromXml( const XmlReader::XmlReaderIterator& xmlItr );
	static Enemy* CreateEnemy( const std::string& templateName );

	Enemy();
	virtual ~Enemy();

	void Initialize();
	void LoadAssets();
	void Update( float dt );
	void Draw( Window* window );

	void PrepareFire();
	void TakeDamage( int damage, Entity* instigator );
	void Kill();

	void OnAnimationComplete( const char* animName );

private:
	MD2Animation* mBodyAnim;
	MD2Animation* mHeadAnim;
	MD2Animation* mWeaponAnim;
	MD2Animation* mGibs;
	MD2Animation* mSpecialDeathAnim;
	bool mGibed;
	const EnemyTemplate* mTemplate;

	static EnemyFactory* GetFactory();
};