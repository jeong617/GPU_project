#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoords;

int main()
{
	gl_Position = projection * view * mode * vec4(aPos, 1.0);
	gl_PointSize = gl_Position.z;
}