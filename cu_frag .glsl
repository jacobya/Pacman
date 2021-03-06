#version 330

uniform sampler2D tex;
uniform float alpha;
in vec3 fcolor;
in vec2 fuv;
in vec4 pos;
in vec4 norm;
out vec4 color_out;


uniform vec3 light;
uniform float shine;
uniform float amb;
uniform vec3 cameraLoc;
uniform mat4 view;
void main() {

    vec3 L;
    vec3 N = normalize(norm.xyz);
    vec3 cam = (inverse(view) * vec4(0,0,0,1)).xyz;
    vec3 lightP = cam;//(view * vec4(light,1)).xyz;

    L = normalize(lightP - pos.xyz);

    float diffuse = clamp(dot(N,L),0,1);

    float specular = 0.0;

    vec3 reflection = reflect(-L, N);
    vec3 surface = normalize(cam-N);
    float angle=clamp(dot(surface,reflection),0,1);
    specular = pow(angle,shine);

    color_out = vec4((diffuse+specular)*texture(tex,fuv).bgr, alpha);
}
