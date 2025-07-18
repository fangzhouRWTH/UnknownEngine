#version 450

vec2 positions[3] = vec2[](
    vec2(-1.0, -1.0),
    vec2( 3.0, -1.0),
    vec2(-1.0,  3.0)
);

layout(location = 0) out vec2 vUV;

void main() {
    vec2 pos = positions[gl_VertexIndex];
    vUV = (pos + 1.0) * 0.5; // NDC 转 UV
    gl_Position = vec4(pos, 0.0, 1.0);
}