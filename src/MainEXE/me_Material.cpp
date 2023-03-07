#include <stdint.h>
#include <string>
#include <assert.h>

#include <glad/glad.h>

#include "me_Material.h" 

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

uint32_t MaterialManager::UploadTexture(std::string texturePath) {
	int x, y, n;
	unsigned char* data = stbi_load(texturePath.c_str(), &x, &y, &n, 4);
	GLuint texture;
	glCreateTextures(GL_TEXTURE_2D, 1, &texture);
	glTextureStorage2D(texture, 1, GL_RGBA8, x, y);
	glTextureSubImage2D(texture, 0, 0, 0, x, y, GL_RGBA, GL_UNSIGNED_BYTE, data);
	
	//glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
	GLuint64 GL_TextureHandle = glGetTextureHandleARB(texture);
	glMakeTextureHandleResidentARB(GL_TextureHandle);
	//m_GL_pHandles[m_TextureCount] = GL_TextureHandle;
	glNamedBufferSubData(m_GL_TextureHandleBuffer, m_TextureCount * sizeof(GLuint64), sizeof(GLuint64), &GL_TextureHandle);

	stbi_image_free(data);

	return m_TextureCount++;
}

MaterialManager::MaterialManager(std::string basePath) {
	m_BasePath = basePath;
	m_TextureCount = 0;
	
	glCreateBuffers(1, &m_GL_TextureHandleBuffer);
	glNamedBufferStorage(m_GL_TextureHandleBuffer, sizeof(GLuint64) * MAX_TEXTURES, nullptr, GL_DYNAMIC_STORAGE_BIT);
	//glGenBuffers(1, &m_GL_TextureHandleBuffer);
	//glBindBuffer(GL_UNIFORM_BUFFER, m_GL_TextureHandleBuffer);
	//glBufferStorage(GL_UNIFORM_BUFFER,
	//	MAX_TEXTURES * sizeof(GLuint64),
	//	nullptr,
	//	GL_MAP_WRITE_BIT);
	
	//glBindBuffer(GL_UNIFORM_BUFFER, m_GL_TextureHandleBuffer);
	//glBindBufferRange(GL_UNIFORM_BUFFER, 1, m_GL_TextureHandleBuffer , 0, sizeof(GLuint64) * MAX_TEXTURES);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, m_GL_TextureHandleBuffer);

	//glNamedBufferStorage(m_GL_TextureHandleBuffer,
	//	MAX_TEXTURES * sizeof(GLuint64), nullptr, GL_DYNAMIC_STORAGE_BIT);
	//m_GL_pHandles = (GLuint64*)glMapNamedBufferRange(m_GL_TextureHandleBuffer, 0, MAX_TEXTURES * sizeof(GLuint64), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	//m_GL_pHandles = (GLuint64*)glMapBufferRange(GL_UNIFORM_BUFFER, 0, MAX_TEXTURES * sizeof(GLuint64), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
}

// TODO: If same diffuseTexturePath is passed more than once, we do NOT
//       want to load it again (and not create a new resource on GPU!!!)
uint32_t MaterialManager::Create(std::string diffuseTexturePath, float opacity, std::string opacityMapPath) {	
	Material material{};
	if (!diffuseTexturePath.empty()) {
		material.hasDiffuseTextureHandle = 1;
		material.diffuseTextureID = UploadTexture(diffuseTexturePath);
	}
	if (!opacityMapPath.empty()) {
		material.hasOpacityTextureHandle = 1;
		material.opacityTextureID = UploadTexture(opacityMapPath);		
	} 
	material.opacity = opacity;
	m_Materials.push_back(material);

	return m_Materials.size() - 1; // TODO: What if there is something like Delete()? Then the next ID wouldn not be the last entry.
}

Material MaterialManager::GetByID(uint32_t id) {
	assert(id < m_Materials.size());
	return m_Materials[id];
}
