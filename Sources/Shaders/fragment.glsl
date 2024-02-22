#version 150 core

in vec3 texCoords;

out vec4 outColor;

uniform sampler2DArray pieces;

void main()
{
	// outColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
	outColor = texture(pieces, texCoords);
	if (outColor.a < 0.01) {
		discard ;
	}
}
