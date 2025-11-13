#include "../../include/Textures/Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../../include/ThirdParty/stb_image.h"

#include <iostream>

Texture::Texture(const std::string& path,
                 GLenum wrapS,
                 GLenum wrapT,
                 GLenum minFilter,
                 GLenum magFilter)
{
    stbi_set_flip_vertically_on_load(1);
    unsigned char* data = stbi_load(path.c_str(), &m_Width, &m_Height, &m_Channels, 0);
    if (!data) {
        std::cerr << "Failed to load texture: " << path << "\n";
        return;
    }

    // choose formats based on channels
    if (m_Channels == 1) {
        m_InternalFormat = m_DataFormat = GL_RED;
    } else if (m_Channels == 3) {
        m_InternalFormat = GL_RGB8;
        m_DataFormat = GL_RGB;
    } else if (m_Channels == 4) {
        m_InternalFormat = GL_RGBA8;
        m_DataFormat = GL_RGBA;
    } else {
        // fallback
        m_InternalFormat = GL_RGBA8;
        m_DataFormat = GL_RGBA;
    }

    glGenTextures(1, &m_ID);
    glBindTexture(GL_TEXTURE_2D, m_ID);

    // upload
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, m_InternalFormat, m_Width, m_Height, 0, m_DataFormat, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    // params
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(data);
}

Texture::~Texture()
{
    if (m_ID) {
        glDeleteTextures(1, &m_ID);
        m_ID = 0;
    }
}

void Texture::Bind(GLenum unit) const
{
    glActiveTexture(unit);
    glBindTexture(GL_TEXTURE_2D, m_ID);
}

void Texture::Unbind() const
{
    glBindTexture(GL_TEXTURE_2D, 0);
}