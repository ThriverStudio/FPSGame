#pragma once

#include <glm/glm.hpp>
#include <map>
#include <string>
#include <cstdint>

#include "Texture.h"

struct Vertex
{
    glm::vec3 pos;
    glm::vec2 texCoord;
    glm::vec3 normal;
};

struct MeshMaterial
{
    std::string diffusePath;
    std::string normalPath;
    std::string metallicPath;
    std::string roughnessPath;
};

class Mesh
{
public:
    void Init(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, MeshMaterial material, std::unordered_map<std::string, Texture>& textureList,glm::mat4&& transform);
    void Destroy();

    void Render();
    inline const MeshMaterial& GetMaterial() { return m_Material; }
    inline glm::mat4& GetTransform() { return m_Transform; }

private:
    uint32_t m_VertCount;
    uint32_t m_Vao, m_Vbo, m_Ebo;
    glm::mat4 m_Transform = glm::mat4(1.0f);
    
    MeshMaterial m_Material;
    std::unordered_map<std::string, Texture>* m_TextureList = nullptr;
};