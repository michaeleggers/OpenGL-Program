#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include <glm/glm.hpp>

#include <string>
#include <vector>

#include "me_model_import.h"
#include "me_Material.h"

Model ImportModel(MaterialManager& materialManager, const std::string basePath, const std::string assetDir, const std::string& modelFile)
{
    // Create an instance of the Importer class
    Assimp::Importer importer;

    std::string relFilePath = assetDir + modelFile;
    std::string absAssetDir = basePath + "res/models/" + assetDir; // TODO: lazy. What if double '//'? What if assetDir = knight without '/'
    std::string gltfFile = absAssetDir + modelFile;
    printf("AssImp: Loading model [ %s ]...\n", relFilePath.c_str());
    // And have it read the given file with some example postprocessing
    // Usually - if speed is not the most important aspect for you - you'll
    // probably to request more postprocessing than we do in this example.
    const aiScene* scene = importer.ReadFile(gltfFile,
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_SortByPType |
        aiProcess_FlipUVs);

    // If the import failed, report it
    if (nullptr == scene) {
        printf("Error importing: %s\nAssImp: %s\n", gltfFile.c_str(), importer.GetErrorString());
        getchar();
        exit(-1);
    }

    // Get texture names
    aiTextureType texturesToLoad[] = {
        aiTextureType_DIFFUSE,
        aiTextureType_OPACITY
    };

    printf("AssImp: Loading Materials [ %d found ]\n", scene->mNumMaterials);
    for (size_t i = 0; i < scene->mNumMaterials; i++) {
        
        std::string diffuseTexture{};
        float opacity = 1.0f;
        float transparencyFactor = 1.0f;
        float colorTransparent = 1.0f;
        std::string opacityTexture{};
        for (size_t texCount = 0; texCount < 2; texCount++) {
            aiString texturePath;
            aiReturn texFound = scene->mMaterials[i]->GetTexture(texturesToLoad[texCount], 0, &texturePath);
            if (texFound == aiReturn_SUCCESS) {
                printf("AssImp: Texture found: %s\n", texturePath.C_Str());
                if (texturesToLoad[texCount] == aiTextureType_DIFFUSE) {
                    diffuseTexture = absAssetDir + texturePath.C_Str();
                    aiMaterial* material = scene->mMaterials[i];
                    aiGetMaterialFloat(material, AI_MATKEY_OPACITY, &opacity);
                    aiGetMaterialFloat(material, AI_MATKEY_TRANSPARENCYFACTOR, &transparencyFactor);
                    aiGetMaterialFloat(material, AI_MATKEY_COLOR_TRANSPARENT, &colorTransparent);
                    
                }
                if (texturesToLoad[texCount] == aiTextureType_OPACITY) {
                    opacityTexture = absAssetDir + texturePath.C_Str();
                }
            }
        }        
        
       materialManager.Create(diffuseTexture, opacity, opacityTexture);        
    }

    // Now we can access the file's contents.
    Model model{};
    for (size_t meshIdx = 0; meshIdx < scene->mNumMeshes; meshIdx++) {
        aiMesh* aiMesh = scene->mMeshes[meshIdx];
        Mesh mesh{};
        mesh.name = std::string(aiMesh->mName.C_Str());
        printf("AssImp: Loading mesh: %s\n", mesh.name.c_str());
        mesh.vertices.resize(aiMesh->mNumVertices);
        mesh.materialID = aiMesh->mMaterialIndex; // TODO: Might differ from material mangers material ID!
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