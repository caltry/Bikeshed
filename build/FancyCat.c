#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>

const char * const usage = "Usage: %s location file {[location] file ...}\n";

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
		if (fwrite(&buffer, 1, read, dest) != read)
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
			fclose(fp);
			fclose(new_file);
			return 4;
		}

		if (fseek(fp, 0, SEEK_END) != 0)
		{
			fprintf(stderr, "Failed to seek to the end of the file %s\n", argv[current_file]);
			fclose(fp);
			fclose(new_file);
			return 5;
		}

		long int file_size = ftell(fp);
		if (file_size == -1L)
		{
			perror("Couldn't figure out file size\n");
			fclose(fp);
			fclose(new_file);
			return 6;
		}

		if (fseek(fp, 0, SEEK_SET) != 0)
		{
			fprintf(stderr, "Failed to seek to beginning\n");	
			fclose(fp);
			fclose(new_file);
			return 7;
		}

		struct file_header hdr = { load_location, file_size };
		if (fwrite(&hdr, sizeof(hdr), 1, new_file) != 1)
		{
			fprintf(stderr, "Failed to write to destination file\n");
			fclose(fp);
			fclose(new_file);
			return 8;
		}

		if (dump_file(fp, new_file)	!= 0)
		{
			fprintf(stderr, "Failed to copy file %s\n", argv[current_file]);
			fclose(fp);
			fclose(new_file);
			return 9;
		}

		if ((current_file+1) >= argc)
		{
			fclose(fp);
			break; // We're done
		}

		// Grab the next parameter, can be a number or file
		int new_location = 0;
		if (sscanf(argv[current_file+1], "%x", &new_location) != 1)
		{
			printf("Failed to read location\n");
			load_location += file_size;
			++current_file;
		} else {
			printf("Read location\n");
			load_location = new_location;
			current_file += 2;
		}

		fclose(fp);
	}

	fclose(new_file);

	return 0;
}
