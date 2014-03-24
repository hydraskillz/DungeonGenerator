/*
 * Author      : Matthew Johnson
 * Date        : 15/Sep/2013
 * Description :
 *   
 */
 
#pragma once

#include "Types.h"
#include <vector>

class Uniform;
class Shader;

class Effect
{
public:
	static const int INVALID_LOCATION = 0xFFFFFFFF;

	Effect();
	Effect( const Shader& vs, const Shader& fs );
	~Effect();

	void SetShaders( const Shader& vs, const Shader& fs );

	void Apply() const;
	static void Disable();

	int GetUniformLocation( const char* name ) const;
	int GetAttributeLocation( const char* name ) const;

	void AddUniform( Uniform* uniform );
	void RemoveUniform( Uniform* uniform );

private:
	void DebugErrorLog();
	uint32 mProgramId;
	std::vector< Uniform* > mUniforms;
};