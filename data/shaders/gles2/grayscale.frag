#version 100
precision mediump float;

uniform lowp sampler2D texture;
uniform mediump float rnd; // TODO: merge this with intensity into a single vec2
uniform lowp float intensity;

varying highp vec2 frag_texCoord;
varying lowp vec4 frag_color;

void main (void)
{
	lowp vec4 c = texture2D(texture, frag_texCoord);
	mediump float s = (c.r + c.g + c.b) / 3.0f;
	
	s = (s + (1.0f - intensity) + s * intensity) / 2.0f;
	
	gl_FragColor = vec4(s, s, s, c.a) * frag_color;
}
