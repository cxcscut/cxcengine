#version 430 core

in vec2 UV;
in vec3 Position_worldspace;
in vec3 EyeDirection_worldspace;
in vec3 LightDirection_worldspace;
in vec3 Normal_worldspace;

uniform sampler2D TexSampler;

out vec3 color;
uniform vec3 LightPosition_worldspace;
uniform vec3 EyePosition_worldspace;
uniform vec3 Ks;
uniform vec3 Ka;
uniform float LightPower;

void main()
{
   int shineness = 32;

	float distance = length(LightPosition_worldspace - Position_worldspace);

	vec3 LightColor = vec3(1,1,1);

	vec3 n = normalize( Normal_worldspace );
	vec3 l = normalize( LightDirection_worldspace );

	vec3 E = normalize(EyePosition_worldspace + EyeDirection_worldspace);
	vec3 H = normalize(l + E);

	float cos_theta = clamp(dot(n,l),0,1);
	float cos_alpha = clamp(dot(H,n),0,1);

	vec3 MaterialAmbientColor = Ka * vec3(0.2,0.2,0.2);

	vec3 MaterialDiffuseColor = texture(TexSampler, UV).rgb * LightColor * LightPower *cos_theta / distance ;
	vec3 MaterialSpecularColor = Ks * LightColor * LightPower * pow(cos_alpha, shineness) / distance ;

	color = MaterialAmbientColor * MaterialDiffuseColor +
			MaterialSpecularColor  +
			MaterialDiffuseColor;
}