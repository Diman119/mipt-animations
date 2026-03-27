#pragma once
#include <map>
#include <memory>
#include <string>
#include <span>
#include <vector>

#include "3dmath.h"
#include "bone.h"


struct Mesh
{
  std::string name;
  const uint32_t vertexArrayBufferObject;
  const int numIndices;
  std::vector<Bone> skeleton;

  // Skeleton line rendering
  uint32_t skeletonVAO = 0;
  uint32_t skeletonVBO = 0;

  Mesh(const char *name, uint32_t vertexArrayBufferObject, int numIndices) :
    name(name),
    vertexArrayBufferObject(vertexArrayBufferObject),
    numIndices(numIndices)
    {}
};

using MeshPtr = std::shared_ptr<Mesh>;

MeshPtr create_mesh(
    const char *name,
    std::span<const uint32_t> indices,
    std::span<const vec3> vertices,
    std::span<const vec3> normals,
    std::span<const vec2> uv,
    std::span<const vec4> weights,
    std::span<const uvec4> weightsIndex);

MeshPtr create_mesh(
    const char *name,
    std::span<const uint32_t> indices,
    std::span<const vec3> vertices,
    std::span<const vec3> normals,
    std::span<const vec2> uv);

MeshPtr make_plane_mesh();

void render(const MeshPtr &mesh);
void render_skeleton(const MeshPtr &mesh, const mat4 &transform, const mat4 &cameraProjView);