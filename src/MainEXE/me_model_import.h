#ifndef _ME_MODEL_IMPORT_
#define _ME_MODEL_IMPORT_

#include <stdint.h>

#include <string>
#include <vector>

#include <glm/glm.hpp>

struct Vertex
{
    glm::vec3 pos;
    glm::vec2 uv;
    glm::vec3 normal;
};

struct Mesh
{
    std::vector<Vertex>     vertices;
    std::vector<uint32_t>   indices;
    uint32_t                vertexOffset;
    uint32_t                indexOffset;
    std::string             diffuseTexturePath;
};

struct Model
{
    std::vector<Mesh>   meshes;
    uint32_t            vertexCount;
    uint32_t            indexCount;
};


Model ImportModel(const std::string& file);

#endif