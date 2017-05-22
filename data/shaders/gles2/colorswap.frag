#version 100
precision mediump float;

uniform lowp sampler2D texture;
uniform mediump float rnd; // TODO: merge this with intensity into a single vec2
uniform lowp float intensity;
uniform highp vec4 screenPos;
uniform lowp float colorswap;

varying highp vec2 frag_texCoord;
varying lowp vec4 frag_color;

void main (void)
{
	lowp float r = texture2D(texture, frag_texCoord.st).r;
	lowp float g = texture2D(texture, frag_texCoord.st).g;
	lowp float b = texture2D(texture, frag_texCoord.st).b;
	lowp float a = texture2D(texture, frag_texCoord.st).a;
	
	gl_FragColor = vec4(b, g, r, a) * frag_color;
}
