/*
 * Author      : Matthew Johnson
 * Date        : 17/Feb/2014
 * Description :
 *   
 */
 
#pragma once

#include <vector>
#include <map>

class Texture2D;

class MD2Model
{
private:
	static const int MD2_MAX_TRIANGLES = 4096;
	static const int MD2_MAX_VERTICES = 2048;
	static const int MD2_MAX_TEXCOORDS = 2048;
	static const int MD2_MAX_FRAMES	= 512;
	static const int MD2_MAX_SKINS = 32;
	static const int MD2_MAX_FRAMESIZE = ( MD2_MAX_VERTICES * 4 + 128 );

	struct MD2Header
	{
		int id;				// File Type - Normally 'IPD2'
		int version;		// Version
		int texWidth;		// Texture width
		int texHeight;		// Texture height 
		int frameSize;		// Size for frames in bytes
		int nTextures;		// Number of textures
		int nVertices;		// Number of vertices
		int nTexCoords;		// Number of UVs
		int nTriangles;		// Number of tris
		int nGLCmd;			// Number of gl commands
		int nFrames;		// Number of frames
		int texOffset;		// Offset to texture names
		int UVOffset;		// Offset to UV data
		int faceOffset;		// Offset to tri data
		int frameOffset;	// Offset to first frame  
		int glCmdOffset;	// Offset to gl commands
		int EOFOffset;		// Size of file
	};

	struct MD2Vertex
	{
		unsigned char vertex[3];
		unsigned char lightNormalIndex;
	};

	struct MD2Face
	{
		short vertexIndices[3];
		short textureIndices[3];
	};

	struct MD2FaceVertex
	{
		float vertex[3];
		float normal[3];
		float uv[2];
	};

	struct MD2UV
	{
		short u, v;
	};

	struct MD2FrameInfo
	{
		float scale[3];
		float translate[3];
		char name[16];
		MD2Vertex vertices[1];
	};

	struct MD2Frame
	{
		char name[16];
		MD2FaceVertex* vertices;
	};

	struct MD2GLCommand
	{
		float u, v;
		int vertexIndex;
	};

public:
	struct MD2AnimInfo
	{
		char name[16];
		int start;
		int end;
		int numFrames;
	};
private:

	typedef char MD2TextureName[64];

	MD2Header mHeader;
	MD2TextureName* mTextureNames;
	MD2UV* mUVs;
	MD2Face* mFaces;
	MD2Frame* mFrames;
	int* mGLCommands;

	struct Vertex
	{
		float vx, vy, vz;
		float u, v;
		float nx, ny, nz;
	};
	std::vector< Vertex > mVertices;
	std::vector< unsigned > mIndices;
	std::vector< unsigned > mIdVBO;
	unsigned mIdIBO;
	unsigned mNumIndices;
	Texture2D** mTextures;
	std::map< std::string, MD2AnimInfo > mAnimInfo;

	static std::map< std::string, MD2Model* > mModelRegistry;

	MD2Model();
	~MD2Model();

	void CacheAnimInfo();
	void FreeMD2Data();
public:
	// Creates and registers a new MD2Model
	static MD2Model* CreateMD2Model( const char* filename );
	static MD2Model* CreateMD2Model( const std::string& filename );

	static void DestroyAllMD2Models();

	bool Load( const char* filename );
	void Draw( int frame, int textureIndex=0 );
	void DrawInterpolated( int frame1, int frame2, float mix, int textureIndex=0 );
	
	const MD2AnimInfo* GetAnimationInfo( const std::string& animName );

	// Remove the numbers from the frame tag
	static char* GetFrameName( MD2Frame* frame, char _out_name[16] );
};