$input v_color0

#include "common.sh"

uniform vec4 u_color;

void main()
{
	gl_FragColor = v_color0 * (1.0 - u_color.a) + u_color;
}
