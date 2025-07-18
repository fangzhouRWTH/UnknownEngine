#version 450

layout(location = 0) in vec2 vUV;

layout(set = 0, binding = 0) uniform UBO {
  mat4 view;
  mat4 proj;
  mat4 view_proj;

  mat4 view_inv;
  mat4 proj_inv;

  vec4 color1;
  vec4 color2;
  vec4 color3;
  vec4 color4;

  float cull_center_x;
  float cull_center_y;
  float cull_radius;
  float meshlet_density;
} ubo;

//layout(set = 0, binding = 1) uniform samplerCube uSkybox;

layout(location = 0) out vec4 outColor;

void main() {
    vec2 ndc = vUV * 2.0 - 1.0;
    vec4 clip = vec4(ndc, 1.0, 1.0); // z=1 对应 far plane

    vec4 viewPos = ubo.proj_inv * clip;
    viewPos /= viewPos.w;
    vec4 worldPos = ubo.view_inv * viewPos;

    vec3 viewRay = normalize(worldPos.xyz); // viewRay 即 camera → 方向
    //outColor = texture(uSkybox, viewRay);
    outColor = vec4(viewRay,1.0);
}