$input v_texcoord0

#include "bgfx_shader.sh"
#include "shaderlib.sh"

SAMPLER2D(s_texColor, 0);

void main()
{
    // Sample the color texture
    vec4 color = toLinear(texture2D(s_texColor, v_texcoord0));

    // Output the color directly
    gl_FragColor.xyz = color.xyz;
    gl_FragColor.w = color.a;
    gl_FragColor = toGamma(gl_FragColor);

//    gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}