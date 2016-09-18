#version 100
uniform highp float screenWidthDiv; // = 2.0 / screenWidth
uniform highp float screenHeightDiv; //  = 2.0 / screenHeight

attribute highp vec2 in_position;
attribute highp vec2 in_texCoord;
attribute lowp vec4 in_color;

varying highp vec2 frag_texCoord;
varying lowp vec4 frag_color;

void main(void)
{
	gl_Position = vec4(
		in_position.x * screenWidthDiv - 1.0f,
		1.0f - in_position.y * screenHeightDiv,
		0.0f, 1.0f);

	frag_texCoord = in_texCoord;
	frag_color = in_color;
}
