#include "MD2Model.h"

#include "Texture.h"
#include "StringUtil.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <glew.h>

#define MD2_DEBUG_OUTPUT 0

std::map< std::string, MD2Model* > MD2Model::mModelRegistry;

//---------------------------------------
MD2Model* MD2Model::CreateMD2Model( const char* filename )
{
	MD2Model*& model = mModelRegistry[ filename ];
	if ( !model )
	{
		model = new MD2Model();
		model->Load( filename );
	}
	return model;
}
//---------------------------------------
MD2Model* MD2Model::CreateMD2Model( const std::string& filename )
{
	return CreateMD2Model( filename.c_str() );
}
//---------------------------------------
void MD2Model::DestroyAllMD2Models()
{
	for ( auto itr = mModelRegistry.begin(); itr != mModelRegistry.end(); ++itr )
	{
		MD2Model*& model = itr->second;
		delete model;
		model = 0;
	}
	mModelRegistry.clear();
}
//---------------------------------------
char* MD2Model::GetFrameName( MD2Frame* frame, char _out_name[16] )
{
	strcpy( _out_name, frame->name );
	int n = strlen( _out_name ) - 1;
	int i = 0;
	while ( _out_name[ n ] >= '0' && _out_name[ n ] <= '9' && i < 2 )
	{
		--n;
		++i;
	}
	_out_name[ n + 1 ] = 0;
	return _out_name;
}
//---------------------------------------




//---------------------------------------
MD2Model::MD2Model()
	: mTextureNames( 0 )
	, mUVs( 0 )
	, mFaces( 0 )
	, mFrames( 0 )
	, mGLCommands( 0 )
{}
//---------------------------------------
MD2Model::~MD2Model()
{
	FreeMD2Data();
	delete[] mTextures;
}
//---------------------------------------
void MD2Model::FreeMD2Data()
{
	if ( mTextureNames )
		delete[] mTextureNames;

	if ( mUVs )
		delete[] mUVs;

	if ( mFaces )
		delete[] mFaces;

	if ( mFrames )
	{
		for ( int i = 0; i < mHeader.nFrames; ++i )
		{
			if ( mFrames[i].vertices )
				delete[] mFrames[i].vertices;
		}
	}

	if ( mGLCommands )
		delete[] mGLCommands;
}
//---------------------------------------
bool MD2Model::Load( const char* filename )
{
	FILE* file;
	unsigned char buffer[ MD2_MAX_FRAMESIZE ];

	file = fopen( filename, "rb" );

	if ( !file )
	{
		printf( "Failed to load md2 '%s' : cannot open file.\n", filename );
		return false;
	}

	// Read header
	fread( &mHeader, sizeof( MD2Header ), 1, file );

#if MD2_DEBUG_OUTPUT
	printf( "MD2Model:          %s\n", filename );
	printf( "ID:                %d\n", mHeader.id );
	printf( "Version:           %d\n", mHeader.version );   
	printf( "Tex Width:         %d\n", mHeader.texWidth );
	printf( "Tex Height:        %d\n", mHeader.texHeight );
	printf( "Frame Size:        %d\n", mHeader.frameSize );
	printf( "Textures:          %d\n", mHeader.nTextures );
	printf( "Vertices:          %d\n", mHeader.nVertices );
	printf( "UVs:               %d\n", mHeader.nTexCoords );
	printf( "Faces:             %d\n", mHeader.nTriangles );
	printf( "GL cmds:           %d\n", mHeader.nGLCmd );
	printf( "Frames:            %d\n", mHeader.nFrames );
	printf( "Texture Offset:    %d\n", mHeader.texOffset );
	printf( "UV Offset:         %d\n", mHeader.UVOffset );
	printf( "Face Offset:       %d\n", mHeader.faceOffset );
	printf( "Frame Offset:      %d\n", mHeader.frameOffset );
	printf( "GL Offset:         %d\n", mHeader.glCmdOffset );
	printf( "File Size:         %d\n", mHeader.EOFOffset );
#endif

	// id must be 'IDP2' as dword
	if ( mHeader.id != 844121161 )
	{
		printf( "Failed to load md2 '%s' : Invalid id\n" );
		return false;
	}

	// Read textures
	if ( mHeader.nTextures > 0 )
	{
		fseek( file, mHeader.texOffset, SEEK_SET );
		mTextureNames = new MD2TextureName[ mHeader.nTextures ];
		fread( &mTextureNames[0], sizeof( MD2TextureName ), mHeader.nTextures, file );
	}

	// Read UV coords
	if ( mHeader.nTexCoords > 0 )
	{
		fseek( file, mHeader.UVOffset, SEEK_SET );
		mUVs = new MD2UV[ mHeader.nTexCoords ];
		fread( &mUVs[0], sizeof( MD2UV ), mHeader.nTexCoords, file );
	}

	// Read faces
	if ( mHeader.nTriangles > 0 )
	{
		fseek( file, mHeader.faceOffset, SEEK_SET );
		mFaces = new MD2Face[ mHeader.nTriangles ];
		fread( &mFaces[0], sizeof( MD2Face ), mHeader.nTriangles, file );
	}

	// Read frames
	if ( mHeader.nFrames > 0 )
	{
		fseek( file, mHeader.frameOffset, SEEK_SET );
		mFrames = new MD2Frame[ mHeader.nFrames ];
		for ( int i = 0; i < mHeader.nFrames; ++i )
		{
			MD2FrameInfo* frame = (MD2FrameInfo*) buffer;
			mFrames[i].vertices = new MD2FaceVertex[ mHeader.nVertices ];
			fread( frame, mHeader.frameSize, 1, file );
			strcpy( mFrames[i].name, frame->name );
			for ( int j = 0; j < mHeader.nVertices; ++j )
			{
				// Swizzle vertices so +y is up
				mFrames[i].vertices[j].vertex[0] = (float) ( (int) frame->vertices[j].vertex[0] ) * frame->scale[0] + frame->translate[0];
				mFrames[i].vertices[j].vertex[2] = -1.0f * ( ( (int) frame->vertices[j].vertex[1] ) * frame->scale[1] + frame->translate[1] );
				mFrames[i].vertices[j].vertex[1] = (float) ( (int) frame->vertices[j].vertex[2] ) * frame->scale[2] + frame->translate[2];
			}

			for ( int j = 0; j < mHeader.nTriangles; ++j )
			{
				MD2Face* face = &mFaces[j];

				for ( int k = 0; k < 3; ++k )
				{

					mFrames[i].vertices[ face->vertexIndices[k] ].uv[0] = mUVs[ face->textureIndices[k] ].u / (float) mHeader.texWidth;
					mFrames[i].vertices[ face->vertexIndices[k] ].uv[1] = mUVs[ face->textureIndices[k] ].v / (float) mHeader.texHeight;
				}
			}
		}

		// Calculate normals
		for ( int i = 0; i < mHeader.nFrames; ++i )
		{
			for ( int j = 0; j < mHeader.nVertices; ++j )
			{
				mFrames[i].vertices[j].normal[0] = 0;
				mFrames[i].vertices[j].normal[1] = 0;
				mFrames[i].vertices[j].normal[2] = 0;
			}

			for ( int j = 0; j < mHeader.nTriangles; ++j )
			{
				float normal[3];
				float v1[3], v2[3];
				MD2Face* face = &mFaces[j];

				v1[0] = mFrames[i].vertices[ face->vertexIndices[1] ].vertex[0] - mFrames[i].vertices[ face->vertexIndices[0] ].vertex[0];
				v1[1] = mFrames[i].vertices[ face->vertexIndices[1] ].vertex[1] - mFrames[i].vertices[ face->vertexIndices[0] ].vertex[1];
				v1[2] = mFrames[i].vertices[ face->vertexIndices[1] ].vertex[2] - mFrames[i].vertices[ face->vertexIndices[0] ].vertex[2];

				v2[0] = mFrames[i].vertices[ face->vertexIndices[2] ].vertex[0] - mFrames[i].vertices[ face->vertexIndices[0] ].vertex[0];
				v2[1] = mFrames[i].vertices[ face->vertexIndices[2] ].vertex[1] - mFrames[i].vertices[ face->vertexIndices[0] ].vertex[1];
				v2[2] = mFrames[i].vertices[ face->vertexIndices[2] ].vertex[2] - mFrames[i].vertices[ face->vertexIndices[0] ].vertex[2];

				normal[0] = v1[1] * v2[2] - v1[2] * v2[1];
				normal[1] = v1[2] * v2[0] - v1[0] * v2[2];
				normal[2] = v1[0] * v2[1] - v1[1] * v2[0];

				for ( int k = 0; k < 3; ++k )
				{
					mFrames[i].vertices[ face->vertexIndices[k] ].normal[0] -= normal[0];
					mFrames[i].vertices[ face->vertexIndices[k] ].normal[1] += normal[2];
					mFrames[i].vertices[ face->vertexIndices[k] ].normal[2] -= normal[1];
				}
			}

			for ( int j = 0; j < mHeader.nVertices; ++j )
			{
				float* normal = mFrames[i].vertices[j].normal;
				float len = sqrt( normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2] );
				if ( len != 0 )
				{
					normal[0] /= len;
					normal[1] /= len;
					normal[2] /= len;
				}
			}
		}
	}

	// Read gl commands
	if ( mHeader.nGLCmd > 0 )
	{
		fseek( file, mHeader.glCmdOffset, SEEK_SET );
		mGLCommands = new int[ mHeader.nGLCmd ];
		fread( mGLCommands, sizeof( int ), mHeader.nGLCmd, file );
	}

	fclose( file );

	mIdVBO.resize( mHeader.nFrames );
	glGenBuffers( mHeader.nFrames, &mIdVBO[0] );
	for ( int k = 0; k < mHeader.nFrames; ++k )
	{
		MD2Frame* f = &mFrames[ k ];
		mVertices.clear();
		for ( int i = 0; i < mHeader.nVertices; ++i )
		{
			Vertex v;

			v.nx = f->vertices[ i ].normal[0];
			v.ny = f->vertices[ i ].normal[1];
			v.nz = f->vertices[ i ].normal[2];

			v.u = f->vertices[ i ].uv[0];//mUVs[ i ].u / (float) mHeader.texWidth;
			v.v = f->vertices[ i ].uv[1];//mUVs[ i ].v / (float) mHeader.texHeight;

			v.vx = f->vertices[ i ].vertex[0];
			v.vy = f->vertices[ i ].vertex[1];
			v.vz = f->vertices[ i ].vertex[2];

			mVertices.push_back( v );
		}
		
		glBindBuffer( GL_ARRAY_BUFFER, mIdVBO[k] );
		glBufferData( GL_ARRAY_BUFFER, sizeof( Vertex ) * mVertices.size(), &mVertices[0], GL_STATIC_DRAW );
	}

	for ( int i = 0; i < mHeader.nTriangles; ++i )
	{
		MD2Face* face = &mFaces[i];
		mIndices.push_back( face->vertexIndices[0] );
		mIndices.push_back( face->vertexIndices[1] );
		mIndices.push_back( face->vertexIndices[2] );
	}

	mNumIndices = mIndices.size();
	
	glGenBuffers( 1, &mIdIBO );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, mIdIBO );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( unsigned ) * mNumIndices, &mIndices[0], GL_STATIC_DRAW );

	mTextures = new Texture2D*[ mHeader.nTextures ];
	for ( int i = 0; i < mHeader.nTextures; ++i )
	{
		int len = strlen( mTextureNames[i] );
		strcpy( mTextureNames[i] + len - 3, "png" );
		std::string texturePath = StringUtil::StripFilenameFromPath( filename ) + "/" + StringUtil::ExtractFilenameFromPath( mTextureNames[i] );
		mTextures[i] = Texture2D::CreateTexture( texturePath, false );
	}

	CacheAnimInfo();

	return true;
}
//---------------------------------------
void MD2Model::Draw( int frame, int textureIndex )
{
	mTextures[ textureIndex ]->Bind();

	glEnableVertexAttribArray( 0 );
	glEnableVertexAttribArray( 1 );
	glEnableVertexAttribArray( 2 );

	glBindBuffer( GL_ARRAY_BUFFER, mIdVBO[ frame ] );
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( Vertex ), (const void*) offsetof( Vertex, vx ) );
	glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, sizeof( Vertex ), (const void*) offsetof( Vertex, nx ) );
	glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, sizeof( Vertex ), (const void*) offsetof( Vertex, u ) );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, mIdIBO );

	glDrawElements( GL_TRIANGLES, mNumIndices, GL_UNSIGNED_INT, 0 );

	glDisableVertexAttribArray( 0 );
	glDisableVertexAttribArray( 1 );
	glDisableVertexAttribArray( 2 );
	
	/*MD2Frame* f = &mFrames[ frame ];

	glBegin( GL_TRIANGLES );
	{
		for ( int i = 0; i < mHeader.nTriangles; ++i )
		{
			MD2Face* face = &mFaces[i];

			for ( int j = 0; j < 3; ++j )
			{
				//glVertexAttrib3f( 1,
				glNormal3f(
					f->vertices[ face->vertexIndices[j] ].normal[0],
					f->vertices[ face->vertexIndices[j] ].normal[1],
					f->vertices[ face->vertexIndices[j] ].normal[2] );

				//glVertexAttrib2f( 2,
				glTexCoord2f(
					mUVs[ face->textureIndices[j] ].u / (float) mHeader.texWidth,
					mUVs[ face->textureIndices[j] ].v / (float) mHeader.texHeight );

				//glVertexAttrib3f( 0,
				glVertex3f(
					f->vertices[ face->vertexIndices[j] ].vertex[0],
					f->vertices[ face->vertexIndices[j] ].vertex[1],
					f->vertices[ face->vertexIndices[j] ].vertex[2] );
			}
		}
	}
	glEnd();*/
}
//---------------------------------------
void MD2Model::DrawInterpolated( int frame1, int frame2, float mix, int textureIndex )
{
	mTextures[ textureIndex ]->Bind();

	glEnableVertexAttribArray( 0 );
	glEnableVertexAttribArray( 1 );
	glEnableVertexAttribArray( 2 );
	glEnableVertexAttribArray( 3 );
	glEnableVertexAttribArray( 4 );

	glBindBuffer( GL_ARRAY_BUFFER, mIdVBO[ frame1 ] );
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( Vertex ), (const void*) offsetof( Vertex, vx ) );
	glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, sizeof( Vertex ), (const void*) offsetof( Vertex, nx ) );
	glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, sizeof( Vertex ), (const void*) offsetof( Vertex, u ) );

	glBindBuffer( GL_ARRAY_BUFFER, mIdVBO[ frame2 ] );
	glVertexAttribPointer( 3, 3, GL_FLOAT, GL_FALSE, sizeof( Vertex ), (const void*) offsetof( Vertex, vx ) );
	glVertexAttribPointer( 4, 3, GL_FLOAT, GL_FALSE, sizeof( Vertex ), (const void*) offsetof( Vertex, nx ) );

	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, mIdIBO );

	glDrawElements( GL_TRIANGLES, mNumIndices, GL_UNSIGNED_INT, 0 );

	glDisableVertexAttribArray( 0 );
	glDisableVertexAttribArray( 1 );
	glDisableVertexAttribArray( 2 );
	glDisableVertexAttribArray( 3 );
	glDisableVertexAttribArray( 4 );
}
//---------------------------------------
void MD2Model::CacheAnimInfo()
{
	char name[16];
	char last[16];
	int startFrame = 0;
	int endFrame = 0;
	int nFrames = 0;
	int frameCount = 0;

	GetFrameName( &mFrames[0], last );

	for ( int i = 0; i <= mHeader.nFrames; ++i )
	{
		if ( i == mHeader.nFrames )
			strcpy( name, "" );
		else
			GetFrameName( &mFrames[i], name );

		if ( strcmp( last, name ) )
		{	
			startFrame = frameCount - nFrames;
			endFrame = frameCount - 1;
#if MD2_DEBUG_OUTPUT
			printf( "MD2Anim\n name: %s\n frames: %d\n start: %d\n end: %d\n"
				, last, frameCount, startFrame, endFrame );
#endif
			MD2AnimInfo& info = mAnimInfo[ last ];
			info.start = startFrame;
			info.end = endFrame;
			info.numFrames = frameCount;

			strcpy( last, name );
			nFrames = 0;
		}
		++frameCount;
		++nFrames;
	}

	if ( mAnimInfo.size() == 0 )
	{
#if MD2_DEBUG_OUTPUT
		printf( "MD2Anim\n name: %s\n frames: %d\n start: %d\n end: %d\n"
			, last, frameCount, startFrame, endFrame );
#endif
		MD2AnimInfo& info = mAnimInfo[ last ];
		info.start = frameCount - nFrames;
		info.end = frameCount - 2;
		info.numFrames = frameCount;
	}
}
//---------------------------------------
const MD2Model::MD2AnimInfo* MD2Model::GetAnimationInfo( const std::string& animName )
{
	MD2AnimInfo* info = 0;
	auto found = mAnimInfo.find( animName );
	if ( found != mAnimInfo.end() )
	{
		info = &found->second;
	}

	return info;
}
//---------------------------------------