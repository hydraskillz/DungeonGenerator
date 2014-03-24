#include "PhysicsWorld.h"
#include "Util.h"
#include "Entity.h"
#include "Player.h"
#include "Actor.h"
#include "Mesh.h"
#include "Logger.h"

#include "BulletCollision/CollisionDispatch/btGhostObject.h"
#include "BulletCollision/CollisionShapes/btPolyhedralConvexShape.h"
#include "BulletCollision/CollisionShapes/btTriangleMeshShape.h"
#include "BulletCollision/CollisionShapes/btBoxShape.h"
#include "BulletCollision/CollisionShapes/btSphereShape.h"
#include "BulletCollision/CollisionShapes/btConeShape.h"
#include "BulletCollision/CollisionShapes/btCylinderShape.h"
#include "BulletCollision/CollisionShapes/btTetrahedronShape.h"
#include "BulletCollision/CollisionShapes/btCompoundShape.h"
#include "BulletCollision/CollisionShapes/btCapsuleShape.h"
#include "BulletCollision/CollisionShapes/btConvexTriangleMeshShape.h"
#include "BulletCollision/CollisionShapes/btUniformScalingShape.h"
#include "BulletCollision/CollisionShapes/btStaticPlaneShape.h"
#include "BulletCollision/CollisionShapes/btMultiSphereShape.h"
#include "BulletCollision/CollisionShapes/btConvexPolyhedron.h"
#include "BulletCollision/CollisionShapes/btShapeHull.h"
#include "BulletCollision/Gimpact/btGImpactShape.h"
#include "BulletCollision/NarrowPhaseCollision/btRaycastCallback.h"

#include <glew.h>

// True if changed collision properties
bool callbackFn( btManifoldPoint& cp, const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0, const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1 )
{
	Entity* e1 = (Entity*) ( colObj0Wrap->getCollisionObject()->getUserPointer() );
	Entity* e2 = (Entity*) ( colObj1Wrap->getCollisionObject()->getUserPointer() );
	if ( e1 && e2 )
	{
		if ( !e1->OnCollision( e2 ) )
			e2->OnCollision( e1 );
	}

	return false;
}

class ClosestNotMeRayResultCallback : public btCollisionWorld::ClosestRayResultCallback
{
private:
	btCollisionObject * mMe;

public:
	ClosestNotMeRayResultCallback( const btVector3& from, const btVector3& to, btCollisionObject * me) :
	  btCollisionWorld::ClosestRayResultCallback(from, to),
		  mMe(me)
	  {}

	  btScalar addSingleResult(btCollisionWorld::LocalRayResult & rayResult, bool normalInWorldSpace)
	  {
		  if (rayResult.m_collisionObject == mMe)
			  return 1.0;

		  return btCollisionWorld::ClosestRayResultCallback::addSingleResult(rayResult, normalInWorldSpace);
	  }
};

inline void glDrawVector(const btVector3& v) { glVertex3d(v[0], v[1], v[2]); }

void drawSphere (const btVector3& p, btScalar radius, const btVector3& color)
{
	glColor4f (color.getX(), color.getY(), color.getZ(), btScalar(1.0f));
	glPushMatrix ();
	glTranslatef (p.getX(), p.getY(), p.getZ());

	int lats = 5;
	int longs = 5;

	int i, j;
	for(i = 0; i <= lats; i++) {
		btScalar lat0 = SIMD_PI * (-btScalar(0.5) + (btScalar) (i - 1) / lats);
		btScalar z0  = radius*sin(lat0);
		btScalar zr0 =  radius*cos(lat0);

		btScalar lat1 = SIMD_PI * (-btScalar(0.5) + (btScalar) i / lats);
		btScalar z1 = radius*sin(lat1);
		btScalar zr1 = radius*cos(lat1);

		glBegin(GL_QUAD_STRIP);
		for(j = 0; j <= longs; j++) {
			btScalar lng = 2 * SIMD_PI * (btScalar) (j - 1) / longs;
			btScalar x = cos(lng);
			btScalar y = sin(lng);

			glNormal3f(x * zr0, y * zr0, z0);
			glVertex3f(x * zr0, y * zr0, z0);
			glNormal3f(x * zr1, y * zr1, z1);
			glVertex3f(x * zr1, y * zr1, z1);
		}
		glEnd();
	}

	glPopMatrix();
}

void	drawLine(const btVector3& from,const btVector3& to,const btVector3& fromColor, const btVector3& toColor)
{
	glBegin(GL_LINES);
	glColor3f(fromColor.getX(), fromColor.getY(), fromColor.getZ());
	glVertex3d(from.getX(), from.getY(), from.getZ());
	glColor3f(toColor.getX(), toColor.getY(), toColor.getZ());
	glVertex3d(to.getX(), to.getY(), to.getZ());
	glEnd();
}

void	drawLine(const btVector3& from,const btVector3& to,const btVector3& color)
{
	drawLine(from,to,color,color);
}

void PhysicsWorld::CheckGhostCollision( btPairCachingGhostObject* ghostObject )
{
	btManifoldArray manifoldArray;
	btBroadphasePairArray& pairArray = ghostObject->getOverlappingPairCache()->getOverlappingPairArray();
	int numPairs = pairArray.size();

	for ( int i = 0; i < numPairs; i++ )
	{
		manifoldArray.clear();

		const btBroadphasePair& pair = pairArray[i];

		//unless we manually perform collision detection on this pair, the contacts are in the dynamics world paircache:
		btBroadphasePair* collisionPair = mDynamicsWorld->getPairCache()->findPair( pair.m_pProxy0, pair.m_pProxy1 );
		if ( !collisionPair )
			continue;
		if ( collisionPair->m_algorithm )
			collisionPair->m_algorithm->getAllContactManifolds( manifoldArray );

		for ( int j = 0; j < manifoldArray.size(); j++ )
		{
			btPersistentManifold* manifold = manifoldArray[j];
			if ( manifold->getNumContacts() )
			{
				int id = reinterpret_cast< int >( ( manifold->getBody0() != ghostObject ) ?
					static_cast< const btCollisionObject* >( manifold->getBody0() )->getUserPointer():
					static_cast< const btCollisionObject* >( manifold->getBody1() )->getUserPointer() );
				DebugPrintf( "\tcontact (%d) : %s\n", manifold->getNumContacts(), (id==1)?"box":"actor" );
			}
		}
	}
}

//---------------------------------------
PhysicsWorld::PhysicsWorld()
	: ShowDebugCollision( false )
	, mCollisionConfig( 0 )
	, mDispatcher( 0 )
	, mBroadphase( 0 )
	, mSolver( 0 )
	, mDynamicsWorld( 0 )
{}
//---------------------------------------
PhysicsWorld::~PhysicsWorld()
{}
//---------------------------------------
void PhysicsWorld::InitPhysics()
{
	DebugPrintf( "Initializing PhysicsWorld...\n" );

	gContactAddedCallback = callbackFn;

	///collision configuration contains default setup for memory, collision setup
	mCollisionConfig = new btDefaultCollisionConfiguration();
	//m_collisionConfiguration->setConvexConvexMultipointIterations();

	///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
	mDispatcher = new	btCollisionDispatcher(mCollisionConfig);

	mBroadphase = new btDbvtBroadphase();
	mBroadphase->getOverlappingPairCache()->setInternalGhostPairCallback( new btGhostPairCallback() );

	///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
	btSequentialImpulseConstraintSolver* sol = new btSequentialImpulseConstraintSolver;
	mSolver = sol;

	mDynamicsWorld = new btDiscreteDynamicsWorld(mDispatcher,mBroadphase,mSolver,mCollisionConfig);

	mDynamicsWorld->setGravity(btVector3(0,-10,0));
}
//---------------------------------------
void PhysicsWorld::DestroyPhysics()
{
	DebugPrintf( "Destroying PhysicsWorld...\n" );

	if ( mDynamicsWorld )
	{
		for (int i=mDynamicsWorld->getNumCollisionObjects()-1; i>=0 ;i--)
		{
			btCollisionObject* obj = mDynamicsWorld->getCollisionObjectArray()[i];
			btRigidBody* body = btRigidBody::upcast(obj);
			if (body && body->getMotionState())
			{
				delete body->getMotionState();
			}
			mDynamicsWorld->removeCollisionObject( obj );
			delete obj;
		}
	}
	//delete collision shapes
	for (int j=0;j<mCollisionShapes.size();j++)
	{
		btCollisionShape* shape = mCollisionShapes[j];
		delete shape;
	}
	mCollisionShapes.clear();
		
	SafeDelete( mDynamicsWorld );
	SafeDelete( mSolver );
	SafeDelete( mBroadphase );
	SafeDelete( mDispatcher );
	SafeDelete( mCollisionConfig );
}
//---------------------------------------
void PhysicsWorld::Step( float dt )
{
	mDynamicsWorld->stepSimulation( dt );
}
//---------------------------------------
void PhysicsWorld::DebugDraw()
{
	if ( !ShowDebugCollision )
		return;

	btScalar	m[16];
	btMatrix3x3	rot;rot.setIdentity();
	const int	numObjects=mDynamicsWorld->getNumCollisionObjects();
	for(int i=0;i<numObjects;i++)
	{
		btCollisionObject*	colObj=mDynamicsWorld->getCollisionObjectArray()[i];
		btRigidBody*		body=btRigidBody::upcast(colObj);
		if(body&&body->getMotionState())
		{
			btDefaultMotionState* myMotionState = (btDefaultMotionState*)body->getMotionState();
			myMotionState->m_graphicsWorldTrans.getOpenGLMatrix(m);
			rot=myMotionState->m_graphicsWorldTrans.getBasis();
		}
		else
		{
			colObj->getWorldTransform().getOpenGLMatrix(m);
			rot=colObj->getWorldTransform().getBasis();
		}

		btVector3 aabbMin(0,0,0),aabbMax(0,0,0);
		//m_dynamicsWorld->getBroadphase()->getBroadphaseAabb(aabbMin,aabbMax);

		aabbMin-=btVector3(BT_LARGE_FLOAT,BT_LARGE_FLOAT,BT_LARGE_FLOAT);
		aabbMax+=btVector3(BT_LARGE_FLOAT,BT_LARGE_FLOAT,BT_LARGE_FLOAT);
		DrawOpenGL(m,colObj->getCollisionShape(),aabbMin,aabbMax);
	}
}
//---------------------------------------
btRigidBody* PhysicsWorld::AddStaticMesh( btTriangleMesh* mesh )
{
	btBvhTriangleMeshShape* trimesh = new btBvhTriangleMeshShape(mesh,true,true);
	trimesh->buildOptimizedBvh();
	//btGImpactMeshShape * trimesh = new btGImpactMeshShape(mesh);
	//trimesh->updateBound();

	mCollisionShapes.push_back( trimesh );

	btTransform startTransform;
	startTransform.setIdentity();

	btCollisionShape* colShape = trimesh;
	btRigidBody::btRigidBodyConstructionInfo rbInfo( 0, 0, colShape, btVector3( 0,0,0 ) );
	rbInfo.m_startWorldTransform = startTransform;
	btRigidBody* body = new btRigidBody(rbInfo);
	
	mDynamicsWorld->addRigidBody(body);
	return body;
}
//---------------------------------------
btRigidBody* PhysicsWorld::AddBox( const glm::vec3& halfExtents, bool ghost )
{
	btCollisionShape* box = new btBoxShape( btVector3( halfExtents.x, halfExtents.y, halfExtents.z ) );

	mCollisionShapes.push_back( box );
	
	btRigidBody::btRigidBodyConstructionInfo rbInfo( 0.0f, 0, box, btVector3( 0,0,0 ) );
	btRigidBody* body = new btRigidBody(rbInfo);

	if ( ghost )
		body->setCollisionFlags( body->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE );

	mDynamicsWorld->addRigidBody( body);

	return body;
}
//---------------------------------------
btRigidBody* PhysicsWorld::GenerateBoxCollision( Mesh* mesh, bool ghost )
{
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	for ( unsigned i = 0; i < mesh->GetNumEntries(); ++i )
	{
		const Mesh::MeshEntry& e = mesh->GetMeshEntry( i );
		for ( unsigned j = 0; j < e.numIndices; ++j )
		{
			const Mesh::Vertex& v = e.mVerts[j];
			const float px = fabs( v.position.x );
			const float py = fabs( v.position.y );
			const float pz = fabs( v.position.z );
			
			if ( px > x )
				x = px;
			if ( py > y )
				y = py;
			if ( pz > z )
				z = pz;
		}
	}

	return AddBox( glm::vec3( x, y / 2.0f, z ), ghost );
}
//---------------------------------------
void PhysicsWorld::RemoveBody( btRigidBody* body )
{
	if ( body->getMotionState() )
	{
		delete body->getMotionState();
	}
	mDynamicsWorld->removeRigidBody( body );
	delete body;
}
//---------------------------------------
void PhysicsWorld::AddStaticPlane( float x, float y, float z, float c )
{
	btCollisionShape* plane = new btStaticPlaneShape( btVector3( x, y, z ), c );
	mCollisionShapes.push_back( plane );
	btTransform groundTransform;
	groundTransform.setIdentity();
	//groundTransform.setOrigin( btVector3( 0, -c, 0 ) );

	//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
	btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(0,myMotionState,plane,btVector3(0,0,0));
	btRigidBody* body = new btRigidBody(rbInfo);

	//add the body to the dynamics world
	mDynamicsWorld->addRigidBody(body);
}
//---------------------------------------
void PhysicsWorld::AddStaticQuad( float x, float y, float z, float w, float h, int n, float a )
{
	float hw = w / 2.0f;
	float hh = h / 2.0f;
	btVector3 quad[4];
	
	if ( n == QuadNorm_PosX )
	{
		quad[3] = btVector3( + hw, + h, + hh );
		quad[2] = btVector3( + hw, + h, - hh );
		quad[1] = btVector3( + hw,   0, - hh );
		quad[0] = btVector3( + hw,   0, + hh );
	}
	else if ( n == QuadNorm_NegX )
	{
		quad[3] = btVector3( - hw, + h, - hh );
		quad[2] = btVector3( - hw, + h, + hh );
		quad[1] = btVector3( - hw,   0, + hh );
		quad[0] = btVector3( - hw,   0, - hh );
	}
	else if ( n == QuadNorm_PosY )
	{
		quad[3] = btVector3( - hw, 0, - hh );
		quad[2] = btVector3( + hw, 0, - hh );
		quad[1] = btVector3( + hw, 0, + hh );
		quad[0] = btVector3( - hw, 0, + hh );
	}
	else if ( n == QuadNorm_NegY )
	{
		quad[3] = btVector3( - hw, + h, + hh );
		quad[2] = btVector3( + hw, + h, + hh );
		quad[1] = btVector3( + hw, + h, - hh );
		quad[0] = btVector3( - hw, + h, - hh );
	}
	else if ( n == QuadNorm_PosZ )
	{
		quad[3] = btVector3( - hw, + h, + hh );
		quad[2] = btVector3( + hw, + h, + hh );
		quad[1] = btVector3( + hw,   0, + hh );
		quad[0] = btVector3( - hw,   0, + hh );
	}
	else if ( n == QuadNorm_NegZ )
	{
		quad[3] = btVector3( + hw, + h, - hh );
		quad[2] = btVector3( - hw, + h, - hh );
		quad[1] = btVector3( - hw,   0, - hh );
		quad[0] = btVector3( + hw,   0, - hh );
	}

	btTriangleMesh* mesh = new btTriangleMesh();
	mesh->addTriangle(quad[0],quad[1],quad[2],true);
	mesh->addTriangle(quad[0],quad[2],quad[3],true);
	
	btBvhTriangleMeshShape* trimesh = new btBvhTriangleMeshShape(mesh,true,true);
//	trimesh->buildOptimizedBvh();
//	btGImpactMeshShape * trimesh = new btGImpactMeshShape(mesh);
//	trimesh->updateBound();

	mCollisionShapes.push_back( trimesh );

	/// Create Static Object
	btTransform startTransform;
	startTransform.setIdentity();
	startTransform.setOrigin( btVector3( x, y, z ) );
	startTransform.setRotation( btQuaternion( btVector3( 0, 1, 0 ), a ) );

	btCollisionShape* colShape = trimesh;
	btRigidBody::btRigidBodyConstructionInfo rbInfo( 0, 0, colShape, btVector3( 0,0,0 ) );
	rbInfo.m_startWorldTransform = startTransform;
	btRigidBody* body = new btRigidBody(rbInfo);

	body->setUserPointer( &Entity::WORLD );
	mDynamicsWorld->addRigidBody(body);
}
//---------------------------------------
btCollisionShape* PhysicsWorld::GenerateTriangleMeshCollision( Mesh* mesh )
{
	if ( mesh->GetNumEntries() == 0 )
		return 0;

	btTriangleMesh* triMesh = new btTriangleMesh();
	for ( unsigned i = 0; i < mesh->GetNumEntries(); ++i )
	{
		const Mesh::MeshEntry& e = mesh->GetMeshEntry( i );
		for ( unsigned j = 0; j < e.numIndices; j+=3 )
		{
			const Mesh::Vertex& v0 = e.mVerts[j];
			const Mesh::Vertex& v1 = e.mVerts[j+1];
			const Mesh::Vertex& v2 = e.mVerts[j+2];

			btVector3 p0( v0.position.x, v0.position.y, v0.position.z );
			btVector3 p1( v1.position.x, v1.position.y, v1.position.z );
			btVector3 p2( v2.position.x, v2.position.y, v2.position.z );

			triMesh->addTriangle( p0, p1, p2 );
		}
	}

	btBvhTriangleMeshShape* bvhMesh = new btBvhTriangleMeshShape(triMesh,true,true);
	//	trimesh->buildOptimizedBvh();
	//	btGImpactMeshShape * trimesh = new btGImpactMeshShape(mesh);
	//	trimesh->updateBound();

	mCollisionShapes.push_back( bvhMesh );

	mesh->SetCollisionShape( bvhMesh );

	return bvhMesh;
}
//---------------------------------------
btRigidBody* PhysicsWorld::AddStaticMesh( btCollisionShape* collisionShape )
{
	// Create Static Object
	btTransform startTransform;
	startTransform.setIdentity();

	btRigidBody::btRigidBodyConstructionInfo rbInfo( 0, 0, collisionShape, btVector3( 0,0,0 ) );
	rbInfo.m_startWorldTransform = startTransform;
	btRigidBody* body = new btRigidBody(rbInfo);

	body->setUserPointer( &Entity::WORLD );
	mDynamicsWorld->addRigidBody(body);

	return body;
}
//---------------------------------------
/*void PhysicsWorld::LoadBulletFile( const char* bulletFile )
{
	btBulletWorldImporter importer( mDynamicsWorld );
	importer.loadFile( bulletFile );
}*/
//---------------------------------------
void PhysicsWorld::AddStairs( float x, float y, float z, int nSteps, float stepHeight, float stepDepth, int axis )
{
	btTriangleMesh* mesh = new btTriangleMesh();
	btVector3 quad[4];
	for ( int i = 0; i < nSteps; ++i )
	{
		// h
		quad[3] = btVector3( stepDepth * i,  stepHeight * i, 0 );
		quad[2] = btVector3( stepDepth + stepDepth * i, stepHeight * i, 0 );
		quad[1] = btVector3( stepDepth + stepDepth * i, stepHeight * i, 1 );
		quad[0] = btVector3( stepDepth * i, stepHeight * i, 1 );

		mesh->addTriangle(quad[0],quad[1],quad[2],true);
		mesh->addTriangle(quad[0],quad[2],quad[3],true);

		// v
		quad[3] = btVector3( stepDepth * (i+1), stepHeight * (i+1), 0 );
		quad[2] = btVector3( stepDepth * (i+1), stepHeight * (i+1), 1 );
		quad[1] = btVector3( stepDepth + stepDepth * i, stepHeight * i, 1 );
		quad[0] = btVector3( stepDepth + stepDepth * i, stepHeight * i, 0 );

		mesh->addTriangle(quad[0],quad[1],quad[2],true);
		mesh->addTriangle(quad[0],quad[2],quad[3],true);
	}

	btBvhTriangleMeshShape* trimesh = new btBvhTriangleMeshShape(mesh,true,true);
	trimesh->buildOptimizedBvh();
	mCollisionShapes.push_back( trimesh );
	btTriangleIndexVertexArray a;
	btIndexedMesh m;
	

	/// Create Static Object
	btTransform startTransform;
	startTransform.setIdentity();
	startTransform.setOrigin( btVector3( x, y, z ) );
	if ( axis == QuadNorm_PosZ )
		startTransform.setRotation( btQuaternion( btVector3( 0, 1, 0 ), glm::radians( -90.0f ) ) );
	else if ( axis == QuadNorm_NegZ )
		startTransform.setRotation( btQuaternion( btVector3( 0, 1, 0 ), glm::radians( 90.0f ) ) );
	else if ( axis == QuadNorm_PosX )
		startTransform.setRotation( btQuaternion( btVector3( 0, 1, 0 ), glm::radians( 180.0f ) ) );

	btCollisionShape* colShape = trimesh;
	btRigidBody::btRigidBodyConstructionInfo rbInfo( 0, 0, colShape, btVector3( 0,0,0 ) );
	rbInfo.m_startWorldTransform = startTransform;
	btRigidBody* body = new btRigidBody(rbInfo);

	body->setUserPointer( &Entity::WORLD );
	mDynamicsWorld->addRigidBody(body);
}
//---------------------------------------
void PhysicsWorld::AddCharacter( Player* player )
{
	player->InitPhysics( this );

	mDynamicsWorld->addCollisionObject( player->GetGhostObject(),btBroadphaseProxy::CharacterFilter, btBroadphaseProxy::StaticFilter|btBroadphaseProxy::DefaultFilter);
	mDynamicsWorld->addAction( player->GetController() );
	
	mDynamicsWorld->getBroadphase()->getOverlappingPairCache()->cleanProxyFromPairs(player->GetGhostObject()->getBroadphaseHandle(),mDynamicsWorld->getDispatcher());
	player->GetController()->reset( mDynamicsWorld );

	mCollisionShapes.push_back( player->GetGhostObject()->getCollisionShape() );
}
//---------------------------------------
void PhysicsWorld::AddActor( Actor* actor )
{
	// Add actor and controller to world
	mDynamicsWorld->addCollisionObject( actor->GetGhostObject(),btBroadphaseProxy::CharacterFilter, btBroadphaseProxy::StaticFilter|btBroadphaseProxy::DefaultFilter);
	mDynamicsWorld->addAction( actor->GetController() );

	// Clean controller in case this actor is being re-added
	mDynamicsWorld->getBroadphase()->getOverlappingPairCache()->cleanProxyFromPairs( actor->GetGhostObject()->getBroadphaseHandle(), mDynamicsWorld->getDispatcher() );
	actor->GetController()->reset( mDynamicsWorld );

	// Add the actors collision to the world
	mCollisionShapes.push_back( actor->GetGhostObject()->getCollisionShape() );
}
//---------------------------------------
void PhysicsWorld::RemoveActor( Actor* actor )
{
	mDynamicsWorld->removeCollisionObject( actor->GetGhostObject() );
	mDynamicsWorld->removeAction( actor->GetController() );
}
//---------------------------------------
void PhysicsWorld::Raycast( const glm::vec3& _to, const glm::vec3& _from )
{
	btVector3 to( _to.x, _to.y, _to.z );
	btVector3 from( _from.x, _from.y, _from.z );

	drawLine(from,to,btVector3(0,0,1));

	btCollisionWorld::ClosestRayResultCallback	closestResults(from,to);
	closestResults.m_flags |= btTriangleRaycastCallback::kF_FilterBackfaces;
	mDynamicsWorld->rayTest(from,to,closestResults);
	
	btVector3 red(1,0,0);
	btVector3 blue(0,0,1);

	if (closestResults.hasHit())
	{
		btVector3 p = from.lerp(to,closestResults.m_closestHitFraction);
		drawSphere(p,0.1f,blue);
		drawLine(p,p+closestResults.m_hitNormalWorld,blue);

	}
}
//---------------------------------------
Entity* PhysicsWorld::RaycastClosestNotMe( const glm::vec3& _to, const glm::vec3& _from, btCollisionObject* me, glm::vec3& out_hitNormal, glm::vec3& out_hitLocation )
{
	btVector3 to( _to.x, _to.y, _to.z );
	btVector3 from( _from.x, _from.y, _from.z );

	//drawLine(from,to,btVector3(0,0,1));

	
	ClosestNotMeRayResultCallback closestResults( from, to, me );
	closestResults.m_flags |= btTriangleRaycastCallback::kF_FilterBackfaces;
	mDynamicsWorld->rayTest(from,to,closestResults);

	btVector3 red(1,0,0);
	btVector3 blue(0,0,1);

	if ( ShowDebugCollision )
		drawLine(from,to,btVector4(0,0,1,1),btVector4(1,0,0,1));

	bool hit = closestResults.hasHit();
	Entity* hitEntity = 0;
	
	if ( hit )
	{
		if ( closestResults.m_collisionObject )
			hitEntity = (Entity*) closestResults.m_collisionObject->getUserPointer();

		btVector3 p = from.lerp(to,closestResults.m_closestHitFraction);
		if ( ShowDebugCollision )
		{
			drawSphere(p,0.05f,blue);
			drawLine(p,p+closestResults.m_hitNormalWorld,blue);
		}

		out_hitNormal   = btVect3ToglmVec3( closestResults.m_hitNormalWorld );
		out_hitLocation = btVect3ToglmVec3( p );
	}
	return hitEntity;
}
//---------------------------------------
class GlDrawcallback : public btTriangleCallback
{

public:

	bool	m_wireframe;

	GlDrawcallback()
		:m_wireframe(false)
	{
	}

	virtual void processTriangle(btVector3* triangle,int partId, int triangleIndex)
	{

		(void)triangleIndex;
		(void)partId;


		if (m_wireframe)
		{
			glBegin(GL_LINES);
			glColor3f(1, 0, 0);
			glVertex3d(triangle[0].getX(), triangle[0].getY(), triangle[0].getZ());
			glVertex3d(triangle[1].getX(), triangle[1].getY(), triangle[1].getZ());
			glColor3f(0, 1, 0);
			glVertex3d(triangle[2].getX(), triangle[2].getY(), triangle[2].getZ());
			glVertex3d(triangle[1].getX(), triangle[1].getY(), triangle[1].getZ());
			glColor3f(0, 0, 1);
			glVertex3d(triangle[2].getX(), triangle[2].getY(), triangle[2].getZ());
			glVertex3d(triangle[0].getX(), triangle[0].getY(), triangle[0].getZ());
			glEnd();
		} else
		{
			glBegin(GL_TRIANGLES);
			//glColor3f(1, 1, 1);


			glVertex3d(triangle[0].getX(), triangle[0].getY(), triangle[0].getZ());
			glVertex3d(triangle[1].getX(), triangle[1].getY(), triangle[1].getZ());
			glVertex3d(triangle[2].getX(), triangle[2].getY(), triangle[2].getZ());

			glVertex3d(triangle[2].getX(), triangle[2].getY(), triangle[2].getZ());
			glVertex3d(triangle[1].getX(), triangle[1].getY(), triangle[1].getZ());
			glVertex3d(triangle[0].getX(), triangle[0].getY(), triangle[0].getZ());
			glEnd();
		}
	}
};
struct ShapeCache
{
	struct Edge { btVector3 n[2];int v[2]; };
	ShapeCache(btConvexShape* s) : m_shapehull(s) {}
	btShapeHull					m_shapehull;
	btAlignedObjectArray<Edge>	m_edges;
};
btAlignedObjectArray<ShapeCache*>	m_shapecaches;
ShapeCache*		cache(btConvexShape* shape)
{
	ShapeCache*		sc=(ShapeCache*)shape->getUserPointer();
	if(!sc)
	{
		sc=new(btAlignedAlloc(sizeof(ShapeCache),16)) ShapeCache(shape);
		sc->m_shapehull.buildHull(shape->getMargin());
		m_shapecaches.push_back(sc);
		shape->setUserPointer(sc);
		/* Build edges	*/ 
		const int			ni=sc->m_shapehull.numIndices();
		const int			nv=sc->m_shapehull.numVertices();
		const unsigned int*	pi=sc->m_shapehull.getIndexPointer();
		const btVector3*	pv=sc->m_shapehull.getVertexPointer();
		btAlignedObjectArray<ShapeCache::Edge*>	edges;
		sc->m_edges.reserve(ni);
		edges.resize(nv*nv,0);
		for(int i=0;i<ni;i+=3)
		{
			const unsigned int* ti=pi+i;
			const btVector3		nrm=btCross(pv[ti[1]]-pv[ti[0]],pv[ti[2]]-pv[ti[0]]).normalized();
			for(int j=2,k=0;k<3;j=k++)
			{
				const unsigned int	a=ti[j];
				const unsigned int	b=ti[k];
				ShapeCache::Edge*&	e=edges[btMin(a,b)*nv+btMax(a,b)];
				if(!e)
				{
					sc->m_edges.push_back(ShapeCache::Edge());
					e=&sc->m_edges[sc->m_edges.size()-1];
					e->n[0]=nrm;e->n[1]=-nrm;
					e->v[0]=a;e->v[1]=b;
				}
				else
				{
					e->n[1]=nrm;
				}
			}
		}
	}
	return(sc);
}
void PhysicsWorld::DrawOpenGL( btScalar* m, const btCollisionShape* shape, const btVector3& worldBoundsMin,const btVector3& worldBoundsMax )
{
	glFrontFace( GL_CCW );
	glColor3f( 1, 1, 1 );
	if (shape->getShapeType() == CUSTOM_CONVEX_SHAPE_TYPE)
	{
		btVector3 org(m[12], m[13], m[14]);
		btVector3 dx(m[0], m[1], m[2]);
		btVector3 dy(m[4], m[5], m[6]);
//		btVector3 dz(m[8], m[9], m[10]);
		const btBoxShape* boxShape = static_cast<const btBoxShape*>(shape);
		btVector3 halfExtent = boxShape->getHalfExtentsWithMargin();
		dx *= halfExtent[0];
		dy *= halfExtent[1];
//		dz *= halfExtent[2];
		glColor3f(1,1,1);
		glDisable(GL_LIGHTING);
		glLineWidth(2);

		glBegin(GL_LINE_LOOP);
		glDrawVector(org - dx - dy);
		glDrawVector(org - dx + dy);
		glDrawVector(org + dx + dy);
		glDrawVector(org + dx - dy);
		glEnd();
		return;
	} 
	else if((shape->getShapeType() == BOX_SHAPE_PROXYTYPE) )
	{
		btVector3 org(m[12], m[13], m[14]);
		btVector3 dx(m[0], m[1], m[2]);
		btVector3 dy(m[4], m[5], m[6]);
		btVector3 dz(m[8], m[9], m[10]);
		const btBoxShape* boxShape = static_cast<const btBoxShape*>(shape);
		btVector3 halfExtent = boxShape->getHalfExtentsWithMargin();
		dx *= halfExtent[0];
		dy *= halfExtent[1];
		dz *= halfExtent[2];
		glBegin(GL_LINE_LOOP);
		glDrawVector(org - dx - dy - dz);
		glDrawVector(org + dx - dy - dz);
		glDrawVector(org + dx + dy - dz);
		glDrawVector(org - dx + dy - dz);
		glDrawVector(org - dx + dy + dz);
		glDrawVector(org + dx + dy + dz);
		glDrawVector(org + dx - dy + dz);
		glDrawVector(org - dx - dy + dz);
		glEnd();
		glBegin(GL_LINES);
		glDrawVector(org + dx - dy - dz);
		glDrawVector(org + dx - dy + dz);
		glDrawVector(org + dx + dy - dz);
		glDrawVector(org + dx + dy + dz);
		glDrawVector(org - dx - dy - dz);
		glDrawVector(org - dx + dy - dz);
		glDrawVector(org - dx - dy + dz);
		glDrawVector(org - dx + dy + dz);
		glEnd();
		return;
	}

	glPushMatrix(); 
	glMultMatrixf(m);

	if (shape->isConvex())
	{
		const btConvexPolyhedron* poly = shape->isPolyhedral() ? ((btPolyhedralConvexShape*) shape)->getConvexPolyhedron() : 0;
		if (poly)
		{
			int i;
			glBegin (GL_TRIANGLES);
			for (i=0;i<poly->m_faces.size();i++)
			{
				btVector3 centroid(0,0,0);
				int numVerts = poly->m_faces[i].m_indices.size();
				if (numVerts>2)
				{
					btVector3 v1 = poly->m_vertices[poly->m_faces[i].m_indices[0]];
					for (int v=0;v<poly->m_faces[i].m_indices.size()-2;v++)
					{

						btVector3 v2 = poly->m_vertices[poly->m_faces[i].m_indices[v+1]];
						btVector3 v3 = poly->m_vertices[poly->m_faces[i].m_indices[v+2]];
						btVector3 normal = (v3-v1).cross(v2-v1);
						normal.normalize ();
						glNormal3f(normal.getX(),normal.getY(),normal.getZ());
						glVertex3f (v1.x(), v1.y(), v1.z());
						glVertex3f (v2.x(), v2.y(), v2.z());
						glVertex3f (v3.x(), v3.y(), v3.z());
					}
				}
			}
			glEnd ();
		} else
		{
			ShapeCache*	sc=cache((btConvexShape*)shape);
			//glutSolidCube(1.0);
			btShapeHull* hull = &sc->m_shapehull/*(btShapeHull*)shape->getUserPointer()*/;

			if (hull->numTriangles () > 0)
			{
				int index = 0;
				const unsigned int* idx = hull->getIndexPointer();
				const btVector3* vtx = hull->getVertexPointer();

				glBegin (GL_TRIANGLES);

				for (int i = 0; i < hull->numTriangles (); i++)
				{
					int i1 = index++;
					int i2 = index++;
					int i3 = index++;
					btAssert(i1 < hull->numIndices () &&
						i2 < hull->numIndices () &&
						i3 < hull->numIndices ());

					int index1 = idx[i1];
					int index2 = idx[i2];
					int index3 = idx[i3];
					btAssert(index1 < hull->numVertices () &&
						index2 < hull->numVertices () &&
						index3 < hull->numVertices ());

					btVector3 v1 = vtx[index1];
					btVector3 v2 = vtx[index2];
					btVector3 v3 = vtx[index3];
					btVector3 normal = (v3-v1).cross(v2-v1);
					normal.normalize ();
					glNormal3f(normal.getX(),normal.getY(),normal.getZ());
					glVertex3f (v1.x(), v1.y(), v1.z());
					glVertex3f (v2.x(), v2.y(), v2.z());
					glVertex3f (v3.x(), v3.y(), v3.z());

				}
				glEnd ();

			}
		}
	}
	if (shape->isConcave() && !shape->isInfinite())
	{
		btConcaveShape* concaveMesh = (btConcaveShape*) shape;

		GlDrawcallback drawCallback;
		drawCallback.m_wireframe = 1;

		concaveMesh->processAllTriangles(&drawCallback,worldBoundsMin,worldBoundsMax);

	}
	glPopMatrix();
	glFrontFace( GL_CW );
}
//---------------------------------------