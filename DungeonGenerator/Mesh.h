/*
 * Author      : Matthew Johnson
 * Date        : 5/Feb/2014
 * Description :
 *   
 */
 
#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <map>
#include <assimp.hpp>

class Texture2D;
struct aiScene;
struct aiMesh;
class btCollisionShape;

class Mesh
{
public:
	struct Vertex
	{
		Vertex() {}
		Vertex( const glm::vec3& pos, const glm::vec2& tex, const glm::vec3& norm )
			: position( pos )
			, texture( tex )
			, normal( norm )
		{}

		glm::vec3 position;
		glm::vec2 texture;
		glm::vec3 normal;
		glm::vec4 boneWeights;
		glm::ivec4 boneIds;
	};

	struct MeshEntry
	{
		MeshEntry();
		~MeshEntry();

		void Init( const std::vector< Vertex >& verts,
			const std::vector< unsigned >& indices );

		unsigned idVB;
		unsigned idIB;
		unsigned numIndices;
		unsigned materialIndex;

		std::vector< Vertex > mVerts;
		std::vector< unsigned > mIndices;
	};
private:
	Mesh();
	~Mesh();

	bool Load( const char* filename );

public:
	void Draw();

	// Creates and registers a new Mesh
	static Mesh* CreateMesh( const char* filename );
	static Mesh* CreateMesh( const std::string& filename );
	static Mesh* CreateMesh( const char* name, const std::vector< Vertex >& verts, const std::vector< unsigned >& indices );

	static void DestroyAllMeshes();

	// Use this to unload a single mesh
	// You should make sure no one else is using it
	static void DestroyMesh( Mesh*& mesh );

	// Called when the physics world is destroyed
	// This will set all the collisionShapes to 0
	// so you will need to re-add the collision shape for the mesh
	// in the next initialization call in your entity
	static void InvalidatePhysics();

	inline unsigned GetNumEntries() const { return mEntries.size(); }
	inline const MeshEntry& GetMeshEntry( unsigned i ) { return mEntries[i]; }
	inline void SetCollisionShape( btCollisionShape* shape ) { mCollisionShape = shape; }
	inline btCollisionShape* GetCollisionShape() const { return mCollisionShape; }
	inline const char* GetName() const { return mName.c_str(); }

private:
	static const unsigned INVALID_ID = 0xFFFFFFFF;

	static void RegisterMesh( const char* filename, Mesh* mesh );
	static void UnregisterMesh( const char* filename );

	bool InitScene( const aiScene* scene, const char* filename );
	bool InitMaterials( const aiScene* scene, const char* filename );
	void InitMesh( unsigned index, const aiMesh* mesh );
	void LoadBoneData( unsigned index, const aiMesh* mesh, std::vector< Vertex >& verts );

	// Importer
	Assimp::Importer mImporter;
	const aiScene* mScene;

	// Mesh data
	std::vector< MeshEntry > mEntries;
	std::vector< Texture2D* > mTextures;

	// Skeletal data
	struct BoneInfo
	{
		glm::mat4 boneOffset;
		glm::mat4 boneTM;
	};
	unsigned mNumBones;
	std::vector< BoneInfo > mBoneInfo;
	std::map< std::string, unsigned > mBoneMap;

	// Registry
	std::string mName;
	static std::map< std::string, Mesh* > mMeshRegistry;

	// Physics
	btCollisionShape* mCollisionShape;

	// Shader attribute locations
	static const unsigned POSITION_LOC  = 0U;
	static const unsigned NORMAL_LOC    = 1U;
	static const unsigned TEX_COORD_LOC = 2U;
};