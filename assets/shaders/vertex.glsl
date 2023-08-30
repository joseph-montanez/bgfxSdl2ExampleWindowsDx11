$input a_position, a_texcoord0
$output v_texcoord0

#include "bgfx_shader.sh"
#include "shaderlib.sh"

void main()
{
    vec3 wpos = mul(u_model[0], vec4(a_position, 1.0)).xyz;

    gl_Position = mul(u_viewProj, vec4(wpos, 1.0));

    v_texcoord0 = a_texcoord0;
}
