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
	float f = rand(vec2(0.0, gl_FragCoord.x+rnd));

	lowp vec4 c = texture2D(texture, frag_texCoord.st);
	lowp vec4 color = vec4(1.0, 1.0-f*0.5, 1.0-f, c.w);

	lowp float r = c.r;
	lowp float b = c.b;
	c.r = r*(1.0-colorswap) + b*colorswap;
	c.b = b*(1.0-colorswap) + r*colorswap;
	
	gl_FragColor = c * frag_color + color*intensity*(1.0+rnd);
}
