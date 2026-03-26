#version 450

struct VsOutput
{
  vec3 VertexColor;
};

uniform mat4 Transform;
uniform mat4 ViewProjection;

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec2 UV;
layout(location = 3) in vec4 BoneWeights;
layout(location = 4) in uvec4 BoneIndex;


vec3 randomColor(float seed) {
  vec3 k = vec3(53758.5453, 22578.1234, 38965.9871);
  return fract(sin(seed * k) * 43758.5453);
}


out VsOutput vsOutput;

void main()
{
  vec3 VertexPosition = (Transform * vec4(Position, 1)).xyz;
  gl_Position = ViewProjection * vec4(VertexPosition, 1);

  vsOutput.VertexColor = BoneWeights.x * randomColor(BoneIndex.x) +
                         BoneWeights.y * randomColor(BoneIndex.y) +
                         BoneWeights.z * randomColor(BoneIndex.z) +
                         BoneWeights.w * randomColor(BoneIndex.w);
}