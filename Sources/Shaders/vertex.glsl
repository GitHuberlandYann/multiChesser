#version 150 core

in ivec2 position;

uniform int win_width;
uniform int win_height;

void main()
{
	gl_Position = vec4((2.0 * position.x) / win_width - 1.0, -((2.0 * position.y) / win_height - 1.0), 0.0, 1.0);
}
