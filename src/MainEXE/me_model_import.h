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
};

struct Mesh
{
    std::vector<Vertex>     vertices;
    std::vector<uint32_t>   indices;
};

struct Model
{
    std::vector<Mesh>   meshes;
    uint32_t            vertexCount;
    uint32_t            indexCount;
};


Model ImportModel(const std::string& file);

#endif