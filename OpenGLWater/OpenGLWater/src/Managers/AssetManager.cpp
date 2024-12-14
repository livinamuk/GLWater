#pragma once
#include "AssetManager.h"
#include "../File/AssimpImporter.h"
#include "../Managers/MeshManager.hpp"
#include "../Util.hpp"

#include "../Tools/DDSHelpers.h"
#include "../Tools/ImageTools.h"

namespace AssetManager {

    std::vector<Texture> g_textures;
    std::vector<Model> gModels;
    std::unordered_map<std::string, int> g_textureIndexMap;

    void ExportCustomFileFormats() {
        // Models
        auto modelPaths = Util::IterateDirectory("res/models/", { "obj", "fbx" });
        for (FileInfo& fileInfo : modelPaths) {
            std::string outputPath = MODEL_DIR + fileInfo.name + ".model";
            if (!Util::FileExists(outputPath)) {
                ModelData modelData = AssimpImporter::ImportFbx(fileInfo.path);
                File::ExportModel(modelData);
            }
        }
    }

    void LoadCustomFileFormats() {
        // Models
        auto modelPaths = Util::IterateDirectory(MODEL_DIR);
        for (FileInfo& fileInfo : modelPaths) {
            ModelData modelData = File::ImportModel(fileInfo.GetFileNameWithExtension());
            CreateModelFromData(modelData);
        }
    }

    void AddItemToLoadLog(std::string text) {
        static std::mutex logMutex;
        std::lock_guard<std::mutex> lock(logMutex);
        std::cout << text << "\n";
    }

    void LoadTextures() {
        // Find file paths
        auto texturePaths = std::filesystem::directory_iterator("res/textures/");
        for (const auto& entry : texturePaths) {
            FileInfoOld info = Util::GetFileInfo(entry);
            if (info.filetype == "png" || info.filetype == "jpg" || info.filetype == "tga") {
                g_textures.emplace_back(Texture(info.fullpath, true));
            }
        }
        // Async load them all
        std::vector<std::future<void>> futures;   
        for (Texture& texture : g_textures) {
            if (texture.GetLoadingState() == LoadingState::AWAITING_LOADING_FROM_DISK) {
                texture.SetLoadingState(LoadingState::LOADING_FROM_DISK);
                futures.push_back(std::async(std::launch::async, LoadTexture, &texture));
                AddItemToLoadLog("Loaded " + texture.m_fullPath);
            }
        }
        for (auto& future : futures) {
            future.get();
        }
        futures.clear();

        // Bake textures
        for (Texture& texture : g_textures) {
            texture.Bake();
        }
        // Build index maps
        for (int i = 0; i < g_textures.size(); i++) {
            g_textureIndexMap[g_textures[i].GetFilename()] = i;
        }
    }

    void Init() {

        ExportCustomFileFormats();
        LoadCustomFileFormats();
        LoadTextures();
    }

    void LoadTexture(Texture* texture) {
        texture->Load();
    }

    Texture* GetTextureByName(const std::string& name) {
        for (Texture& texture : g_textures) {
            if (texture.GetFilename() == name)
                return &texture;
        }
        std::cout << "AssetManager::GetTextureByName() failed because '" << name << "' does not exist\n";
        return nullptr;
    }

    int GetTextureIndexByName(const std::string& name) {
        for (int i = 0; i < g_textures.size(); i++) {
            if (g_textures[i].GetFilename() == name)
                return i;
        }
        std::cout << "AssetManager::GetTextureIndexByName() failed because '" << name << "' does not exist\n";
        return -1;
    }

    Texture* GetTextureByIndex(int index) {
        if (index != -1) {
            return &g_textures[index];
        }
        std::cout << "AssetManager::GetTextureByIndex() failed because index was -1\n";
        return nullptr;
    }


    Model* GetModelByIndex(int index) {
        if (index != -1) {
            return &gModels[index];
        }
        std::cout << "AssetManager::GetModelByIndex() failed because index was -1\n";
        return nullptr;
    }

    void CreateModelFromData(ModelData& modelData) {
        Model& model = gModels.emplace_back();
        model.m_name = modelData.name;
        for (MeshData& meshData : modelData.meshes) {
            OpenGLDetachedMesh& mesh = model.m_meshes.emplace_back();
            mesh.SetName(meshData.name);
            mesh.UpdateVertexBuffer(meshData.vertices, meshData.indices);
        }
    }
}




    



    

