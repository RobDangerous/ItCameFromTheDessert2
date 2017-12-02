#version 450

#ifdef GL_ES
precision mediump float;
#endif

uniform sampler2D tex;

uniform vec3 diffuseCol;
uniform vec3 specularCol;
uniform float specularPow;

#define MAX_LIGHTS 10
uniform int numLights;
uniform vec4 lightPos[MAX_LIGHTS];

in vec2 texCoord;
in vec3 normal;
//in vec3 lightDirection;
in vec3 eyeCoord;

out vec4 FragColor;

void applyLight(vec4 lightPosition, out vec3 ambientOut, out vec3 diffuseOut, out vec3 specularOut) {
	
	vec3 lightDirection;
	float attenuation = 1.0;
	
	if (lightPosition.w == 0.0) {
		// Spot light
		lightDirection = normalize(lightPosition.xyz - eyeCoord);
		
		float distanceToLight = length(lightPosition.xyz - eyeCoord);
		float lightAttenuation = 0.1;
		attenuation = 1.0 / (1.0 + lightAttenuation * pow(distanceToLight, 2));
		
		// Cone restrictions (affects attenuation)
		vec3 coneDirection = vec3(0, -1, 0);
		float coneAngle = 15.0;
		float lightToSurfaceAngle = degrees(acos(dot(-lightDirection, normalize(coneDirection))));
		if(lightToSurfaceAngle > coneAngle){
			attenuation = 0.0;
		}
		
	} else {
		// Directional light
		lightDirection = normalize(lightPosition.xyz);
		attenuation = 1.0; // No attenuation for directional lights
	}
	
	// Ambient
	const float amb = 0.3;
	vec3 ambient = vec3(amb);
	
	// Diffuse
	vec3 diffuse = max(dot(lightDirection, normal), 0.0) * vec3(diffuseCol);
	
	// Specular
	vec3 halfVector = normalize(lightDirection - normalize(eyeCoord));
	vec3 specular = pow(max(0.0, dot(halfVector, reflect(-lightDirection, normal))), specularPow) * vec3(specularCol);
	
	ambientOut = ambient;
	diffuseOut = attenuation * diffuse;
	specularOut = attenuation * specular;
}

void main() {
	vec3 finalAmbient = vec3(0, 0, 0);
	vec3 finalDiffuse = vec3(0, 0, 0);
	vec3 finalSpecular = vec3(0, 0, 0);
	vec3 diffuse = vec3(0, 0, 0);
	for (int i = 0; i < numLights; ++i) {
		vec3 ambient, diffuse, specular;
		applyLight(lightPos[i], ambient, diffuse, specular);
		finalAmbient += ambient;
		finalDiffuse += diffuse;
		finalSpecular += specular;
	}
	
	FragColor = vec4((finalAmbient + finalDiffuse) * texture(tex, texCoord).rgb + finalSpecular, 1.0);
}
