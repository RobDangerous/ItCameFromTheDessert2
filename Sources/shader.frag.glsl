#version 450

#ifdef GL_ES
precision mediump float;
#endif

uniform sampler2D tex;

in vec2 texCoord;

out vec4 FragColor;

void kore() {
	FragColor = texture(tex, texCoord);
}
