#include "Model.h"

#include "Utils.h"

#include <stb/stb_image.h>

static glm::mat4 ConvertMatrix(const aiMatrix4x4& m)
{
    return glm::mat4(
        m.a1, m.b1, m.c1, m.d1,
        m.a2, m.b2, m.c2, m.d2,
        m.a3, m.b3, m.c3, m.d3,
        m.a4, m.b4, m.c4, m.d4
    );
}

void Model::Init(std::string path)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_FlipWindingOrder);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        FATAL("Failed to load model. Reason by assimp :- " + std::string(importer.GetErrorString()));
    }

    m_Dir = path.substr(0, path.find_last_of('/'));
    ProcessNode(scene->mRootNode, scene);
}

void Model::Destroy()
{
    for (auto& mesh : m_Meshes)
        mesh.Destroy();

    for(auto& [name, tex] : m_Textures)
        tex.Destroy();
}

void Model::Render(std::string attributeName, Shader& shader)
{
    for (auto& mesh : m_Meshes) {
        shader.PutMat4(attributeName, mesh.GetTransform());
        mesh.Render();
    }
}

void Model::ProcessNode(aiNode* node, const aiScene* scene, glm::mat4 transform)
{
    glm::mat4 nodeTransform = transform * ConvertMatrix(node->mTransformation);

    for (uint32_t i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        m_Meshes.push_back(ProcessMesh(mesh, scene, nodeTransform));
    }

    for (uint32_t i = 0; i < node->mNumChildren; i++)
    {
        ProcessNode(node->mChildren[i], scene, nodeTransform);
    }
}

Mesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene, glm::mat4 transform)
{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    MeshMaterial mat;

    for (uint32_t i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex{};
        vertex.pos = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        vertex.texCoord = mesh->mTextureCoords[0] ? 
                          glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y) : 
                          glm::vec2(0.0f, 0.0f);
        if (mesh->HasNormals())
        {
            vertex.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        }
        else
        {
            vertex.normal = glm::vec3(0.0f);
        }

        vertices.push_back(vertex);
    }

    for (uint32_t i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (uint32_t j = 0; j < face.mNumIndices; j++)
        {
            indices.push_back(face.mIndices[j]);
        }
    }

    if (mesh->mMaterialIndex >= 0)
    {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        std::vector<std::string> diffuse = GetMaterialTextures(material, aiTextureType_DIFFUSE);
        std::vector<std::string> normals = GetMaterialTextures(material, aiTextureType_NORMALS);
        std::vector<std::string> metalness = GetMaterialTextures(material, aiTextureType_METALNESS);
        std::vector<std::string> roughness = GetMaterialTextures(material, aiTextureType_DIFFUSE_ROUGHNESS);

        if (!diffuse.empty())
        {
            if(m_Textures.count(diffuse[0]) == 0) {
                int width, height, channels;
                auto* data = stbi_load(diffuse[0].c_str(), &width, &height, &channels, 4);
                if(!data)
                {
                    FATAL("Failed to load model texture. Reason by stb image :- " + std::string(stbi_failure_reason()))
                }

                Texture texture;
                texture.Init(width, height, data);
                m_Textures[diffuse[0]] = texture;
                stbi_image_free(data);
            }
            mat.diffusePath = diffuse[0];
        }
        if (!normals.empty())
        {
            if(m_Textures.count(normals[0]) == 0) {
                int width, height, channels;
                auto* data = stbi_load(normals[0].c_str(), &width, &height, &channels, 4);
                if(!data)
                {
                    FATAL("Failed to load model texture. Reason by stb image :- " + std::string(stbi_failure_reason()))
                }

                Texture texture;
                texture.Init(width, height, data);
                m_Textures[normals[0]] = texture;
                stbi_image_free(data);
            }
            mat.normalPath = normals[0];
        }
        if (!metalness.empty())
        {
            if(m_Textures.count(metalness[0]) == 0) {
                int width, height, channels;
                auto* data = stbi_load(metalness[0].c_str(), &width, &height, &channels, 4);
                if(!data)
                {
                    FATAL("Failed to load model texture. Reason by stb image :- " + std::string(stbi_failure_reason()))
                }

                Texture texture;
                texture.Init(width, height, data);
                m_Textures[metalness[0]] = texture;
                stbi_image_free(data);
            }
            mat.metallicPath = metalness[0];
        }
        if (!roughness.empty())
        {
            if(m_Textures.count(roughness[0]) == 0) {
                int width, height, channels;
                auto* data = stbi_load(roughness[0].c_str(), &width, &height, &channels, 4);
                if(!data)
                {
                    FATAL("Failed to load model texture. Reason by stb image :- " + std::string(stbi_failure_reason()))
                }

                Texture texture;
                texture.Init(width, height, data);
                m_Textures[roughness[0]] = texture;
                stbi_image_free(data);
            }
            mat.roughnessPath = roughness[0];
        }
    }

    Mesh newMesh;
    newMesh.Init(vertices, indices, mat, m_Textures, std::move(transform));
    return newMesh;
}

std::vector<std::string> Model::GetMaterialTextures(aiMaterial* mat, aiTextureType type)
{
    std::vector<std::string> textures;

    for (uint32_t i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);

        std::string path = m_Dir + std::string("/") + std::string(str.C_Str());
        textures.push_back(path);
    }

    return textures;
}