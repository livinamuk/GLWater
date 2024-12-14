#pragma once
#include "../Common/Types.h"
#include "../File/File.h"
#include "../Model.hpp"
#include "../Texture.h"
#include <string>


namespace AssetManager {

    void Init();

    void LoadTexture(Texture* texture);
    Texture* GetTextureByName(const std::string& name);
    int GetTextureIndexByName(const std::string& name);
    Texture* GetTextureByIndex(int index);

    void CreateModelFromData(ModelData& modelData);
    Model* GetModelByIndex(int index);


}