#version 460 core
out vec4 FragColor;

in vec2 vTexCoord;
in vec3 vNormal;
in vec3 vWorldPos;

uniform sampler2D diffuse_texture;
uniform sampler2D specular_texture;

uniform mat4 view;
uniform vec3 uCamPos;
uniform vec3 uDirectionalLight;

void main()
{
    float ambientStrength = 0.1;
    vec3 lightColor = vec3(0.9,0.95,1.0);
    vec3 lightPos = vec3(50.0,50.0,50.0);
    vec3 ambient = ambientStrength * lightColor;

    vec3 norm = normalize(vNormal);
    //vec3 lightDir = normalize(lightPos-vWorldPos);
    vec3 lightDir = -normalize(uDirectionalLight);
    float diff = max(dot(norm,lightDir),0.0);
    vec3 diffuse = diff * lightColor;

    float specularStrength = texture(specular_texture,vTexCoord).x;
    vec3 viewDir = normalize(uCamPos - vWorldPos);
    vec3 reflectionDir = reflect(-lightDir,norm);

    float spec = pow(max(dot(viewDir,reflectionDir),0.0),32);
    vec3 specular = specularStrength * spec * lightColor;

    vec4 baseColor = texture(diffuse_texture,vTexCoord);
    vec4 result = baseColor * vec4((ambient + diffuse + specular),1.0);
    FragColor = result;
}