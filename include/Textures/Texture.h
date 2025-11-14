#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>
#include <glad/glad.h>

class Texture
{
public:
    // load from file; format and wrap/filter are optional
    Texture(const std::string& path,
            GLenum wrapS = GL_REPEAT,
            GLenum wrapT = GL_REPEAT,
            GLenum minFilter = GL_LINEAR_MIPMAP_LINEAR,
            GLenum magFilter = GL_LINEAR);
    ~Texture();

    void Bind(GLenum unit = GL_TEXTURE0) const;
    void Unbind() const;

    GLuint GetID() const { return m_ID; }
    int GetWidth() const { return m_Width; }
    int GetHeight() const { return m_Height; }

private:
    GLuint m_ID = 0;
    int m_Width = 0;
    int m_Height = 0;
    int m_Channels = 0;
    GLenum m_InternalFormat = GL_RGBA;
    GLenum m_DataFormat = GL_RGBA;
};

#endif