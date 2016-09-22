#version 100
precision highp float;

uniform highp vec4 screenPos;

attribute highp vec2 in_position; // TODO: merge this with in_texCoord into a single vec4
attribute highp vec2 in_texCoord;
attribute lowp vec4 in_color;

varying highp vec2 frag_texCoord;
varying lowp vec4 frag_color;

void main(void)
{
	gl_Position = vec4(
		(in_position.x - screenPos.x) * screenPos.z,
		(screenPos.y - in_position.y) * screenPos.w,
		0.0, 1.0);

	frag_texCoord = in_texCoord;
	frag_color = in_color;
}
