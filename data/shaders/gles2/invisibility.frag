#version 100
precision mediump float;

uniform lowp sampler2D texture;
uniform mediump float rnd; // TODO: merge this with intensity into a single vec2
uniform lowp float intensity;
uniform lowp float colorswap;

varying highp vec2 frag_texCoord;
varying lowp vec4 frag_color;

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main (void)
{
	float f = rand(vec2(0.0, frag_texCoord.y)) * rand(vec2(0.0, frag_texCoord.y+rnd));// * max(0.2f, 1.0f - cs*0.5);

	vec4 c = texture2D(texture, frag_texCoord.st) * frag_color;
	c *= 1.0-intensity+rnd*0.1;
	
	lowp float r = c.r;
	lowp float b = c.b;
	c.r = r*(1.0-colorswap) + b*colorswap;
	c.b = b*(1.0-colorswap) + r*colorswap;
	
	lowp vec4 color = vec4(-f, f, -f, 0.0);
	gl_FragColor = c + color*intensity;
}
