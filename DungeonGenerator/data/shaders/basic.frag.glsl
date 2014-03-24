#version 330

#define FOG_START 28.0
#define FOG_END 30.0

const int MAX_POINT_LIGHTS = 64;

struct GlobalAmbientLight
{
	vec3 Color;
	float Intensity;
};

struct PointLight
{
	vec3 Color;
	float Intensity;
	vec3 Position;
	float ConstantAtten;
	float LinearAtten;
};

in vec2 vTexCoord;
in vec3 vNormal;
in vec3 vModelPos;
in vec3 vPosition;

uniform GlobalAmbientLight uGlobalAmbientLight;
uniform PointLight uPointLights[ MAX_POINT_LIGHTS ];
uniform sampler2D uTexture;
uniform vec3 uColor;

out vec4 fFragColor;

void main()
{
	vec4 totalLight = vec4( uGlobalAmbientLight.Color * uGlobalAmbientLight.Intensity, 1.0 );

	for ( int i = 0; i < MAX_POINT_LIGHTS; ++i )
	{
		vec3 lightDir = vModelPos - uPointLights[ i ].Position;
		float d = length( lightDir );
		lightDir = normalize( lightDir );
		float atten = clamp( uPointLights[ i ].ConstantAtten + ( uPointLights[ i ].LinearAtten - d ) / uPointLights[ i ].LinearAtten, 0.0, 1.0 );
		//float atten = uPointLights[ i ].ConstantAtten + uPointLights[ i ].LinearAtten * d;
		//atten = clamp( 1.0 / atten, 0.0, 1.0 );
		vec4 color = vec4( uPointLights[ i ].Color * uPointLights[ i ].Intensity, 1.0 );
		totalLight += color * atten;
	}

	float fogIntensity = clamp( 1.0 * ( vPosition.z - FOG_START ) / ( FOG_END - FOG_START ), 0.0, 1.0 );

	fFragColor = texture2D( uTexture, vTexCoord.xy ) * vec4( uColor, 1 ) * totalLight * ( 1.0 - fogIntensity ) + fogIntensity * vec4( 0, 0, 0, 1 );
}