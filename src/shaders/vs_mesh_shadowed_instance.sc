$input a_position, a_normal, i_data0, i_data1, i_data2, i_data3, i_data4
$output v_position, v_normal, v_shadowcoord, v_color0

#include "common.sh"

uniform mat4 u_lightMtx;

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

	const float shadowMapOffset = 0.001;
	vec3 posOffset = a_position + a_normal.xyz * shadowMapOffset;
    vec4 worldPosOffset = instMul(model, vec4(posOffset, 1.0));
	v_shadowcoord = mul(u_lightMtx, worldPosOffset);
}
