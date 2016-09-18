#version 300 es
uniform highp float screenWidthDiv; // = 2.0 / screenWidth
uniform highp float screenHeightDiv; //  = 2.0 / screenHeight
uniform mediump float rnd;
uniform mediump float intensity;

layout(location = 0) in highp vec2 in_position;
layout(location = 1) in highp vec2 in_texCoord;
layout(location = 2) in lowp vec4 in_color;

out highp vec2 frag_texCoord;
out lowp vec4 frag_color;

float rand(in vec2 co){
	return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453);
}

void main(void)
{
	highp float f = rand(vec2(0.0f, in_texCoord.y + rnd));

	gl_Position = vec4(
		in_position.x * screenWidthDiv - 1.0f + f * 0.015f,
		1.0f - in_position.y * screenHeightDiv,
		0.0f, 1.0f);

	frag_texCoord = in_texCoord;
	frag_color = in_color;
}
