#include "Mesh.h"

#include <glad/glad.h>

#include <cmath>

void Mesh::Init(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, MeshMaterial material, std::unordered_map<std::string, Texture>& textureList, glm::mat4&& transform)
{
    m_VertCount = indices.size();
    m_Material = material;
    m_Transform = transform;
    m_TextureList = &textureList;
    
    {
        glGenVertexArrays(1, &m_Vao);
        glGenBuffers(1, &m_Vbo);
        glGenBuffers(1, &m_Ebo);

        glBindVertexArray(m_Vao);

        glBindBuffer(GL_ARRAY_BUFFER, m_Vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * indices.size(), indices.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

        glBindVertexArray(0);
    }
}

void Mesh::Destroy()
{
    glDeleteVertexArrays(1, &m_Vao);
    glDeleteBuffers(1, &m_Vbo);
    glDeleteBuffers(1, &m_Ebo);
}

void Mesh::Render()
{
    Texture* texture = &(*m_TextureList)[m_Material.diffusePath];
    if (texture)
    {
        texture->Active(1);
        texture->Bind();
    }

    glBindVertexArray(m_Vao);
    glDrawElements(GL_TRIANGLES, m_VertCount, GL_UNSIGNED_INT, 0);

    glBindTexture(GL_TEXTURE_2D, 0);
}