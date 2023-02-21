#include "me_model_import.h"

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include <glm/glm.hpp>

#include <string>
#include <vector>


Model ImportModel(const std::string& file)
{
    // Create an instance of the Importer class
    Assimp::Importer importer;

    // And have it read the given file with some example postprocessing
    // Usually - if speed is not the most important aspect for you - you'll
    // probably to request more postprocessing than we do in this example.
    const aiScene* scene = importer.ReadFile(file,
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_SortByPType);

    // If the import failed, report it
    if (nullptr == scene) {
        printf("Error importing: %s\nAssImp: %s\n", file.c_str(), importer.GetErrorString());
        getchar();
        exit(-1);
    }

    // Get texture names
    std::vector<std::string> diffuseTexturePaths;
    for (size_t i = 0; i < scene->mNumMaterials; i++) {
        aiString path;
        aiReturn texFound = scene->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, 0, &path);
        if (texFound == aiReturn_SUCCESS) {
            diffuseTexturePaths.push_back(path.C_Str());
            printf("AssImp: Diffuse Texture found: %s\n", path.C_Str());
        }
    }

    // Now we can access the file's contents.
    Model model{};
    for (size_t meshIdx = 0; meshIdx < scene->mNumMeshes; meshIdx++) {
        aiMesh* aiMesh = scene->mMeshes[meshIdx];
        Mesh mesh{};
        mesh.vertices.resize(aiMesh->mNumVertices);
        mesh.diffuseTexturePath = diffuseTexturePaths[aiMesh->mMaterialIndex];
        memset(mesh.vertices.data(), 0, mesh.vertices.size()*sizeof(Vertex));
        for (size_t faceIdx = 0; faceIdx < aiMesh->mNumFaces; faceIdx++) {
            aiFace face = aiMesh->mFaces[faceIdx];
            uint32_t idx0 = face.mIndices[0];
            uint32_t idx1 = face.mIndices[1];
            uint32_t idx2 = face.mIndices[2];
            aiVector3D aiv0 = aiMesh->mVertices[idx0];
            aiVector3D aiv1 = aiMesh->mVertices[idx1];
            aiVector3D aiv2 = aiMesh->mVertices[idx2];
            Vertex v0{}, v1{}, v2{};
            v0.pos = glm::vec3(aiv0.x, aiv0.y, aiv0.z);
            v1.pos = glm::vec3(aiv1.x, aiv1.y, aiv1.z);
            v2.pos = glm::vec3(aiv2.x, aiv2.y, aiv2.z);
            aiVector3D ain0 = aiMesh->mNormals[idx0];
            aiVector3D ain1 = aiMesh->mNormals[idx1];
            aiVector3D ain2 = aiMesh->mNormals[idx2];
            v0.normal = glm::vec3(ain0.x, ain0.y, ain0.z);
            v1.normal = glm::vec3(ain1.x, ain1.y, ain1.z);
            v2.normal = glm::vec3(ain2.x, ain2.y, ain2.z);
            aiVector3D aiuv0 = aiMesh->mTextureCoords[0][idx0];
            aiVector3D aiuv1 = aiMesh->mTextureCoords[0][idx1];
            aiVector3D aiuv2 = aiMesh->mTextureCoords[0][idx2];
            v0.uv = glm::vec2(aiuv0.x, aiuv0.y);
            v1.uv = glm::vec2(aiuv1.x, aiuv1.y);
            v2.uv = glm::vec2(aiuv2.x, aiuv2.y);
            mesh.indices.push_back(idx0 + model.vertexCount);
            mesh.indices.push_back(idx1 + model.vertexCount);
            mesh.indices.push_back(idx2 + model.vertexCount);
            mesh.vertices[idx0] = v0;
            mesh.vertices[idx1] = v1;
            mesh.vertices[idx2] = v2;
        }
        mesh.indexOffset = model.indexCount;
        mesh.vertexOffset = model.vertexCount;
        model.indexCount += mesh.indices.size();
        model.vertexCount += mesh.vertices.size();
        model.meshes.push_back(mesh);
    }

    // We're done. Everything will be cleaned up by the importer destructor
    return model;
}