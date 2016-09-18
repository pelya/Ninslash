#version 100
uniform lowp sampler2D texture;
uniform mediump float rnd;
uniform lowp float intensity;

varying highp vec2 frag_texCoord;
varying lowp vec4 frag_color;

void main (void)
{
	gl_FragColor = texture2D(texture, frag_texCoord);
	// * frag_color;
}
