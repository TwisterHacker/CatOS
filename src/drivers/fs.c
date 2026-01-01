#include <drivers/fs.h>
#include <common.h>
#include <drivers/terminal.h>
#include <drivers/timer.h>

extern char path[32][13];
extern int path_counter;

device_t devices[MAXDEVICES];

void init_devices(){
	for (int i = 0; i < MAXDEVICES; i++) {
    	devices[i].active = 0;
	}
}

int mount_device(const char* name,
	int (*open)(char*),
	int (*close)(char*),
	int (*write)(char*, uint8_t*),
	int (*list)(char path_list[32][13]),
	int (*load_dir)(char path[32][13]),
	int (*create_entry)(char*, int),
	int (*remove_entry)(char*, int),
	uint8_t *file,
	uint32_t file_size) // file buffer
{
	for (int i = 0; i < MAXDEVICES; i++){
		if (devices[i].active == 0){
			devices[i] = (device_t){
				.name = name,
				.open = open,
				.close = close,
				.write = write,
				.list = list,
				.load_dir = load_dir,
				.create_entry = create_entry,
				.remove_entry = remove_entry,
				.file = file,
				.file_size = file_size,
				.active = 1
			};
			printf("\nMounted new device : \"", YELLOW);
			printf(name, WHITE);
			printf("\" on path \"/mnt/", YELLOW);
			printf(name, YELLOW);
			printf("\"!", YELLOW);
			return 1;
		}
	}

	printf("\nERROR: There are too much devices mounted!", RED);

	return -1;

}

int unmount_device(const char* name){
	for (int i = 0; i < MAXDEVICES; i++){
		if (strcmp(devices[i].name, name) == 0){
			devices[i] = (device_t){
				.name = 0,
				.open = 0,
				.close = 0,
				.write = 0,
				.list = 0,
				.load_dir = 0,
				.create_entry = 0,
				.remove_entry = 0,
				.file = 0,
				.file_size = 0,
				.active = 0
			};
			printf("\nUnmounted device : \"", YELLOW);
			printf(name, WHITE);
			printf("\"!", YELLOW);
			return 1;
		}
	}

	printf("\nERROR: There are no devices with name: \"", RED);
	printf(name, WHITE);
	printf("\"!", RED);

	return -1;
}

int detect_device(){
	int dev_num = -1;
	for (int i = 0; i < 16; i++) {
		if (path[2][0] == '\0'){
			dev_num = -1;
		}
		else if (strncmp(path[2], devices[i].name, 4) == 0){
			dev_num = i;
			break;
		}
	}

	return dev_num;
}

void list_directory(char path_list[32][13]){

	// parsing path

	if (path_list[0][0] == '\0' && path_list[1][0] == '\0'){ // root dir ---> "/"
        printf("DIR  | ", WHITE);
        printf("tmp", YELLOW);

        printf("\n", 0);

        printf("DIR  | ", WHITE);
        printf("mnt", YELLOW);
        
        printf("\n", 0);
	}
	else if (strncmp(path_list[1], "mnt", 3) == 0 && path_list[1][3] == '\0'){ // mnt dir ---> "/mnt/"
		if (path_list[2][0] == '\0'){ // mnt dir ---> "/mnt/"
			for (int i = 0; i < MAXDEVICES; i++){
				if (devices[i].active == 1){
        			printf("DIR  | ", WHITE);
					printf(devices[i].name, YELLOW);
        			printf("\n", 0);
				}
			}
		} 
		else{ // device dir ---> "/mnt/device_t"
			int error = 1;
			for (int i = 0; i < MAXDEVICES; i++){
				if (strncmp(path_list[2], devices[i].name, sizeof(devices[i].name)) == 0 && path_list[2][sizeof(devices[i].name)] == '\0'){
					devices[i].list(path_list);
					error = 0;
					break;
				}
			}
			if (error){
				printf("\nERROR: Unkonwn device!", RED);
			}
		}
	}
	else {
		printf("\nERROR: Unkonwn directory!", RED);
	}
}

int create_folder(char* name){
	int dev_num = detect_device();

	if (dev_num != -1){

		if (devices[dev_num].create_entry(name, 1) == -1){
			return -1;
		}
		
	}else{

		// !!!!!!!!!!!!

	}

	printf("\nFolder \"", GREEN);
	printf(name, WHITE);
	printf("\" created succesfully!\n", GREEN);

	return 1;
}

void change_directory(char* dir){
	if (dir == NULL || dir[0] == '\0' || dir[0] == ' ') {
        printf("\nERROR: Empty path!", RED);
        return;
    }

	if (dir != NULL && dir[0] != '\0'){

		int dev_num = -1;

		// strip path
		
		char path_list[32][13] = {0};

		int j = 0; // path_list element index
		int i = 0; // path_list[j] symbol position

		for (int m = 0; dir[m] != '\0'; m++){
			if (dir[m] != '/'){
				path_list[j][i] = dir[m];
				i++;
			}
			else{
				if (m != 0){
					if (i > 0) j++; // cd folder1////folder2// ---> cd folder1/folder2
					i = 0;
				}
				else{
					i = 0;
				}
			}
		}

		// parsing path

		for (int n = 0; n <= j; n++){
			if (strncmp(path_list[n], "..", 2) == 0 && path_list[n][2] == '\0'){
				memset(path[path_counter], 0, 13);
				if (path_counter > 0) path_counter--;

				dev_num = detect_device();

				if (path_counter > 1 && dev_num != -1) devices[dev_num].load_dir(path);
			}
			else{
				if (path_counter < 31){
					if (path_list[n][0] != '\0'){ // cd folder1/folder2// ---> cd folder1/folder2

						path_counter++;
						strcpy(path[path_counter], path_list[n]);

						dev_num = detect_device();

						if (path_counter > 1 && dev_num != -1){
							if (devices[dev_num].load_dir(path) == 0){

							}
							else {
								memset(path[path_counter], 0, 13);
								if (path_counter > 0) path_counter--;
								return;
							}
						}

						continue;
					}
				}
				else{
					printf("ERROR: Too deep path!", RED);
					return;
				}
			}
		}

	}
}

void print_current_directory(){
	for (int i = 0; i < path_counter+1; i++){
		printf(path[i], LIGHTBLUE);
		printf("/", BLUE);
	}
}

void cat_file(char* name){
	int dev_num = detect_device();

	if (dev_num != -1){
		if (devices[dev_num].open(name) == -1){
			return;
		}

		printf("\n", GREEN);

	    for (uint32_t i = 0; i < devices[dev_num].file_size; i++) {
	        char c = devices[dev_num].file[i];
	        if (c >= 32 && c <= 126) {
	            print_char(c, WHITE);
	            msleep(1);
	        } 
	        else {
	        	if (c == 10)
	        		printf("\n", 0);
	        }
	    }

	    devices[dev_num].close(name);

		printf("\n", GREEN);
	}
}