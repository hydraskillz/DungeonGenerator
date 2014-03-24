#version 330

in layout (location=0) vec3 aPosition;
in layout (location=1) vec3 aNormal;
in layout (location=2) vec2 aTexCoord;

in layout (location=3) vec3 aPositionNext;
in layout (location=4) vec3 aNormalNext;

uniform mat4 uMVP;
uniform mat4 uModel;
uniform float uInterpolationFactor;

out vec2 vTexCoord;
out vec3 vNormal;
out vec3 vModelPos;
out vec3 vPosition;

void main()
{
	vec3 interpolatedNormal = mix( aNormal, aNormalNext, uInterpolationFactor );
	vec3 interpolatedPosition = mix( aPosition, aPositionNext, uInterpolationFactor );

	vec4 position = uMVP * vec4( interpolatedPosition, 1.0 );
	vPosition = vec3( position );
	vTexCoord = aTexCoord;
	vNormal   = ( uModel * vec4( interpolatedNormal, 0.0 ) ).xyz;
	vModelPos = ( uModel * vec4( interpolatedPosition, 1.0 ) ).xyz;

	gl_Position = position;
}