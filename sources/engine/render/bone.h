#pragma once
#include "glm/mat4x4.hpp"

struct Bone
{
    std::string name;
    glm::mat4x4 transform;
    int32_t parent_idx;
};
