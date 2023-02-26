#ifndef _ME_MATERIAL_
#define _ME_MATERIAL_

#include <string>
#include <vector>

//#pragma pack(push, 1)
struct Material
{
	uint64_t GL_DiffuseTextureHandle;
	uint64_t GL_OpacityTextureHandle;
	uint32_t hasDiffuseTextureHandle;
	uint32_t hasOpacityTextureHandle;
	float    opacity;
};
//#pragma pack(pop)

class MaterialManager
{
public:
	MaterialManager(std::string basePath);

	uint32_t Create(std::string diffuseTexturePath, float opacity, std::string opacityMapPath);
	Material GetByID(uint32_t id);

	std::vector<Material>	m_Materials;

private:
	std::string				m_BasePath;
};

#endif