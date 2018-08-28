$input v_texcoord0

#include "common.sh"

SAMPLER2D(s_position, 1);
SAMPLER2D(s_normal, 2);

uniform vec4 u_lightPos;
uniform vec4 u_lightColor1;
uniform vec4 u_lightColor2;
uniform vec4 u_lightParams;

void main()
{
    vec3 color1 = u_lightColor1.xyz;
    vec3 color2 = u_lightColor2.xyz;
    vec3 position = texture2D(s_position, v_texcoord0).xyz;
    vec4 texNormal = texture2D(s_normal, v_texcoord0);
    vec3 normal = texNormal.xyz * 2.0 - 1.0 * texNormal.w;
    vec3 lightDir = normalize(u_lightPos.xyz - position);
    float distance = length(u_lightPos.xyz - position);
    float intensity = u_lightParams.x;
    float radius = u_lightParams.y;
    float slope = u_lightParams.z * 2.0 - 1.0;
    float falloff = u_lightParams.w;
    
    float attenuation = pow(max((radius - distance), 0.0) / radius, falloff);
    float a = clamp(attenuation - slope, 0.0, 1.0);
    vec3 colorMixed = color1 * a + color2 * (1.0 - a);
    float shadowed = max(dot(normal, lightDir), 0.0);
    gl_FragColor = vec4(colorMixed * attenuation * intensity * shadowed, 1.0);
}
