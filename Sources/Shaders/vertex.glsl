#version 150 core

/*
 * specifications is packed
 * 0x1 x offset
 * 0x2 y offset
 * rest layer in texture_2D_array
 */
in int specifications;
in ivec2 position;

uniform int win_width;
uniform int win_height;

out vec3 texCoords;

// half pixel correction to get correct location of texel
// equal to 0.5 / texSize, in this case textures are 300x300 so 0.5 / 300
const float half_pxl = 0.0016666666666666668;

void main()
{
	gl_Position = vec4((2.0 * position.x) / win_width - 1.0, -((2.0 * position.y) / win_height - 1.0), 0.0, 1.0);
	texCoords = vec3(((specifications & 0x1) == 0x1) ? 1.0f - half_pxl : half_pxl,
				((specifications & 0x2) == 0x2) ? 1.0f - half_pxl : half_pxl, specifications >> 2);
}
