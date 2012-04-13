#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

const char * const usage = "Usage: %s location file {[location] file ...}\n"

struct file_header
{
	uint32_t location;
	uint32_t size;
};

int dump_file(FILE *src, FILE *dest)
{
	char buffer[4096];
	int read = 0;

	while (!feof(src) && !ferror(src))
	{
		read = fread(&buffer, 1, sizeof(buffer), src);
		if (fwrite(&buffer, read, 1, dest) != 1)
		{
			return 1;
		}
	}

	return 0;
}

int main(int argc, char *argv[])
{
	if (argc < 3)
	{
		fprintf(stderr, usage, argv[0]);
		return 1;
	}

	int load_location = 0;
	if (sscanf(argv[1], "%x", &load_location) != 1)
	{
		fprintf(stderr, "Invalid initial location\n");
		return 2;
	}

	FILE *new_file = fopen("image.dat", "wb");
	if (new_file == NULL)
	{
		fprintf(stderr, "Failed to create output file\n");
		return 3;
	}

	int current_file = 2;

	while (current_file < argc)
	{
		FILE *fp = fopen(argv[current_file], "rb");
		if (fp == NULL)
		{
			fprintf(stderr, "Error reading in file %s\n", argv[current_file]);
			return 4;
		}

		if (fseek(fp, 0, SEEK_END) != 0)
		{
			fprintf(stderr, "Failed to seek to the end of the file %s\n", argv[current_file]);
			return 5;
		}

		long int file_size = ftell(fp);
		if (file_size == -1L)
		{
			perror(errno);
			return 6;
		}

		if (fseek(fp, 0, SEEK_SET) != 0)
		{
			fprintf(stderr, "Failed to seek to beginning\n");	
			return 7;
		}

		struct file_header hdr = { load_location, file_size };
		if (fwrite(&hdr, sizeof(hdr), 1, new_file) != 1)
		{
			fprintf(stderr, "Failed to write to destination file\n");
			return 8;
		}

			
	}

	return 0;
}
