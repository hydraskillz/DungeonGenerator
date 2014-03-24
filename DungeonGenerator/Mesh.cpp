#include "Mesh.h"
#include "Util.h"
#include "Texture.h"
#include "StringUtil.h"
#include "Logger.h"

#include "BulletCollision/CollisionShapes/btCollisionShape.h"

#include <aiScene.h>
#include <aiMesh.h>
#include <assimp.hpp>
#include <aiPostProcess.h>
#include <glew.h>

glm::mat4 glmMat4FromaiMat4( const aiMatrix4x4& aiMat )
{
	return glm::mat4(
		aiMat.a1, aiMat.a2, aiMat.a3, aiMat.a4,
		aiMat.b1, aiMat.b2, aiMat.b3, aiMat.b4,
		aiMat.c1, aiMat.c2, aiMat.c3, aiMat.c4,
		aiMat.d1, aiMat.d2, aiMat.d3, aiMat.d4 );
}

std::map< std::string, Mesh* > Mesh::mMeshRegistry;

//---------------------------------------
void Mesh::RegisterMesh( const char* key, Mesh* mesh )
{
	mMeshRegistry[ key ] = mesh;
	mesh->mName = key;
}
//---------------------------------------
void Mesh::UnregisterMesh( const char* key )
{
	auto i = mMeshRegistry.find( key );
	if ( i != mMeshRegistry.end() )
	{
		mMeshRegistry.erase( i );
	}
}
//---------------------------------------
Mesh* Mesh::CreateMesh( const char* filename )
{
	Mesh*& mesh = mMeshRegistry[ filename ];
	if ( !mesh )
	{
		mesh = new Mesh();
		mesh->Load( filename );
	}
	return mesh;
}
//---------------------------------------
Mesh* Mesh::CreateMesh( const std::string& filename )
{
	return CreateMesh( filename.c_str() );
}
//---------------------------------------
Mesh* Mesh::CreateMesh( const char* name, const std::vector< Mesh::Vertex >& verts, const std::vector< unsigned >& indices )
{
	Mesh*& mesh = mMeshRegistry[ name ];
	if ( !mesh )
	{
		mesh = new Mesh();
		mesh->mEntries.resize( 1 );
		mesh->mEntries[0].Init( verts, indices );
		mesh->mTextures.resize( 1 );
		mesh->mTextures[0] = Texture2D::CreateTexture( "../data/textures/white.png" );
		mesh->mName = name;
	}
	return mesh;
}
//---------------------------------------
void Mesh::DestroyAllMeshes()
{
	for ( auto itr = mMeshRegistry.begin(); itr != mMeshRegistry.end(); ++itr )
	{
		Mesh*& mesh = itr->second;
		delete mesh;
		mesh = 0;
	}
	mMeshRegistry.clear();
}
//---------------------------------------
void Mesh::DestroyMesh( Mesh*& mesh )
{
	Mesh*& mesh2 = mMeshRegistry[ mesh->mName ];
	if ( mesh2 )
	{
		UnregisterMesh( mesh->mName.c_str() );
		//	delete mesh2;
		mesh = 0;
	}
}
//---------------------------------------
void Mesh::InvalidatePhysics()
{
	for ( auto itr = mMeshRegistry.begin(); itr != mMeshRegistry.end(); ++itr )
	{
		Mesh* m = itr->second;
		m->mCollisionShape = 0;
	}
}
//---------------------------------------


//---------------------------------------
Mesh::MeshEntry::MeshEntry()
	: idVB( INVALID_ID )
	, idIB( INVALID_ID )
	, numIndices( 0 )
	, materialIndex( INVALID_ID )
{}
//---------------------------------------
Mesh::MeshEntry::~MeshEntry()
{
	if ( idVB != INVALID_ID )
		glDeleteBuffers( 1, &idVB );
	if ( idIB != INVALID_ID )
		glDeleteBuffers( 1, &idIB );
}
//---------------------------------------
void Mesh::MeshEntry::Init( const std::vector< Vertex >& verts, const std::vector< unsigned >& indices )
{
	numIndices = indices.size();

	glGenBuffers( 1, &idVB );
	glBindBuffer( GL_ARRAY_BUFFER, idVB );
	glBufferData( GL_ARRAY_BUFFER, sizeof( Vertex ) * verts.size(), &verts[0], GL_STATIC_DRAW );

	glGenBuffers( 1, &idIB );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, idIB );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( unsigned ) * numIndices, &indices[0], GL_STATIC_DRAW );

	mVerts = verts;
	mIndices = indices;
}
//---------------------------------------


//---------------------------------------
Mesh::Mesh()
	: mCollisionShape( 0 )
{}
//---------------------------------------
Mesh::~Mesh()
{}
//---------------------------------------
bool Mesh::Load( const char* filename )
{
	mName = filename;

	mScene = mImporter.ReadFile( filename,
		aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs );

	if ( mScene )
	{
		if ( mScene->HasAnimations() )
		{
			mScene->mRootNode->mTransformation;
		}

		return InitScene( mScene, filename );
	}

	DebugPrintf( "Error loading Mesh: '%s': '%s'\n", filename, mImporter.GetErrorString() );
	return false;
}
//---------------------------------------
void Mesh::Draw()
{
	glFrontFace( GL_CCW );
#if 1
	glEnableVertexAttribArray( POSITION_LOC );
	glEnableVertexAttribArray( NORMAL_LOC );
	glEnableVertexAttribArray( TEX_COORD_LOC );

	for ( unsigned i = 0; i < mEntries.size(); ++i )
	{
		if ( mTextures[mEntries[i].materialIndex] )
		{
			mTextures[mEntries[i].materialIndex]->Bind();
		}

		glBindBuffer( GL_ARRAY_BUFFER, mEntries[i].idVB );
		glVertexAttribPointer( POSITION_LOC, 3, GL_FLOAT, GL_FALSE, sizeof( Vertex ), (const void*) offsetof( Vertex, position.x ) );
		glVertexAttribPointer( NORMAL_LOC, 3, GL_FLOAT, GL_FALSE, sizeof( Vertex ), (const void*) offsetof( Vertex, normal.x ) );
		glVertexAttribPointer( TEX_COORD_LOC, 2, GL_FLOAT, GL_FALSE, sizeof( Vertex ), (const void*) offsetof( Vertex, texture.x ) );
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, mEntries[i].idIB );

		glDrawElements( GL_TRIANGLES, mEntries[i].numIndices, GL_UNSIGNED_INT, 0 );
	}

	glDisableVertexAttribArray( POSITION_LOC );
	glDisableVertexAttribArray( NORMAL_LOC );
	glDisableVertexAttribArray( TEX_COORD_LOC );
#endif
#if 0
	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_NORMAL_ARRAY );
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );

	for ( unsigned i = 0; i < mEntries.size(); ++i )
	{
		if ( mTextures[mEntries[i].materialIndex] )
			mTextures[mEntries[i].materialIndex]->Bind();

		glBindBuffer( GL_ARRAY_BUFFER, mEntries[i].idVB );
		glVertexPointer( 3, GL_FLOAT, sizeof( Vertex ), (const void*) offsetof( Vertex, position.x ) );
		glTexCoordPointer( 2, GL_FLOAT, sizeof( Vertex ), (const void*) offsetof( Vertex, texture.x ) );
		glNormalPointer( GL_FLOAT, sizeof( Vertex ), (const void*) offsetof( Vertex, normal.x ) );
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, mEntries[i].idIB );
		
		glDrawElements( GL_TRIANGLES, mEntries[i].numIndices, GL_UNSIGNED_INT, 0 );
	}

	glDisableClientState( GL_VERTEX_ARRAY );
	glDisableClientState( GL_NORMAL_ARRAY );
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );
#endif
	glFrontFace( GL_CW );
}
//---------------------------------------
bool Mesh::InitScene( const aiScene* scene, const char* filename )
{
	mEntries.resize( scene->mNumMeshes );
	mTextures.resize( scene->mNumMaterials );

	for ( unsigned i = 0; i < mEntries.size(); ++i )
	{
		const aiMesh* mesh = scene->mMeshes[i];
		InitMesh( i, mesh );
	}

	return InitMaterials( scene, filename );
}
//---------------------------------------
bool Mesh::InitMaterials( const aiScene* scene, const char* filename )
{
	// Extract the directory part from the file name
	std::string sfilename( filename );
	std::string::size_type SlashIndex = sfilename.find_last_of("/");
	std::string Dir;

	if (SlashIndex == std::string::npos) {
		Dir = ".";
	}
	else if (SlashIndex == 0) {
		Dir = "/";
	}
	else {
		Dir = sfilename.substr(0, SlashIndex);
	}

	bool result = false;

	for ( unsigned i = 0; i < scene->mNumMaterials; ++i )
	{
		const aiMaterial* mat = scene->mMaterials[i];
		mTextures[i] = 0;

		if ( mat->GetTextureCount( aiTextureType_DIFFUSE ) > 0 )
		{
			aiString path;

			if ( mat->GetTexture( aiTextureType_DIFFUSE, 0 , &path, 0, 0, 0, 0, 0 ) == aiReturn_SUCCESS )
			{
				// Just gonna assume the textures are next to the mesh files
				std::string txFilename = StringUtil::ExtractFilenameFromPath( path.data );
				std::string fullPath = Dir + "/" + txFilename;
				mTextures[i] = Texture2D::CreateTexture( fullPath.c_str(), false );

				if ( !mTextures[i]->Load() )
				{
					Texture2D::DestroyTexture( mTextures[i] );
					result = false;
				}
			}
		}

		if ( !mTextures[i] )
		{
			mTextures[i] = Texture2D::CreateTexture( "../data/textures/white.png" );
			result = mTextures[i]->Load();
		}
	}

	return result;
}
//---------------------------------------
void Mesh::InitMesh( unsigned index, const aiMesh* mesh )
{
	mEntries[index].materialIndex = mesh->mMaterialIndex;

	std::vector< Vertex > verts;
	std::vector< unsigned > indices;

	const aiVector3D zero( 0.0f );

	for ( unsigned i = 0; i < mesh->mNumVertices; ++i )
	{
		const aiVector3D& pos  = mesh->mVertices[i];
		const aiVector3D& norm = mesh->mNormals[i];
		const aiVector3D& tex  = mesh->HasTextureCoords(0) ? mesh->mTextureCoords[0][i] : zero;

		Vertex v( glm::vec3( pos.x, pos.y, pos.z ),
				  glm::vec2( tex.x, tex.y ),
				  glm::vec3( norm.x, norm.y, norm.z ) );

		verts.push_back( v );
	}

	LoadBoneData( index, mesh, verts );

	for ( unsigned i = 0; i < mesh->mNumFaces; ++i )
	{
		const aiFace& face = mesh->mFaces[i];

		indices.push_back( face.mIndices[0] );
		indices.push_back( face.mIndices[1] );
		indices.push_back( face.mIndices[2] );
	}

	mEntries[index].Init( verts, indices );
}
//---------------------------------------
void Mesh::LoadBoneData( unsigned index, const aiMesh* mesh, std::vector< Vertex >& verts )
{
	for ( unsigned i = 0; i < mesh->mNumBones; ++i )
	{
		unsigned boneIndex = 0;
		const aiBone* bone = mesh->mBones[i];
		std::string boneName = bone->mName.data;

		auto itr = mBoneMap.find( boneName );
		if ( itr != mBoneMap.end() )
		{
			boneIndex = itr->second;
		}
		else
		{
			boneIndex = mNumBones++;
			mBoneInfo.push_back( BoneInfo() );
			mBoneInfo[ boneIndex ].boneOffset = glmMat4FromaiMat4( bone->mOffsetMatrix );
			itr->second = boneIndex;

			DebugPrintf( "Adding Bone '%s' : %u\n", itr->first.c_str(), itr->second );
		}

		unsigned nWeights = bone->mNumWeights;
		assert( nWeights <= 4U );
		for ( unsigned j = 0; j < nWeights; ++j )
		{
			const unsigned vertexId = bone->mWeights[j].mVertexId;
			verts[ vertexId ].boneWeights[j] = bone->mWeights[j].mWeight;
			verts[ vertexId ].boneIds[j] = boneIndex;
		}
	}
}
//---------------------------------------