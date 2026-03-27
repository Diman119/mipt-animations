#include "scene.h"
#include "engine/render/mesh.h"
#include "engine/render/shader.h"
#include <vector>
#include <glad/glad.h>

// Skeleton shader for rendering bone lines
static ShaderPtr skeletonShader;

void init_skeleton_shader()
{
  if (!skeletonShader) {
    skeletonShader = compile_shader("skeleton",
      "/home/diman119/Projects/Animations2026/sources/shaders/vertex_color_vs.glsl",
      "/home/diman119/Projects/Animations2026/sources/shaders/vertex_color_ps.glsl");
  }
}

void render_character(const Character &character, const mat4 &cameraProjView, vec3 cameraPosition, const DirectionLight &light)
{
  const Material &material = *character.material;
  const Shader &shader = material.get_shader();

  shader.use();
  material.bind_uniforms_to_shader();
  shader.set_mat4x4("Transform", character.transform);
  shader.set_mat4x4("ViewProjection", cameraProjView);
  shader.set_vec3("CameraPosition", cameraPosition);
  shader.set_vec3("LightDirection", glm::normalize(light.lightDirection));
  shader.set_vec3("AmbientLight", light.ambient);
  shader.set_vec3("SunLight", light.lightColor);

  for (const MeshPtr &mesh : character.meshes) {
    render(mesh);
  }
  
  init_skeleton_shader();
  skeletonShader->use();
  skeletonShader->set_mat4x4("Transform", character.transform);
  skeletonShader->set_mat4x4("ViewProjection", cameraProjView);
  skeletonShader->set_vec3("Color", vec3(1.0f, 1.0f, 0.0f));

  for (const MeshPtr &mesh : character.meshes) {
    render_skeleton(mesh, character.transform, cameraProjView);
  }
}

void application_render(Scene &scene)
{
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_BLEND);
  const float grayColor = 0.3f;
  glClearColor(grayColor, grayColor, grayColor, 1.f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


  const mat4 &projection = scene.userCamera.projection;
  const glm::mat4 &transform = scene.userCamera.transform;
  mat4 projView = projection * inverse(transform);

  for (const Character &character : scene.characters)
    render_character(character, projView, glm::vec3(transform[3]), scene.light);
}