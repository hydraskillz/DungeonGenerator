#include "Uniform.h"

#ifdef ANDROID
#   include <EGL/egl.h>
#   include <GLES/gl.h>
#   include <GLES2/gl2.h>
#   include <GLES2/gl2ext.h>
#else
#   include <glew.h>
#endif

//---------------------------------------
Uniform::Uniform( int location )
	: mLocation( location )
{}
//---------------------------------------
Uniform::~Uniform()
{}
//---------------------------------------




//---------------------------------------
Uniform1i::Uniform1i( int location )
	: Uniform( location )
	, mValue( 0 )
{}
//---------------------------------------
Uniform1i::~Uniform1i()
{}
//---------------------------------------
void Uniform1i::Apply()
{
	glUniform1i( mLocation, mValue );
}
//---------------------------------------
void Uniform1i::SetValue( int value )
{
	mValue = value;
}
//---------------------------------------




//---------------------------------------
Uniform1f::Uniform1f( int location )
	: Uniform( location )
	, mValue( 0 )
{}
//---------------------------------------
Uniform1f::~Uniform1f()
{}
//---------------------------------------
void Uniform1f::Apply()
{
	glUniform1f( mLocation, mValue );
}
//---------------------------------------
void Uniform1f::SetValue( float value )
{
	mValue = value;
}
//---------------------------------------



//---------------------------------------
Uniform2f::Uniform2f( int location )
	: Uniform( location )
	, mValue( 0 )
{}
//---------------------------------------
Uniform2f::~Uniform2f()
{}
//---------------------------------------
void Uniform2f::Apply()
{
	glUniform2f( mLocation, mValue[0], mValue[1] );
}
//---------------------------------------
void Uniform2f::SetValue( const glm::vec2& value )
{
	mValue = value;
}
//---------------------------------------




//---------------------------------------
Uniform3f::Uniform3f( int location )
	: Uniform( location )
	, mValue( 0 )
{}
//---------------------------------------
Uniform3f::~Uniform3f()
{}
//---------------------------------------
void Uniform3f::Apply()
{
	glUniform3f( mLocation, mValue[0], mValue[1], mValue[2] );
}
//---------------------------------------
void Uniform3f::SetValue( const glm::vec3& value )
{
	mValue = value;
}
//---------------------------------------




//---------------------------------------
Uniform1fv::Uniform1fv( int location )
	: Uniform( location )
	, mValue( 0 )
	, mCount( 0 )
{}
//---------------------------------------
Uniform1fv::~Uniform1fv()
{}
//---------------------------------------
void Uniform1fv::Apply()
{
	if ( mValue )
		glUniform1fv( mLocation, mCount, mValue );
}
//---------------------------------------
void Uniform1fv::SetValue( float* value, int count )
{
	mValue = value;
	mCount = count;
}
//---------------------------------------



//---------------------------------------
Uniform2fv::Uniform2fv( int location )
	: Uniform( location )
	, mValue( 0 )
	, mCount( 0 )
{}
//---------------------------------------
Uniform2fv::~Uniform2fv()
{}
//---------------------------------------
void Uniform2fv::Apply()
{
	if ( mValue )
		glUniform2fv( mLocation, mCount, &mValue[0].x );
}
//---------------------------------------
void Uniform2fv::SetValue( glm::vec2* value, int count )
{
	mValue = value;
	mCount = count;
}
//---------------------------------------



//---------------------------------------
UniformMatrix4fv::UniformMatrix4fv( int location )
	: Uniform( location )
	, mValue( 0 )
	, mCount( 0 )
{}
//---------------------------------------
UniformMatrix4fv::~UniformMatrix4fv()
{}
//---------------------------------------
void UniformMatrix4fv::Apply()
{
	if ( mValue )
		glUniformMatrix4fv( mLocation, mCount, GL_FALSE, &mValue[0][0][0] );
}
//---------------------------------------
void UniformMatrix4fv::SetValue( glm::mat4* value, int count )
{
	mValue = value;
	mCount = count;
}
//---------------------------------------