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
out vec3 oFragPos;
out mat3 oTBN;

void main() {
	vec4 worldPos = model * vec4(pos, 1.0);
	gl_Position = proj * view * worldPos;
	
	oTexCoord = texCoord;
	oFragPos = worldPos.xyz;

	mat3 invModel = mat3(transpose(inverse(model)));
	oTBN = mat3(normalize(invModel * tangent), normalize(invModel * bitangent), normalize(invModel * normal));
}

@type fragment
#version 330 core

out vec4 FragColor;

in vec2 oTexCoord;
in vec3 oFragPos;
in mat3 oTBN;

uniform vec3 cameraPos;

uniform sampler2D u_Albedo;
uniform sampler2D u_Normal;

void main() {
	vec4 albedo = texture(u_Albedo, oTexCoord);
	albedo.rgb = pow(albedo.rgb, vec3(2.2));
	if(albedo.a < 0.5)
		discard;

	vec3 normal = texture(u_Normal, oTexCoord).rgb * 2.0 - 1.0;
	normal = normalize(oTBN * normal);

	float lightIntensity = 1000.0;
	vec3 lightPos = vec3(0, 5, -5);
	vec3 lightDir = lightPos - oFragPos;
	float dist = dot(lightDir, lightDir);
	lightDir = normalize(lightDir);
	vec3 cameraDir = normalize(cameraPos - oFragPos);

	float attenuation = 1.0 / dist;
	float diffuse = max(dot(lightDir, normal), 0.0);
	const float ambient = 0.01;
	float specular = pow(max(dot(cameraDir, reflect(-lightDir, normal)), 0.0), 128);
	vec3 color = albedo.rgb * attenuation * lightIntensity * (diffuse + specular) + ambient;
	
	FragColor = vec4(color, 1.0);
	FragColor.rgb = FragColor.rgb / (FragColor.rgb + 1.0);
	FragColor.rgb = pow(FragColor.rgb, vec3(1.0/2.2));
}