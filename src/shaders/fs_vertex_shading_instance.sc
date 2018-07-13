$input v_color0, v_normal

#include "common.sh"

void main()
{
    vec3 sunDir = vec3(0.5, 0.5, -1);
    vec3 color = dot(v_normal, -sunDir) * v_color0.rgb;
	gl_FragColor = vec4(color, v_color0.a);
}
