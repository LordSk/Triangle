$input a_position, a_normal, a_texcoord0
$output v_position, v_normal, v_color0, v_texcoord0

#include "common.sh"

uniform vec4 u_color;

void main()
{
	gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
    v_position = mul(u_model[0], vec4(a_position, 1.0));
	v_normal = normalize(mul(u_model[0], vec4(a_normal, 0.0)).xyz);
    v_color0 = u_color;
    v_texcoord0 = a_texcoord0;
}
