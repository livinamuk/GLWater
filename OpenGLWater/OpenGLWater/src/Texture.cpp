#include "Texture.h"
#include "../Util.hpp"

Texture::Texture(std::string fullpath, bool compressed) {
    m_compressed = compressed;
    m_fullPath = fullpath;
    m_fileName = Util::GetFilename(m_fullPath);
    m_fileType = Util::GetFileInfo(m_fullPath).filetype;
}

void Texture::Load() {
    glTexture.Load(m_fullPath, m_compressed);
}

void Texture::Bake() {
    if (m_bakingState == BakingState::AWAITING_BAKE) {
        glTexture.Bake();
    }
    m_bakingState = BakingState::BAKE_COMPLETE;
}

void Texture::BakeCMPData(CMP_Texture* cmpTexture) {
    if (m_bakingState == BakingState::AWAITING_BAKE) {
        glTexture.BakeCMPData(cmpTexture);
    }
    m_bakingState = BakingState::BAKE_COMPLETE;
}

int Texture::GetWidth() {
    return glTexture.GetWidth();
}

int Texture::GetHeight() {
    return glTexture.GetHeight();
}

std::string& Texture::GetFilename() {
    return m_fileName;
}

std::string& Texture::GetFiletype() {
    return m_fileType;
}

OpenGLTexture& Texture::GetGLTexture() {
    return glTexture;
}


void Texture::SetLoadingState(LoadingState loadingState) {
    m_loadingState = loadingState;
}

const LoadingState Texture::GetLoadingState() {
    return m_loadingState;
}

const BakingState Texture::GetBakingState() {
    return m_bakingState;
}