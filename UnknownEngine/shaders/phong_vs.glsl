#version 460 core
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
//out vec3 ourColor;
out vec2 vTexCoord;
out vec3 vNormal;
out vec3 vWorldPos;
//out vec3 vViewPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    //gl_Position = vec4(aPos, 1.0);
    vec4 w_pos = model * vec4(aPosition,1.0);
    gl_Position = projection * view * w_pos;
    vWorldPos = w_pos.xyz;
    vTexCoord = aTexCoord;
    vNormal = aNormal;
    //vViewPos = view[3].xyz;
}