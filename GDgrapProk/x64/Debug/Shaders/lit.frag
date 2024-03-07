#version 330 core
//full fledged point light

in vec2 texCoord;
in vec3 normCoord;
in vec3 fragPos;

uniform sampler2D tex0;

//SKYBOX AMBIENT LIGHT
uniform samplerCube skybox;

//DIRECTIONAL LIGHT
uniform vec3 dLightDir;
uniform vec3 dLightColor;

//POINT LIGHT
uniform vec3 pLightPos;
//COLOR defines point light BRIGHTNESS
uniform vec3 pLightColor;

//AMBIENT LIGHT
uniform vec3 ambientLightColor;
uniform float ambientLightIntensity;

//CAMERA
uniform vec3 cameraPos;

//SPEC LIGHTING
uniform float specStrength;
uniform float specPhong;

//OUT
out vec4 FragColor;

void main(){
	//Directional Vectors
	vec3 normal = normalize(normCoord);
	vec3 viewDir = normalize(cameraPos - fragPos);

	//Directional light set-up
	vec3 dirLightDir = normalize(dLightDir);
	vec3 dDiffuse = max(dot(normal, dirLightDir), 0.0f) * dLightColor;

	//Point light set-up + intensity calculation from distance
	vec3 pLightOffset = pLightPos - fragPos;
	vec3 pLightDir = normalize(pLightOffset);
	float pLightIntensity = min(1 / (pow(pLightOffset.x, 2) + pow(pLightOffset.y, 2)), 1);
	vec3 pLightFinalColor = pLightColor * pLightIntensity;
	vec3 pDiffuse = max(dot(normal, pLightDir), 0.0f) * pLightFinalColor;

	//Diffuse calculation
	vec3 diffuse = dDiffuse + pDiffuse;

	//Ambient calculation
	vec3 skyboxReflectDir = reflect(-viewDir, normal);
	vec4 ambientColor = texture(skybox, skyboxReflectDir);
	vec3 ambient = vec3(ambientColor.x, ambientColor.y, ambientColor.z) * ambientLightIntensity;

	//Spec lighting calculation
	vec3 reflectDir = reflect(-pLightDir, normal);
	float spec = pow(max(dot(reflectDir, viewDir), 0.1), specPhong);
	vec3 specColor = spec * specStrength * pLightFinalColor;

	//Combine all light sources
	FragColor = vec4(diffuse + ambient + specColor, 1.0f) * max(texture(tex0, texCoord), 0.2);
	//FragColor = vec4(normal, 1.0f);
}