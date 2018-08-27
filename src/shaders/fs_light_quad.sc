$input v_texcoord0

#include "common.sh"

SAMPLER2D(s_position, 1);
SAMPLER2D(s_normal, 2);

uniform vec4 u_lightPos;
uniform vec4 u_lightColor;
uniform vec4 u_lightLinearQuadraticIntensity;

void main()
{
    vec3 color = u_lightColor.xyz;
    vec3 position = texture2D(s_position, v_texcoord0).xyz;
    vec4 texNormal = texture2D(s_normal, v_texcoord0);
    vec3 normal = texNormal.xyz * 2.0 - 1.0 * texNormal.w;
    vec3 lightDir = normalize(u_lightPos.xyz - position);
    float distance = length(u_lightPos.xyz - position);
    float linear_ = u_lightLinearQuadraticIntensity.x;
    float quadratic = u_lightLinearQuadraticIntensity.y;
    float intensity = u_lightLinearQuadraticIntensity.z;
    
    float light = (max(dot(normal, lightDir), 0.0) * intensity) / (1.0 + linear_ * distance + quadratic * (distance * distance));
    gl_FragColor = vec4(color * light, 1.0);
    //gl_FragColor = vec4(color, 1.0);
}
