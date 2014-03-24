/*
 * Author      : Matthew Johnson
 * Date        : 15/Sep/2013
 * Description :
 *   
 */
 
#pragma once

#include <glm/glm.hpp>

class Uniform
{
public:
	static const int INVALID_LOCATION = 0xFFFFFFFF;

	Uniform( int location=INVALID_LOCATION );
	virtual ~Uniform();

	void SetLocation( int location ) { mLocation = location; }

	virtual void Apply() = 0;

protected:
	int mLocation;
};




class Uniform1i
	: public Uniform
{
public:
	Uniform1i( int location=INVALID_LOCATION );
	virtual ~Uniform1i();

	void Apply();

	void SetValue( int value );
	inline int GetValue() const { return mValue; }

private:
	int mValue;
};




class Uniform1f
	: public Uniform
{
public:
	Uniform1f( int location=INVALID_LOCATION );
	virtual ~Uniform1f();

	void Apply();

	void SetValue( float value );
	inline float GetValue() const { return mValue; }

private:
	float mValue;
};



class Uniform2f
	: public Uniform
{
public:
	Uniform2f( int location=INVALID_LOCATION );
	virtual ~Uniform2f();

	void Apply();

	void SetValue( const glm::vec2& value );
	inline glm::vec2 GetValue() const { return mValue; }

private:
	glm::vec2 mValue;
};




class Uniform3f
	: public Uniform
{
public:
	Uniform3f( int location=INVALID_LOCATION );
	virtual ~Uniform3f();

	void Apply();

	void SetValue( const glm::vec3& value );
	inline glm::vec3 GetValue() const { return mValue; }

private:
	glm::vec3 mValue;
};




class Uniform1fv
	: public Uniform
{
public:
	Uniform1fv( int location=INVALID_LOCATION );
	virtual ~Uniform1fv();

	void Apply();

	void SetValue( float* value, int count );
	inline float* GetValue() const { return mValue; }
	inline int GetCount() const	{ return mCount; }

private:
	float* mValue;
	int mCount;
};




class Uniform2fv
	: public Uniform
{
public:
	Uniform2fv( int location=INVALID_LOCATION );
	virtual ~Uniform2fv();

	void Apply();

	void SetValue( glm::vec2* value, int count );
	inline glm::vec2* GetValue() const { return mValue; }
	inline int GetCount() const	{ return mCount; }

private:
	glm::vec2* mValue;
	int mCount;
};




class UniformMatrix4fv
	: public Uniform
{
public:
	UniformMatrix4fv( int location=INVALID_LOCATION );
	virtual ~UniformMatrix4fv();

	void Apply();

	void SetValue( glm::mat4* value, int count );
	inline glm::mat4* GetValue() const { return mValue; }
	inline int GetCount() const	{ return mCount; }

private:
	glm::mat4* mValue;
	int mCount;
};