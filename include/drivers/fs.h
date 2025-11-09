#ifndef FS_H
#define FS_H

#include <common.h>

int mount_device(const char* name,
	int (*open)(char*),
	int (*close)(char*),
	int (*write)(char*, uint8_t*),
	int (*list)(char path_list[32][13]),
	int (*load_dir)(char path[32][13]),
	int (*create_entry)(char*, int), // name, mode (0 - directory, 1 - file)
	int (*remove_entry)(char*, int), // name, mode (0 - directory, 1 - file)
	uint8_t *file,
	uint32_t file_size);

typedef struct
{
	const char* name; // 4 letters name
	int (*open)(char* name);
	int (*close)(char* name);
	int (*write)(char* name, uint8_t* buffer);
	int (*list)(char path_list[32][13]);
	int (*load_dir)(char path[32][13]);
	int (*create_entry)(char* name, int mode);
	int (*remove_entry)(char* name, int mode);
	uint8_t *file; // file buffer
	uint32_t file_size;
	int active;

} device_t;

#define MAXDEVICES 16

#endif