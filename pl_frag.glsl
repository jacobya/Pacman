#version 330

in vec3 fcolor;
out vec4 color_out;
in vec2 fuv;
uniform sampler2D tex;
void main() {
    color_out = vec4(fcolor, 1);
  //color_out = texture(tex,fuv);
}
