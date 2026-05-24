#include "Texture.h"

#include <glad/glad.h>

void Texture::Init(int32_t width, int32_t height, void* pixels, bool isDepth, bool isFloat, bool isColAttachment)
{
    m_Width = width;
    m_Height = height;
    m_Pixels = pixels;
    m_IsDepth = isDepth;
    m_IsFloat = isFloat;
    m_IsColAttachment = isColAttachment;

    glGenTextures(1, &m_Handle);
    glBindTexture(GL_TEXTURE_2D, m_Handle);

    if(!isColAttachment) 
    {
        float max;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max);
    }
    else 
    { 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    if (isDepth)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
    }
    else if (isFloat)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, pixels);
    }
    else
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    }

    if(!isColAttachment)
        glGenerateMipmap(GL_TEXTURE_2D);
    
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::Destroy()
{
    glDeleteTextures(1, &m_Handle);
}

void Texture::Resize(int32_t width, int32_t height)
{
    if(m_Width == width && m_Height == height)
        return;

    m_Width = width;
    m_Height = height;

    Bind();

    GLenum format = m_IsFloat ? GL_RGBA32F : (m_IsDepth ? GL_DEPTH24_STENCIL8 : GL_RGBA);
    GLenum pixelFormat = m_IsDepth ? GL_DEPTH_STENCIL : GL_RGBA;
    GLenum type = m_IsDepth ? GL_UNSIGNED_INT_24_8 : (m_IsFloat ? GL_FLOAT : GL_UNSIGNED_BYTE);

    glTexImage2D(GL_TEXTURE_2D, 0, format, m_Width, m_Height, 0, pixelFormat, type, m_Pixels);
    
    Unbind();
}

void Texture::Active(int slot)
{
    glActiveTexture(GL_TEXTURE0 + (slot - 1));
}

void Texture::Bind()
{
    glBindTexture(GL_TEXTURE_2D, m_Handle);
}

void Texture::Unbind()
{
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::SetPixels(unsigned char* pixels)
{
    if(m_Pixels)
        delete[] (float*)m_Pixels;  
    
    m_Pixels = pixels;
    
    Bind();
    
    glTexImage2D(GL_TEXTURE_2D, 0, (m_IsFloat) ? GL_RGBA32F : ((m_IsDepth) ? GL_DEPTH24_STENCIL8 : GL_RGBA), m_Width, m_Height, 0, GL_RGBA, (m_IsFloat) ? GL_FLOAT : ((m_IsDepth) ? GL_DEPTH_STENCIL : GL_UNSIGNED_BYTE), m_Pixels);

    Unbind();
}