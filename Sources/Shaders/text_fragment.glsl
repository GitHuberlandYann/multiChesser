#version 150 core

in vec2 texCoords;
in vec4 Color;

out vec4 outColor;

uniform sampler2D asciiAtlas;

void main()
{
	outColor = texture(asciiAtlas, texCoords);
	if(outColor.a < 0.01) {
		discard;
	}
	outColor *= Color;
}
