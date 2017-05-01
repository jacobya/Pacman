#version 330

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

in vec3 position;
in vec3 color;
in vec2 uv;
in vec4 normal;
out vec3 fcolor;
out vec2 fuv;

out vec4 pos;
out vec4 norm;
void main() {
      norm = transpose(inverse(view)*inverse(model))*normal;
      pos =  (model)  * vec4(position, 1);
  gl_Position = projection * view * model * vec4(position, 1);
  fcolor = color;
  fuv = uv;
}
