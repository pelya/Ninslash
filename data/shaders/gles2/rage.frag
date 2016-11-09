#version 100
precision mediump float;

uniform lowp sampler2D texture;
uniform mediump float rnd; // TODO: merge this with intensity into a single vec2
uniform lowp float intensity;

varying highp vec2 frag_texCoord;
varying lowp vec4 frag_color;

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main (void)
{
	float f = rand(vec2(floor(gl_FragCoord.y/2.0), floor(gl_FragCoord.x/2.0)+rnd))*rand(vec2(floor(gl_FragCoord.y/2.0), floor(gl_FragCoord.x/2.0)+rnd));
	lowp vec4 t = texture2D(texture, frag_texCoord.st);
	float c1 = t.x + t.y + t.z;
	c1 *= 3.0;
	
	lowp vec4 color = vec4((1.0 - min(c1, 1.0)) * f, 0.0, 0.0, 0.0);
	
	gl_FragColor = t * frag_color + color*intensity;
}
