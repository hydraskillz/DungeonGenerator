/*
 * Author      : Matthew Johnson
 * Date        : 31/Jan/2014
 * Description :
 *   
 */
 
#pragma once

#include "glm/glm.hpp"
#include "btBulletDynamicsCommon.h"

class Entity;
class Player;
class Actor;

class btPairCachingGhostObject;
class Mesh;

struct PhysicsBody
{
	btRigidBody* mBody;
};

class PhysicsWorld
{
public:
	PhysicsWorld();
	~PhysicsWorld();

	enum
	{
		QuadNorm_PosX,
		QuadNorm_NegX,
		QuadNorm_PosY,
		QuadNorm_NegY,
		QuadNorm_PosZ,
		QuadNorm_NegZ,
	};

	void InitPhysics();
	void DestroyPhysics();
	void Step( float dt );
	void DebugDraw();

	btCollisionShape* GenerateTriangleMeshCollision( Mesh* mesh );
	//void LoadBulletFile( const char* bulletFile );
	void AddStaticPlane( float x, float y, float z, float c );
	void AddStaticQuad( float x, float y, float z, float w, float h, int n, float a=0.0f );
	void AddStairs( float x, float y, float z, int numSteps, float stepHeight, float stepDepth, int axis );
	void AddCharacter( Player* player );
	void AddActor( Actor* actor );
	void RemoveActor( Actor* actor );
	btRigidBody* AddStaticMesh( btTriangleMesh* mesh );
	btRigidBody* AddStaticMesh( btCollisionShape* collisionShape );
	btRigidBody* AddBox( const glm::vec3& halfExtents, bool ghost=false );
	btRigidBody* GenerateBoxCollision( Mesh* mesh, bool ghost=false );
	void RemoveBody( btRigidBody* body );

	void CheckGhostCollision( btPairCachingGhostObject* ghostObject );

	void Raycast( const glm::vec3& to, const glm::vec3& from );
	Entity* RaycastClosestNotMe( const glm::vec3& to, const glm::vec3& from, btCollisionObject* me, glm::vec3& out_hitNormal, glm::vec3& out_hitLocation );

	bool ShowDebugCollision;

private:
	void DrawOpenGL( btScalar* m, const btCollisionShape* shape, const btVector3& worldBoundsMin,const btVector3& worldBoundsMax );

	btDefaultCollisionConfiguration* mCollisionConfig;
	btCollisionDispatcher* mDispatcher;
	btDbvtBroadphase* mBroadphase;
	btConstraintSolver* mSolver;
	btDynamicsWorld* mDynamicsWorld;
	btAlignedObjectArray< btCollisionShape* > mCollisionShapes;
//	btAlignedObjectArray< btGhostObject* > mGhostObjects;
};