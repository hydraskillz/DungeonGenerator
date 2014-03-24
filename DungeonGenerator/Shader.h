/*
 * Author      : Matthew Johnson
 * Date        : 15/Sep/2013
 * Description :
 *   
 */
 
#pragma once

#include <string>

class Shader
{
public:
	enum ShaderType
	{
		ST_VERTEX,
		ST_FRAGMENT,
		ST_COUNT
	};

	Shader( ShaderType type, const char* source );
	~Shader();

	unsigned int GetShaderId() const;

	static std::string GetShaderText( const char* filename );
	static Shader* CreateVertexShader( const char* filename );

private:
	static std::string PreprocessShaderText( const std::string& path, const std::string& textString );

	unsigned int mShaderId;
};
