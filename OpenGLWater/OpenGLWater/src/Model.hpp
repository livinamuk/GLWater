#pragma once
#include "../API/OpenGL/Types/GL_detachedMesh.hpp"
#include <vector>

struct Model {

    std::string m_name;
    std::vector<OpenGLDetachedMesh> m_meshes;
};