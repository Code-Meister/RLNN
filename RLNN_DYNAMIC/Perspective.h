#pragma once
#include "GL\glew.h"
#include <string>

typedef std::string String;

template <size_t width, size_t height>
class Texture
{
	float depths[width * height];
	uint8_t colors[width * height * 30];

public:
	Texture(String filename)
	{
		loadTexture(colors, filename);

		memset(depths, 0, sizeof(depths));
	}
private:
	static void loadTexture(uint8_t* colors, String filename)
	{
			printf("Reading image %s\n", filename.c_str());

			unsigned char header[54];
			uint32_t dataPos;
			uint32_t imageSize;
			//uint32_t width, height;

			//unsigned char * data;

			FILE * file = fopen(filename.c_str(), "rb");
			if (!file) { printf("%s could not be opened. Are you in the right directory ? Don't forget to read the FAQ !\n", filename.c_str()); getchar(); return; }

			if (fread(header, 1, 54, file) != 54) {
				printf("Not a correct BMP file\n");
				return;
			}

			if (header[0] != 'B' || header[1] != 'M') {
				printf("Not a correct BMP file\n");
				return;
			}

			if (*(int*)&(header[0x1E]) != 0) { printf("Not a correct BMP file\n");    return; }
			if (*(int*)&(header[0x1C]) != 24) { printf("Not a correct BMP file\n");    return; }

			dataPos = *(int*)&(header[0x0A]);
			imageSize = *(int*)&(header[0x22]);
			//width = *(int*)&(header[0x12]);
			//height = *(int*)&(header[0x16]);

			if (imageSize == 0)    imageSize = width * height * 3;
			if (dataPos == 0)      dataPos = 54;

			//data = new unsigned char[imageSize];
			//fread(data, 1, imageSize, file);

			fread(colors, 1, imageSize, file);

			fclose(file);

			/*GLuint textureID;
			glGenTextures(1, &textureID);

			glBindTexture(GL_TEXTURE_2D, textureID);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);

			delete[] data;

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			return textureID;*/
		}
};

template <size_t width, size_t height>
class Perspective
{
	Texture<width, height> texture;
	
public:
	Perspective(String filename) : texture(filename)
	{
		
	}
};

