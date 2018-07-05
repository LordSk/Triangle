$input a_position, a_color0, a_normal, i_data0, i_data1, i_data2, i_data3, i_data4
$output v_color0, v_normal

#include "common.sh"

void main()
{
    mat4 model;
	model[0] = i_data0;
	model[1] = i_data1;
	model[2] = i_data2;
	model[3] = i_data3;
    
    vec4 worldPos = instMul(model, vec4(a_position, 1.0) );
	gl_Position = mul(u_viewProj, worldPos);
	v_color0 = i_data4;
	v_normal = normalize(mul(model, vec4(a_normal, 0.0)).xyz);
}
