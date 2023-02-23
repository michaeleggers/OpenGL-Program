#include "me_IO.h"

#include <string>

#include <SDL.h>

std::string LoadTextFile(std::string file)
{
	SDL_RWops* rwOps = SDL_RWFromFile(file.c_str(), "r");
	if (!rwOps) {
		printf("Failed to read file %s.\nSDL ERROR: %s\n", file.c_str(), SDL_GetError());
		exit(-1);
	}
	Sint64 fileSize = rwOps->size(rwOps);
	std::string data{};
	data.resize(fileSize);
	size_t bytesRead = SDL_RWread(rwOps, &data[0], fileSize, 1);

	SDL_RWclose(rwOps);

	return data;
}
