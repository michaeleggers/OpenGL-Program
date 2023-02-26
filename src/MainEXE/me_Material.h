#ifndef _ME_MATERIAL_
#define _ME_MATERIAL_

#include <string>
#include <vector>

struct Material
{
	uint64_t GL_DiffuseTextureHandle;
	uint64_t GL_OpacityTextureHandle;
	bool	 hasDiffuseTextureHandle;
	bool	 hasOpacityTextureHandle;
	//float    opacity;
};

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