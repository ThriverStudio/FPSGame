@type vertex
#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

uniform mat4 proj;
uniform mat4 model;
uniform mat4 view;

out vec2 oTexCoord;
out vec3 oNormal;
out vec3 oFragPos;

void main() {
	vec4 worldPos = model * vec4(pos, 1.0);
	gl_Position = proj * view * worldPos;
	
	oTexCoord = texCoord;
	oNormal = normal;
	oFragPos = worldPos.xyz;
}

@type fragment
#version 330 core

out vec4 FragColor;

in vec2 oTexCoord;
in vec3 oNormal;
in vec3 oFragPos;

uniform vec3 cameraPos;

uniform sampler2D u_Albedo;

void main() {
	vec4 albedo = texture(u_Albedo, oTexCoord);
	albedo.rgb = pow(albedo.rgb, vec3(2.2));
	if(albedo.a < 0.5)
		discard;

	vec3 lightPos = vec3(0, 1, 1);
	vec3 lightDir = lightPos - oFragPos;
	float dist = dot(lightDir, lightDir);
	lightDir = normalize(lightDir);
	vec3 cameraDir = normalize(cameraPos - oFragPos);

	float attenuation = 1.0 / dist;
	float diffuse = max(dot(lightDir, oNormal), 0.0);
	const float ambient = 0.01;
	float specular = pow(max(dot(cameraDir, reflect(-lightDir, oNormal)), 0.0), 128);
	vec3 color = albedo.rgb * attenuation * (diffuse + specular) + ambient;
	
	FragColor = vec4(color, 1.0);
	FragColor.rgb = FragColor.rgb / (FragColor.rgb + 1.0);
	FragColor.rgb = pow(FragColor.rgb, vec3(1.0/2.2));
}