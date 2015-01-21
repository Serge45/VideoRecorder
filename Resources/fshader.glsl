#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

uniform sampler2D texture;

varying vec2 v_texcoord;
varying float z_coord;

void main()
{
    // Set fragment color from texture
    float alpha = 2.f - abs(-1 - z_coord);
    alpha /= 2.f;
    vec4 fragment = texture2D(texture, v_texcoord);
    //fragment *= alpha;
    gl_FragColor = fragment;
}

