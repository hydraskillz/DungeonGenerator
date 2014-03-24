#include "Shader.h"
#include "Logger.h"
#include "FileSystem.h"
#include "StringUtil.h"
#include <fstream>

#ifdef ANDROID
#   include <EGL/egl.h>
#   include <GLES/gl.h>
#   include <GLES2/gl2.h>
#   include <GLES2/gl2ext.h>
#else
#   include <glew.h>
#endif

// Mapping of internal enums to OpenGL enums
static GLenum gsOGLShaderType[ Shader::ST_COUNT ] =
{
	GL_VERTEX_SHADER,
	//GL_TESS_CONTROL_SHADER,
	//GL_TESS_EVALUATION_SHADER,
	//GL_GEOMETRY_SHADER,
	GL_FRAGMENT_SHADER
};

//---------------------------------------
Shader::Shader( ShaderType type, const char* source )
{
	mShaderId = glCreateShader( gsOGLShaderType[ type ] );
    
    if ( mShaderId )
    {
        glShaderSource( mShaderId, 1, &source, NULL );
        // Compile the shader
        glCompileShader( mShaderId );
        
        // Check for and report errors
        GLint success = 0;
        glGetShaderiv( mShaderId, GL_COMPILE_STATUS, &success );
        
        if ( !success )
        {
            WarnCrit( "ShaderError: Compile Failed\n" );
            
            GLint infoLen = 0;
            glGetShaderiv( mShaderId, GL_INFO_LOG_LENGTH, &infoLen );
            
            if ( infoLen )
            {
                char* infoLog = (char*) malloc( infoLen );
                if ( infoLog )
                {
                    glGetShaderInfoLog( mShaderId, infoLen, NULL, infoLog );
                    FatalError( "%s\n", infoLog );
                    free( infoLog );
                }
                else
                {
                    FatalError( "Failed to allocate buffer for error message size=%d\n", infoLen );
                }
                
                glDeleteShader( mShaderId );
                mShaderId = 0;
            }
            else
            {
                FatalError( "No info available\n" );
            }
        }
    }
    else
    {
        WarnCrit( "Shader: Failed to create shader\n" );
        
    }
}
//---------------------------------------
Shader::~Shader()
{
	if ( mShaderId )
	{
		glDeleteShader( mShaderId );
	}
}
//---------------------------------------
uint32 Shader::GetShaderId() const
{
	return mShaderId;
}
//---------------------------------------
std::string Shader::GetShaderText( const char* filename )
{
	std::string shaderText;
	char* fileBuf;
	uint32 len;

	if ( OpenDataFile( filename, fileBuf, len ) != FSE_NO_ERROR )
	{
		std::string path = StringUtil::StripFilenameFromPath( filename );
		shaderText = PreprocessShaderText( path, fileBuf );
		delete[] fileBuf;
	
	}
	return shaderText;
}
//---------------------------------------
std::string Shader::PreprocessShaderText( const std::string& path, const std::string& textString )
{
	std::stringstream textStream;
	//std::string textString( rawText );
	int hashIndex = 0;
	int idEnd;
	int lastIndex = 0;

	do
	{
		hashIndex = textString.find( '#', hashIndex );

		if ( hashIndex == std::string::npos ) break;

		idEnd = textString.find( ' ', hashIndex );

		std::string directive = textString.substr( hashIndex+1, idEnd - hashIndex-1 );

		// Include directive
		if ( !StringUtil::StringiCmp( directive, "include" ) )
		{
			int strOpen, strClose;

			strOpen = textString.find( '"', idEnd );

			if ( strOpen == std::string::npos )
			{
				strOpen = textString.find( '<', idEnd );

				// Syntax error
				if ( strOpen == std::string::npos )
				{
					assertion( false, "Syntax error: missing opening '\"' or '<' in include statement.\n" );
				}

				strClose = textString.find( '>', strOpen+1 );

				// Syntax error
				if ( strClose == std::string::npos )
				{
					assertion( false, "Syntax error: missing closing '>' in include statement.\n" );
				}
			}
			else
			{
				strClose = textString.find( '"', strOpen+1 );

				// Syntax error
				if ( strClose == std::string::npos )
				{
					assertion( false, "Syntax error: missing closing '\"' in include statement.\n" );
				}
			}

			std::string includeFileName = textString.substr( strOpen+1, strClose - strOpen-1 );
			std::string fileText;

#ifdef ANDROID
			assertion( false, "[TODO - IMPLEMENT THIS ON ANDROID] Failed to open '%s'\n", includeFileName.c_str() );
#else
			// Adapted from:
			// http://stackoverflow.com/questions/2602013/read-whole-ascii-file-into-c-stdstring
			std::ifstream file( path + includeFileName, std::ios::in | std::ios::binary );
			if ( file )
			{
				file.seekg( 0, std::ios::end );
				fileText.resize( (uint32) file.tellg() );
				file.seekg( 0, std::ios::beg );
				file.read( &fileText[0], fileText.size() );
				file.close();
			}
			else
			{
				assertion( false, "Failed to open '%s'\n", includeFileName.c_str() );
			}
#endif

			std::string textBlock = textString.substr( lastIndex, hashIndex - lastIndex );
			lastIndex = strClose+1;

			textStream << textBlock;
			textStream << PreprocessShaderText( path, fileText );
		}
	} while ( hashIndex++ != std::string::npos );

	std::string textBlock = textString.substr( lastIndex );
	textStream << textBlock;

	//std::string debugText = textStream.str();

	return textStream.str();
}
//---------------------------------------
Shader* Shader::CreateVertexShader( const char* filename )
{
	Shader* shader;
	std::string shaderText = Shader::GetShaderText( filename );
	shader = new Shader( ST_VERTEX, shaderText.c_str() );
	return shader;
}
//---------------------------------------
