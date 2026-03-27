#version 450

struct VsOutput
{
  vec3 VertexColor;
};

in VsOutput vsOutput;
out vec4 FragColor;

void main()
{
  FragColor = vec4(vsOutput.VertexColor, 1.0);
}