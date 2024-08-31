#version 460 core
out vec4 FragColor;

in vec2 TexCoord;
uniform sampler2D texture0;
uniform sampler2D texture1;
void main()
{
    FragColor = texture(texture0, TexCoord) * 0.5f + texture(texture1, TexCoord) * 0.5f;
}