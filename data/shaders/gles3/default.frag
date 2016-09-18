#version 300 es
uniform lowp sampler2D texunit;
uniform mediump float rnd;
uniform lowp float intensity;

in highp vec2 frag_texCoord;
in lowp vec4 frag_color;

layout(location = 0) out lowp vec4 out_color;

void main (void)
{
	out_color = texture(texunit, frag_texCoord) * frag_color;
}
