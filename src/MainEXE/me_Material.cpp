#include <stdint.h>
#include <string>
#include <assert.h>

#include <glad/glad.h>

#include "me_Material.h" 

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

static uint64_t uploadTexture(std::string texturePath) {
	int x, y, n;
	unsigned char* data = stbi_load(texturePath.c_str(), &x, &y, &n, 4);
	GLuint texture;
	glCreateTextures(GL_TEXTURE_2D, 1, &texture);
	glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureStorage2D(texture, 1, GL_RGBA8, x, y);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTextureSubImage2D(texture, 0, 0, 0, x, y, GL_RGBA, GL_UNSIGNED_BYTE, data);
	uint64_t GL_TextureHandle = glGetTextureHandleARB(texture);
	glMakeTextureHandleResidentARB(GL_TextureHandle);
	stbi_image_free(data);

	return GL_TextureHandle;
}

MaterialManager::MaterialManager(std::string basePath) {
	m_BasePath = basePath;
}

// TODO: If same diffuseTexturePath is passed more than once, we do NOT
//       want to load it again (and not create a new resource on GPU!!!)
uint32_t MaterialManager::Create(std::string diffuseTexturePath, float opacity, std::string opacityMapPath) {	
	Material material{};
	if (!diffuseTexturePath.empty()) {
		material.hasDiffuseTextureHandle = true;
		material.GL_DiffuseTextureHandle = uploadTexture(diffuseTexturePath);
	}
	if (!opacityMapPath.empty()) {
		material.hasOpacityTextureHandle = true;
		material.GL_OpacityTextureHandle = uploadTexture(opacityMapPath);		
	} 
	//material.opacity = opacity;
	m_Materials.push_back(material);

	return m_Materials.size() - 1; // TODO: What if there is something like Delete()? Then the next ID wouldn not be the last entry.
}

Material MaterialManager::GetByID(uint32_t id) {
	assert(id < m_Materials.size());
	return m_Materials[id];
}
