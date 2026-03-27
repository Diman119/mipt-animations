#version 450

struct VsOutput
{
  vec3 VertexColor;
};

uniform mat4 Transform;
uniform mat4 ViewProjection;
uniform vec3 Color;

layout(location = 0) in vec3 Position;

out VsOutput vsOutput;

void main()
{
  vec3 VertexPosition = (Transform * vec4(Position, 1)).xyz;
  gl_Position = ViewProjection * vec4(VertexPosition, 1);

  vsOutput.VertexColor = Color;
}