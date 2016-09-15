uniform highp float screenWidthDiv; // = 2.0 / screenWidth
uniform highp float screenHeightDiv; //  = 2.0 / screenHeight

layout(location = 0) in highp vec2 in_position;
layout(location = 1) in highp vec2 in_texCoord;
layout(location = 2) in lowp vec4 in_color;

out highp vec2 frag_texCoord;
out lowp vec4 frag_color;

void main(void)
{
	gl_Position = vec4(
		in_position.x * screenWidthDiv - 1.0f,
		1.0f - in_position.y * screenHeightDiv,
		0.0f, 1.0f);
	frag_texCoord = in_texCoord;
	frag_color = in_color;
}
