$input v_color0, v_normal

#include "common.sh"

uniform vec4 u_color;

void main()
{
    vec3 sunDir = vec3(0.5, 0.5, -1);
    vec3 color = dot(v_normal, -sunDir) * u_color.rgb;
	gl_FragColor = vec4(color, u_color.a);
}
