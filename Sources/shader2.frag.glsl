#version 450

uniform sampler2D tex;

in vec3 position;
in vec2 texCoord;
in vec3 normal;

out vec4 frag;

void main() {
	frag = vec4(((texture(tex, texCoord) * (0.8 + max(dot(vec3(0, 1, 0), normal), 0.0) * 0.2)).xyz), 1);
}
