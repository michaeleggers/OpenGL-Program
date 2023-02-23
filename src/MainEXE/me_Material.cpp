#include <stdint.h>
#include <string>
#include <assert.h>

#include <glad/glad.h>

#include "me_Material.h" 

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>


MaterialManager::MaterialManager(std::string basePath) {
	m_BasePath = basePath;
}

// TODO: If same diffuseTexturePath is passed more than once, we do NOT
//       want to load it again (and not create a new resource on GPU!!!)
uint32_t MaterialManager::Create(std::string diffuseTexturePath) {
	std::string absDiffuseTexturePath = diffuseTexturePath;
	int x, y, n;
	unsigned char* data = stbi_load(absDiffuseTexturePath.c_str(), &x, &y, &n, 4);
	GLuint diffuseTexture;
	glCreateTextures(GL_TEXTURE_2D, 1, &diffuseTexture);
	glTextureParameteri(diffuseTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(diffuseTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureStorage2D(diffuseTexture, 1, GL_RGBA8, x, y);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTextureSubImage2D(diffuseTexture, 0, 0, 0, x, y, GL_RGBA, GL_UNSIGNED_BYTE, data);

	Material material{};
	material.GL_DiffuseTextureHandle = glGetTextureHandleARB(diffuseTexture);
	glMakeTextureHandleResidentARB(material.GL_DiffuseTextureHandle);
	m_Materials.push_back(material);
	stbi_image_free(data);

	return m_Materials.size() - 1; // TODO: What if there is something like Delete()? Then the next ID wouldn not be the last entry.
}

Material MaterialManager::GetByID(uint32_t id) {
	assert(id < m_Materials.size());
	return m_Materials[id];
}
