#include "AssimpImporter.h"
#include "Common.h"
#include "../Util.hpp"
#include "../Texture.h"
#include "Importer.hpp"
#include "scene.h"
#include "postprocess.h"
#include "../Managers/MeshManager.hpp"
#include "../Model.hpp"

ModelData AssimpImporter::ImportFbx(const std::string filepath) {

    ModelData modelData;
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filepath,
        // aiProcess_GenSmoothNormals |
        // aiProcess_SortByPType |
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_ImproveCacheLocality |
        aiProcess_RemoveRedundantMaterials |
        aiProcess_FlipUVs
    );
    if (!scene) {
        std::cout << "LoadAndExportCustomFormat() failed to loaded model " << filepath << "\n";
        std::cerr << "Assimp Error: " << importer.GetErrorString() << "\n";
        return modelData;
    }

    modelData.name = Util::GetFileName(filepath);
    modelData.meshCount = scene->mNumMeshes;
    modelData.meshes.resize(modelData.meshCount);

    // Pre allocate vector memory
    for (int i = 0; i < modelData.meshes.size(); i++) {
        MeshData& meshData = modelData.meshes[i];
        meshData.vertexCount = scene->mMeshes[i]->mNumVertices;
        meshData.indexCount = scene->mMeshes[i]->mNumFaces * 3;
        meshData.name = scene->mMeshes[i]->mName.C_Str();
        meshData.vertices.resize(meshData.vertexCount);
        meshData.indices.resize(meshData.indexCount);
    }

    // Populate vectors
    for (int i = 0; i < modelData.meshes.size(); i++) {
        MeshData& meshData = modelData.meshes[i];
        const aiMesh* assimpMesh = scene->mMeshes[i];
        // Vertices
        for (unsigned int j = 0; j < meshData.vertexCount; j++) {
            meshData.vertices[j] = (Vertex{
                // Pos
                glm::vec3(assimpMesh->mVertices[j].x, assimpMesh->mVertices[j].y, assimpMesh->mVertices[j].z),
                // Normal
                glm::vec3(assimpMesh->mNormals[j].x, assimpMesh->mNormals[j].y, assimpMesh->mNormals[j].z),
                // UV
                assimpMesh->HasTextureCoords(0) ? glm::vec2(assimpMesh->mTextureCoords[0][j].x, assimpMesh->mTextureCoords[0][j].y) : glm::vec2(0.0f, 0.0f),
                // Tangent
                assimpMesh->HasTangentsAndBitangents() ? glm::vec3(assimpMesh->mTangents[j].x, assimpMesh->mTangents[j].y, assimpMesh->mTangents[j].z) : glm::vec3(0.0f)
                });
        }
        // Get indices
        for (unsigned int j = 0; j < assimpMesh->mNumFaces; j++) {
            const aiFace& face = assimpMesh->mFaces[j];
            unsigned int baseIndex = j * 3;
            meshData.indices[baseIndex] = face.mIndices[0];
            meshData.indices[baseIndex + 1] = face.mIndices[1];
            meshData.indices[baseIndex + 2] = face.mIndices[2];
        }
    }
    importer.FreeScene();
    return modelData;
}
