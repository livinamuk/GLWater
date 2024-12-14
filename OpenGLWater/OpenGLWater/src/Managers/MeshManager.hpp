#pragma once
#include "../API/OpenGL/Types/GL_detachedMesh.hpp"

namespace MeshManager {

    inline std::vector<OpenGLDetachedMesh> gMeshes;

    inline void CreateMesh(const std::string& name, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices) {
        OpenGLDetachedMesh& mesh = gMeshes.emplace_back();
        mesh.SetName(name);
        mesh.UpdateVertexBuffer(vertices, indices);
    }
    
    inline int GetMeshIndexByName(const std::string& name) {
        for (int i = 0; i < gMeshes.size(); i++) {
            if (gMeshes[i].GetName() == name)
                return i;
        }
        std::cout << "AssetManager::GetMeshIndexByName() failed because '" << name << "' does not exist\n";
        return -1;
    }

    inline OpenGLDetachedMesh* GetDetachedMeshByName(const std::string& name) {
        for (int i = 0; i < gMeshes.size(); i++) {
            if (gMeshes[i].GetName() == name)
                return &gMeshes[i];
        }
        std::cout << "AssetManager::GetDetachedMeshByName() failed because '" << name << "' does not exist\n";
        return nullptr;
    }

}