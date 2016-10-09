#version 100
precision mediump float;

uniform lowp sampler2D texture;
uniform mediump float rnd; // TODO: merge this with intensity into a single vec2
uniform lowp float intensity;
uniform highp vec4 screenPos;

varying highp vec2 frag_texCoord;
varying lowp vec4 frag_color;

void main (void)
{
	lowp vec4 t = texture2D(texture, frag_texCoord);
	highp float StepY = screenPos.w;
	mediump float SumGreen = (texture2D(texture, frag_texCoord + vec2(0, +StepY)).g + t.g)/2.0;
	
	mediump float g = SumGreen * 0.7;
	
	// get alpha
	float a = step(0.7, t.g);
	
	gl_FragColor = vec4(0, g * frag_color.g, 0, a * frag_color.a);
}