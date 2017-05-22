#version 100
precision mediump float;

uniform lowp sampler2D texture;
uniform mediump float rnd; // TODO: merge this with intensity into a single vec2
uniform lowp float intensity;
uniform lowp float colorswap;

varying highp vec2 frag_texCoord;
varying lowp vec4 frag_color;

void main (void)
{
	gl_FragColor = texture2D(texture, frag_texCoord) * frag_color;
}
