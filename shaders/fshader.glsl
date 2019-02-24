#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

uniform vec4 color;
uniform float width;

void main()
{
    gl_FragColor = color;
}
