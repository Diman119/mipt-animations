#include "render/mesh.h"
#include "render/bone.h"
#include <vector>
#include <map>
#include <3dmath.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include "engine/api.h"
#include "glad/glad.h"

#include "import/model.h"

void assimp_matrix_to_glm(const aiMatrix4x4& from, mat4x4& to) {
  // Transpose during conversion (row-major to column-major)
  to[0][0] = from.a1; to[0][1] = from.b1; to[0][2] = from.c1; to[0][3] = from.d1;
  to[1][0] = from.a2; to[1][1] = from.b2; to[1][2] = from.c2; to[1][3] = from.d2;
  to[2][0] = from.a3; to[2][1] = from.b3; to[2][2] = from.c3; to[2][3] = from.d3;
  to[3][0] = from.a4; to[3][1] = from.b4; to[3][2] = from.c4; to[3][3] = from.d4;
}

void validate_skeleton_hierarchy(const std::vector<Bone> &skeleton)
{
  for (size_t i = 0; i < skeleton.size(); i++)
  {
    int parent_idx = skeleton[i].parent_idx;
    if (parent_idx >= 0 && parent_idx >= static_cast<int>(i))
    {
      engine::warning("Bone '%s' has parent '%s' at index %d, but parent should come before child (index %d)",
                      skeleton[i].name.c_str(), skeleton[parent_idx].name.c_str(), parent_idx, static_cast<int>(i));
    }
  }
}

std::vector<Bone> create_skeleton(const aiMesh *mesh, const aiScene *scene)
{
  std::vector<Bone> skeleton;

  if (!mesh->HasBones())
    return skeleton;

  int numBones = mesh->mNumBones;
  skeleton.resize(numBones);

  // Map bone name to index
  std::map<std::string, int> bone_name_to_idx;
  for (int i = 0; i < numBones; i++)
  {
    const aiBone *bone = mesh->mBones[i];
    bone_name_to_idx[std::string(bone->mName.C_Str())] = i;

    skeleton[i].name = std::string(bone->mName.C_Str());
    assimp_matrix_to_glm(bone->mOffsetMatrix, skeleton[i].transform);
  }

  // Build hierarchy of nodes from scene root
  // For each bone, find corresponding node in the scene hierarchy and set parent index
  if (scene && scene->mRootNode) {
    // Collect all nodes that correspond to bones
    std::vector<aiNode *> bone_nodes;
    bone_nodes.reserve(numBones);
    for (int i = 0; i < numBones; i++) {
      const aiBone *bone = mesh->mBones[i];
      aiNode *node = scene->mRootNode->FindNode(aiString(bone->mName));
      if (node) {
        bone_nodes.push_back(node);
      }
    }

    // For each bone node, find its parent in the bone_nodes list
    for (size_t i = 0; i < bone_nodes.size(); i++) {
      aiNode *node = bone_nodes[i];
      int parent_idx = -1;

      // Find parent node in the hierarchy
      aiNode *current = node->mParent;
      while (current) {
        // Check if parent is one of the bones in this mesh
        auto it = bone_name_to_idx.find(std::string(current->mName.C_Str()));
        if (it != bone_name_to_idx.end()) {
          parent_idx = it->second;
          break;
        }
        current = current->mParent;
      }

      skeleton[i].parent_idx = parent_idx;
    }
  }

  validate_skeleton_hierarchy(skeleton);

  return skeleton;
}

MeshPtr create_mesh(const aiMesh *mesh, const aiScene *scene)
{
  std::vector<uint32_t> indices;
  std::vector<vec3> vertices;
  std::vector<vec3> normals;
  std::vector<vec2> uv;
  std::vector<vec4> weights;
  std::vector<uvec4> weightsIndex;

  int numVert = mesh->mNumVertices;
  int numFaces = mesh->mNumFaces;

  if (mesh->HasFaces())
  {
    indices.resize(numFaces * 3);
    for (int i = 0; i < numFaces; i++)
    {
      assert(mesh->mFaces[i].mNumIndices == 3);
      for (int j = 0; j < 3; j++)
        indices[i * 3 + j] = mesh->mFaces[i].mIndices[j];
    }
  }

  if (mesh->HasPositions())
  {
    vertices.resize(numVert);
    for (int i = 0; i < numVert; i++)
      vertices[i] = to_vec3(mesh->mVertices[i]);
  }

  if (mesh->HasNormals())
  {
    normals.resize(numVert);
    for (int i = 0; i < numVert; i++)
      normals[i] = to_vec3(mesh->mNormals[i]);
  }

  if (mesh->HasTextureCoords(0))
  {
    uv.resize(numVert);
    for (int i = 0; i < numVert; i++)
      uv[i] = to_vec2(mesh->mTextureCoords[0][i]);
  }

  std::vector<Bone> skeleton = create_skeleton(mesh, scene);

  if (mesh->HasBones())
  {
    weights.resize(numVert, vec4(0.f));
    weightsIndex.resize(numVert);
    int numBones = mesh->mNumBones;
    std::vector<int> weightsOffset(numVert, 0);

    for (int i = 0; i < numBones; i++)
    {
      const aiBone *bone = mesh->mBones[i];

      for (unsigned j = 0; j < bone->mNumWeights; j++)
      {
        int vertex = bone->mWeights[j].mVertexId;
        int offset = weightsOffset[vertex]++;
        weights[vertex][offset] = bone->mWeights[j].mWeight;
        weightsIndex[vertex][offset] = i;
      }
    }

    // the sum of weights not 1
    for (int i = 0; i < numVert; i++)
    {
      vec4 w = weights[i];
      float s = w.x + w.y + w.z + w.w;
      weights[i] *= 1.f / s;
    }
  }

  auto mesh_ptr = create_mesh(mesh->mName.C_Str(), indices, vertices, normals, uv, weights, weightsIndex);
  mesh_ptr->skeleton = std::move(skeleton);
  return mesh_ptr;
}

ModelAsset load_model(const char *path)
{

  Assimp::Importer importer;
  // importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
  importer.SetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, 1.f);

  importer.ReadFile(path, aiProcess_Triangulate | aiProcess_LimitBoneWeights |
                              aiProcess_GenNormals | aiProcess_GlobalScale | aiProcess_FlipWindingOrder | aiProcess_PopulateArmatureData);

  const aiScene *scene = importer.GetScene();
  ModelAsset model;
  model.path = path;
  if (!scene)
  {
    engine::error("Filed to read model file \"%s\"", path);
    return model;
  }

  model.meshes.resize(scene->mNumMeshes);
  for (uint32_t i = 0; i < scene->mNumMeshes; i++)
  {
    model.meshes[i] = create_mesh(scene->mMeshes[i], scene);
  }

  engine::log("Model \"%s\" loaded", path);
  return model;
}