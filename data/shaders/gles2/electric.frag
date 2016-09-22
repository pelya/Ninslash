#version 100

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
	float f = rand(vec2(0, frag_texCoord.y)) * rand(vec2(0, gl_FragCoord.y+rnd));// * max(0.2f, 1.0f - cs*0.5);

	lowp vec4 color = vec4(-f*0.5f, f*0.5f, f, 0);
	gl_FragColor = texture2D(texture, frag_texCoord.st) * frag_color + color*intensity;
}
