#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>

#define SECTOR_SIZE 512

const char * const usage = "Usage: %s location file {[location] file ...}\n";

struct file_header
{
	uint32_t location;
	uint32_t size;
};

int dump_file(FILE *src, FILE *dest)
{
	char buffer[4096];
	size_t read = 0;

	while (!feof(src) && !ferror(src))
	{
		read = fread(&buffer, 1, sizeof(buffer), src);
		if (fwrite(&buffer, 1, read, dest) != read)
		{
			return 1;
		}
	}

	if (ferror(src))
	{
		return 2;
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

	unsigned int load_location = 0;
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

	printf("%s:\n", argv[0]);

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

		int number_of_sectors = file_size + sizeof(struct file_header) + 
				(SECTOR_SIZE - ((file_size + sizeof(struct file_header)) % SECTOR_SIZE));

		number_of_sectors /= SECTOR_SIZE;
		

		struct file_header hdr = { load_location, number_of_sectors };
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

		/* Pad to the next 512 byte location */
		if (((file_size + sizeof(struct file_header)) % SECTOR_SIZE) != 0)
		{
			long int pad_amount = SECTOR_SIZE - 
				((file_size + sizeof(struct file_header)) % SECTOR_SIZE);

			char* padding = (char *)malloc(pad_amount);
			memset(padding, 0x22, pad_amount);
			if (fwrite(padding, pad_amount, 1, new_file) != 1)
			{
				free(padding);
				fprintf(stderr, "Failed to pad output file\n");
				fclose(fp);
				fclose(new_file);
				return 10;
			}
			free(padding);
		}

		printf("File: %s, load location: 0x%X, sector count: %d, file size: %ld bytes\n",
				argv[current_file], load_location, number_of_sectors, file_size);

		if ((current_file+1) >= argc)
		{
			fclose(fp);
			break; // We're done
		}

		// Grab the next parameter, can be a number or file
		unsigned int new_location = 0;
		if (sscanf(argv[current_file+1], "%x", &new_location) != 1)
		{
			load_location += file_size;
			++current_file;
		} else {
			load_location = new_location;
			current_file += 2;
		}

		fclose(fp);
	}

	// Put the end byte marker
	unsigned char end[SECTOR_SIZE];
	memset(&end[0], 0, sizeof(end));
	end[0] = end[1] = end[2] = end[3] = 0xFF;
	end[4] = end[5] = end[6] = end[7] = 0xFF;
	if (fwrite(&end, sizeof(end), 1, new_file) != 1)
	{
		fprintf(stderr, "Failed to put end marker\n");
		fclose(new_file);
		return 11;
	}	

	fclose(new_file);

	return 0;
}
