// Fragment Shader – file "simple.frag"

#version 330

precision highp float; // needed only for version 1.30

in  vec4 ex_Color;
out vec4 out_Color;

void main(void) {
	out_Color = ex_Color;
}
