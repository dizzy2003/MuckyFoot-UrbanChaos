// soundconv.cpp
//
// convert sounds using Miles

#include "windows.h"
#include <stdio.h>

#include "c:\fallen\miles\mss.h"
#pragma comment(lib, "c:\\fallen\\miles\\mss32.lib")

static void ConvertDir(char* src, char* dst);
static void ConvertFile(char* src, char* dst);
static void WriteFile(char* filename, void* data, size_t datalen);

// main
//
// does the stuff

int main(int argc, char** argv)
{
	char*	src = "TALK2";
	char*	dst = "TALK2_ADPCM";

	if (argc == 3)
	{
		src = argv[1];
		dst = argv[2];
	}

	char	cmd[256];
	sprintf(cmd, "DELTREE /Y %s", dst);
	system(cmd);

	printf("Converting %s ...\n", src);
	ConvertDir(src, dst);

	return 0;
}

// ConvertDir
//
// goes through and converts all the WAVs in a directory

static void ConvertDir(char* src, char* dst)
{
	char			src2[MAX_PATH];
	char			dst2[MAX_PATH];
	WIN32_FIND_DATA	found;
	HANDLE			hFile;

	if (!CreateDirectory(dst, NULL))
	{
		fprintf(stderr, "Can't create directory %s\n", dst);
		exit(-1);
	}

	sprintf(src2, "%s\\*.*", src);

	hFile = FindFirstFile(src2, &found);

	do
	{
		if (strcmp(found.cFileName, ".") && strcmp(found.cFileName, ".."))
		{
			sprintf(src2, "%s\\%s", src, found.cFileName);
			sprintf(dst2, "%s\\%s", dst, found.cFileName);

			if (found.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				ConvertDir(src2, dst2);
			}
			else
			{
				ConvertFile(src2, dst2);
			}
		}
	} while (FindNextFile(hFile, &found));
}

// ConvertFile
//
// convert a file

static void ConvertFile(char* src, char* dst)
{
	int	extpos = strlen(src) - 4;

	if (stricmp(src + extpos, ".wav"))	return;

	// read in file
	FILE*	fd = fopen(src, "rb");
	if (!fd)
	{
		fprintf(stderr, "Can't open file %s\n", src);
		return;
	}

	fseek(fd, 0, SEEK_END);
	size_t length = ftell(fd);
	fseek(fd, 0, SEEK_SET);

	U8*	wav = new U8[length];

	if (fread(wav, 1, length, fd) != length)
	{
		fprintf(stderr, "Can't read file %s\n", src);
		fclose(fd);
		delete[] wav;
		return;
	}

	fclose(fd);

	// compress file
	AILSOUNDINFO	info;

	if (!AIL_WAV_info(wav, &info))
	{
		fprintf(stderr, "Problem with format of %s\n", src);
		delete[] wav;
		return;
	}

	if (info.format == WAVE_FORMAT_IMA_ADPCM)
	{
		WriteFile(dst, wav, length);
		delete[] wav;
	}
	else
	{
		void*	output;
		U32		outlen;

		if (!AIL_compress_ADPCM(&info, &output, &outlen))
		{
			fprintf(stderr, "Couldn't convert %s\n", src);
			delete[] wav;
			return;
		}

		delete[] wav;

		WriteFile(dst, output, outlen);
		AIL_mem_free_lock(output);
	}

	return;
}

static void WriteFile(char* filename, void* data, size_t datalen)
{
	FILE* fd = fopen(filename, "wb");
	if (!fd)
	{
		fprintf(stderr, "Can't open file %s\n", filename);
		return;
	}

	if (fwrite(data, 1, datalen, fd) != datalen)
	{
		fprintf(stderr, "Can't write file %s\n", filename);
	}
	
	fclose(fd);
	return;
}
