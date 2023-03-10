#ifndef _ME_MATERIAL_
#define _ME_MATERIAL_

#include <string>
#include <vector>

#include <glad/glad.h>

#define MAX_TEXTURES 32

#pragma pack(push, 1)
struct Material
{
	uint32_t albedoTextureID;
	uint32_t opacityTextureID;
	uint32_t metalnessTextureID;
	uint32_t roughnessTextureID;
	uint32_t normalTextureID;
	uint32_t hasAlbedo;
	uint32_t hasOpacity;
	uint32_t hasMetalness;
	uint32_t hasRoughness;
	uint32_t hasNormal;
	float    opacity;
};
#pragma pack(pop)

void checkGlError();

class MaterialManager
{
public:
	MaterialManager(std::string basePath);

	uint32_t Create(std::string diffuseTexturePath, float opacity, std::string opacityMapPath, std::string metalnessTexturePath, std::string roughnessTexturePath, std::string normalTexturePath);
	Material GetByID(uint32_t id);
	uint32_t UploadTexture(std::string texturePath);
	
	std::vector<Material>	m_Materials;
	GLuint					m_GL_TextureHandleBuffer;

private:
	std::string				m_BasePath;
	GLuint64*				m_GL_pHandles;
	uint32_t				m_TextureCount;
};

#endif