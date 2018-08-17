$input a_position, a_normal, i_data0, i_data1, i_data2, i_data3, i_data4
$output v_position, v_normal, v_color0

#include "common.sh"

void main()
{
    mat4 model;
	model[0] = i_data0;
	model[1] = i_data1;
	model[2] = i_data2;
	model[3] = i_data3;
    v_color0 = i_data4;
    
    vec4 worldPos = instMul(model, vec4(a_position, 1.0));
    vec3 worldNormal = normalize(instMul(model, vec4(a_normal, 0.0)).xyz);
	gl_Position = mul(u_viewProj, worldPos);
    
	v_normal = worldNormal;
	v_position = worldPos;
}
