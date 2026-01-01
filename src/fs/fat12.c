#include <common.h>
#include <fs/fat12.h>
#include <drivers/terminal.h>
#include <drivers/timer.h>
#include <memory/memory_managment.h>
#include <drivers/fs.h>

extern int floppy_read_sector();
extern int floppy_write_sector();
extern int mount_device();
extern int detect_device();

extern short floppy_motor_state;
extern char path[32][13];

extern device_t devices[16];

fat12_info_t fat_info[2];
uint8_t* fat[2];

#define ROOT_ENTRIES_MAX 224
#define ENTRY_SIZE 32
uint8_t root_dir_buf[2][ENTRY_SIZE * ROOT_ENTRIES_MAX];

uint8_t* dir_buf;
uint8_t* file_buf;
uint32_t file_size;

uint32_t entries_count = 0;

void lba_to_chs(uint32_t lba, uint8_t* track, uint8_t* head, uint8_t* sector) {
    const uint8_t sectors_per_track = 18;

    *head = (lba % (sectors_per_track * 2)) / sectors_per_track;
    *track = (lba / (sectors_per_track * 2));
    *sector = (lba % sectors_per_track + 1);
}

void print_boot_sector(int drive){
    uint8_t boot_sector_data[512];
    floppy_read_sector(drive, 0, 0, 1, boot_sector_data);

    fat12_boot_sector_t* boot = (fat12_boot_sector_t*)boot_sector_data;

    char str[16];

    itoa(boot->bytes_per_sector, str, 10);
    printf("Bytes per sector: ", WHITE);
    printf(str, WHITE);

    itoa(boot->sectors_per_cluster, str, 10);
    printf("\nSectors per cluster: ", WHITE);
    printf(str, WHITE);

    itoa(boot->reserved_sectors, str, 10);
    printf("\nReserved sectors: ", WHITE);
    printf(str, WHITE);

    itoa(boot->fat_count, str, 10);
    printf("\nFAT copies: ", WHITE);
    printf(str, WHITE);

    itoa(boot->root_entries, str, 10);
    printf("\nRoot entries: ", WHITE);
    printf(str, WHITE);

    itoa(boot->fat_size_sectors, str, 10);
    printf("\nSectors per FAT: ", WHITE);
    printf(str, WHITE);

    itoa(boot->total_sectors, str, 10);
    printf("\nTotal sectors: ", WHITE);
    printf(str, WHITE);
    printf("\n", WHITE);
}

void fat12_parse_boot_sector(int drive) {

    uint8_t sector[512];
    floppy_read_sector(drive, 0, 0, 1, sector);

    fat12_boot_sector_t* bs = (fat12_boot_sector_t*)sector;

    fat_info[drive].bytes_per_sector = bs->bytes_per_sector;
    fat_info[drive].sectors_per_cluster = bs->sectors_per_cluster;
    fat_info[drive].reserved_sectors = bs->reserved_sectors;
    fat_info[drive].fat_count = bs->fat_count;
    fat_info[drive].root_entries = bs->root_entries;
    fat_info[drive].total_sectors = bs->total_sectors;
    fat_info[drive].fat_size_sectors = bs->fat_size_sectors;

    fat_info[drive].root_dir_sectors = ((bs->root_entries * 32) + (bs->bytes_per_sector - 1)) / bs->bytes_per_sector;
    fat_info[drive].fat_start_sector = bs->reserved_sectors;
    fat_info[drive].root_dir_start_sector = fat_info[drive].fat_start_sector + fat_info[drive].fat_count * fat_info[drive].fat_size_sectors;
    fat_info[drive].data_start_sector = fat_info[drive].root_dir_start_sector + fat_info[drive].root_dir_sectors;
}

void fat12_load_fat(int drive) {
    uint32_t fat_size_bytes = fat_info[drive].fat_size_sectors * fat_info[drive].bytes_per_sector;
    fat[drive] = kmalloc(fat_size_bytes);

    if (!fat[drive]) {
        printf("\nERROR: FAT memory allocation failed", RED);
    }

    for (uint32_t i = 0; i < fat_info[drive].fat_size_sectors; i++) {
        uint32_t lba = fat_info[drive].fat_start_sector + i;
        uint8_t head, track, sector;
        lba_to_chs(lba, &track, &head, &sector);

        floppy_read_sector(
            drive,
            track,
            head,
            sector,
            fat[drive] + i * fat_info[drive].bytes_per_sector
        );

    }

}

void fat12_load_root_dir(int drive) {
    uint8_t *buf = root_dir_buf[drive];
    fat12_boot_sector_t* bs = (fat12_boot_sector_t*)kmalloc(512);    
    floppy_read_sector(drive, 0, 0, 1, (uint8_t*)bs);

    // uint16_t sptrack = bs->sectors_per_track;
    // uint16_t nheads  = bs->head_count;

    uint32_t sector0 = fat_info[drive].root_dir_start_sector;
    uint32_t count   = fat_info[drive].root_dir_sectors;
    uint32_t max_sectors = sizeof(root_dir_buf[drive]) / 512;
    if (count > max_sectors) count = max_sectors;

    for (uint32_t i = 0; i < count; i++) {
        uint32_t lba = sector0 + i;
        uint8_t head, track, sector;
        lba_to_chs(lba, &track, &head, &sector);
        floppy_read_sector(drive, track, head, sector, buf + i * 512);
    }
    kfree(bs);
}

fat12_dir_entry_t* find_entry(fat12_dir_entry_t* entries, int count, char* name, int mode){ // if mode == 0: searching file; else if mode == 1: searching dir
    for (int i = 0; i < count; i++){
        if ((uint8_t)entries[i].filename[0] == 0x00) break;
        if ((uint8_t)entries[i].filename[0] == 0xE5) continue;
        if ((entries[i].attr & 0x0F) == 0x0F) continue;

        char fullname[13];
        int k = 0;

        for (int j = 0; j < 8 && entries[i].filename[j] != ' '; j++)
            fullname[k++] = entries[i].filename[j];
        if (entries[i].ext[0] != ' ' && entries[i].ext[0] != '\0'){
            fullname[k++] = '.';
            for (int j = 0; j < 8 && entries[i].ext[j] != ' '; j++)
                fullname[k++] = entries[i].ext[j];
        }

        fullname[k] = 0;

        tolower(fullname);
        tolower(name);

        if (mode == 0 && strcmp(fullname, name) == 0 && (entries[i].attr & 0x3F) == 0x20) return &entries[i];       // FILE
        else if (mode == 1 && strcmp(fullname, name) == 0 && (entries[i].attr & 0x3F) == 0x10) return &entries[i];  // DIRECTORY
    
    }

    if (mode == 0){ // FILE
        printf("\nERROR: Unknown file name: \"", RED);
        printf(name, WHITE);
        printf("\"!", RED); 
        return NULL;
    }
    else if (mode == 1){ // DIRECTORY
        printf("\nERROR: Unknown directory name: \"", RED);
        printf(name, WHITE);
        printf("\"!", RED); 
        return NULL;
    }

    return NULL;
}

uint16_t fat12_get_next_cluster(int drive, uint16_t cluster) {
    uint32_t offset = (cluster * 3) / 2;
    uint16_t entry = *(uint16_t*)&fat[drive][offset];

    if (cluster & 1) {
        entry >>= 4;
    } else {
        entry &= 0x0FFF;
    }

    return entry;
}

uint32_t fat12_get_entry_size(int drive, uint16_t cluster) {
    uint16_t current_cluster = cluster;
    uint32_t size = 0;

    while (current_cluster < 0xFF8 && current_cluster >= 0x002) {
        current_cluster = fat12_get_next_cluster(drive, current_cluster);
        size += fat_info[drive].sectors_per_cluster * fat_info[drive].bytes_per_sector; 
    }

    return size;
}

void fat12_load_entry(int drive, uint16_t cluster, uint32_t buff_size, uint8_t* buf){

    uint16_t current_cluster = cluster;
    uint32_t offset = 0;

    while (current_cluster < 0xFF8 && current_cluster >= 0x002 && offset < buff_size ) {

        // counting this cluster sectors
        uint32_t sector = fat_info[drive].data_start_sector + (current_cluster - 2) * fat_info[drive].sectors_per_cluster;

        // reading this sectors
        for (int i = 0; i < fat_info[drive].sectors_per_cluster; i++){
            if (offset >= buff_size) break;

            uint8_t head, track, sec;
            lba_to_chs(sector + i, &track, &head, &sec);

            floppy_read_sector(drive, track, head, sec, buf + offset);
            offset += 512;
        }

        current_cluster = fat12_get_next_cluster(drive, current_cluster);

    }

}

void fat12_set_cluster_value(int drive, uint16_t cluster, uint16_t value) {
    uint32_t offset = (cluster * 3) / 2;

    if (cluster & 1) {
        // непарний кластер
        fat[drive][offset] = (fat[drive][offset] & 0x0F) | ((value << 4) & 0xF0);
        fat[drive][offset + 1] = (value >> 4) & 0xFF;
    } else {
        // парний кластер
        fat[drive][offset] = value & 0xFF;
        fat[drive][offset + 1] = (fat[drive][offset + 1] & 0xF0) | ((value >> 8) & 0x0F);
    }
}

void fat12_list_dir() {
    fat12_dir_entry_t* entries = (fat12_dir_entry_t*)dir_buf;
    int max_entries = entries_count;

    if (max_entries > ROOT_ENTRIES_MAX) {
        max_entries = ROOT_ENTRIES_MAX;
    }

    for (int i = 0; i < max_entries; i++) {
        if (entries[i].filename[0] == 0x00) break;
        if ((uint8_t)entries[i].filename[0] == 0xE5) continue;
        if ((entries[i].attr & 0x0F) == 0x0F) continue;
        if (entries[i].attr == 0x00) continue;
        if (entries[i].attr == 0xFF) continue;
        
        char name[13];
        int k = 0;

        for (int j = 0; j < 8 && entries[i].filename[j] != ' '; j++)
            name[k++] = entries[i].filename[j];

        if (entries[i].ext[0] != ' ') {
            name[k++] = '.';
            for (int j = 0; j < 3 && entries[i].ext[j] != ' '; j++)
                name[k++] = entries[i].ext[j];
        }

        name[k] = 0;

        char size_buf[16];

        msleep(100);

        if ((entries[i].attr & 0x3F) == 0x10) {
            // DIR
            printf("\nDIR  | ", WHITE);
            printf(name, YELLOW);
        } else if ((entries[i].attr & 0x3F) == 0x20) {
            // FILE
            printf("\nFILE | ", WHITE);
            printf(name, YELLOW);

            for (int m = 0; m < 15 - strlen(name); m++){
                printf(" ", 0);
            }

            printf(" - Size: ", WHITE);

            uint32_t size = entries[i].size;
            if (size >= 1024 * 1024) {
                itoa(size / (1024 * 1024), size_buf, 10);
                printf(size_buf, GREEN);
                printf(" MB", GREEN);
            } else if (size >= 1024) {
                itoa(size / 1024, size_buf, 10);
                printf(size_buf, GREEN);
                printf(" KB", GREEN);
            } else {
                itoa(size, size_buf, 10);
                printf(size_buf, GREEN);
                printf(" B", GREEN);
            }
        }
    }
    printf("\n", 0);
}

int floppy_open_file(char* name){
    int drive;

    if (strncmp(path[2], "flp0", 4) == 0){
        drive = 0;
    }else if (strncmp(path[2], "flp1", 4) == 0){
        drive = 1;
    }else{
        return -1;
    }

    int device = detect_device();

    fat12_dir_entry_t entry;

    fat12_dir_entry_t* entries = (fat12_dir_entry_t*)dir_buf;
    fat12_dir_entry_t* found = find_entry(entries, entries_count, name, 0);

    if (found != NULL) memcpy(&entry, found, sizeof(fat12_dir_entry_t));
    else return -1;

    if (file_buf){
      kfree(file_buf); 
      file_buf = NULL;
    }

    file_buf = kmalloc(entry.size + 1);

    if (!file_buf){
        printf("ERROR: Not enough memory to open file \"", RED);
        printf(name, WHITE);
        printf("\"!", RED);
        return -1;
    }

    devices[device].file = file_buf;
    devices[device].file_size = entry.size;

    fat12_load_entry(drive, entry.first_cluster, entry.size, file_buf);

    return 1;
}

int floppy_close_file(char* name){
    (void)name;
    if (file_buf){
        kfree(file_buf);
        file_buf = NULL;
    }
    return 0;
}

int floppy_create_entry(char* name, int mode){
    (void)name;
    (void)mode;

    return 1;
}

// int floppy_create_entry(char* name, int mode){
//     if (strlen(name) > 12 && mode == 0){
//         printf("\nERROR: Too long name!\n", RED);
//         return -1;
//     }
//     if (strlen(name) > 8 && mode == 1){
//         printf("\nERROR: Too long name!\n", RED);
//         return -1;
//     }

//     int drive = 0;

//     if (strncmp(devices[detect_device()].name, "flp1", 4) == 0){
//         drive = 1;
//     }

    
//     int new_cluster = 0xFFFFFF;

//     int total_clusters = (fat_info[drive].total_sectors - fat_info[drive].data_start_sector) / fat_info[drive].sectors_per_cluster;

//     for (uint16_t cluster = 2; cluster < total_clusters; cluster++) {
//         if (fat12_get_next_cluster(drive, cluster) == 0x000){
//             new_cluster = cluster;
//             break;
//         }
//     }       
//     if (new_cluster == 0xFFFFFF){
//         printf("ERROR: There is no more free clusters in FAT!\n", RED);
//         return -1;
//     }
    

//     if (mode == 0){ // FILE

//     }
//     else{ // FOLDER
//         fat12_dir_entry_t* new_entry;

//         fat12_dir_entry_t* entries = (fat12_dir_entry_t*)dir_buf;
//         for (uint32_t i = 0; i < entries_count; i++) {
//             if ((uint8_t)entries[i].filename[0] == 0x00 || (uint8_t)entries[i].filename[0] == 0xE5) {
                
//                 new_entry = &entries[i];

//                 memset(new_entry, 0, sizeof(fat12_dir_entry_t));

//                 memcpy(new_entry->filename, name, 8);
//                 memcpy(new_entry->ext, "   ", 3);
//                 new_entry->attr = 0x10;
//                 new_entry->first_cluster = new_cluster;
//                 new_entry->time = 0;
//                 new_entry->date = 0;
//                 new_entry->size = 0;

//                 fat12_set_cluster_value(drive, new_cluster, 0xFF8);

//                 for (int copy = 0; copy < fat_info[drive].fat_count; copy++) {
//                     for (uint32_t i = 0; i < fat_info[drive].fat_size_sectors; i++) {
//                         uint32_t lba = fat_info[drive].fat_start_sector + copy * fat_info[drive].fat_size_sectors + i;
//                         uint8_t head, track, sector;
//                         lba_to_chs(lba, &track, &head, &sector);

//                         floppy_write_sector(
//                             drive,
//                             track,
//                             head,
//                             sector,
//                             fat[drive] + i * fat_info[drive].bytes_per_sector
//                         );
//                     }
//                 }

//                 if (entries_count == 224){
//                     for (int i = 0; i < fat_info[drive].root_dir_sectors; i++){
//                         uint32_t lba = fat_info[drive].root_dir_start_sector + i;
//                         uint8_t head, track, sector;
//                         lba_to_chs(lba, &track, &head, &sector);

//                         floppy_write_sector(
//                             drive,
//                             track,
//                             head,
//                             sector,
//                             dir_buf + i * fat_info[drive].bytes_per_sector
//                         );
//                     }
//                 }
//                 else{
//                     uint32_t size = fat12_get_entry_size(drive, new_entry->first_cluster);
//                     for (uint32_t i = 0; i < size / fat_info[drive].bytes_per_sector; i++){
//                         uint32_t lba = fat_info[drive].data_start_sector + (new_entry->first_cluster - 2) * fat_info[drive].sectors_per_cluster + i;
//                         uint8_t head, track, sector;
//                         lba_to_chs(lba, &track, &head, &sector);

//                         floppy_write_sector(
//                             drive,
//                             track,
//                             head,
//                             sector,
//                             dir_buf + i * fat_info[drive].bytes_per_sector
//                         );
//                     }
//                 }
//                 return 1;
//             }
//         }

//     }

//     return 1;
// }

int floppy_remove_entry(char* name, int mode){
    (void)name;
    (void)mode;
    return 1;
}

int floppy_write_file(char* name, uint8_t* buffer){
    (void)name;
    (void)buffer;
    return 1;
}

int floppy_list(char path_list[32][13]){

    (void) path_list;

    fat12_list_dir(); // !!!

    return 0;
}

int floppy_load_dir(char path_list[32][13]){
    int drive = -1;

    uint32_t size = 0;

    if (strncmp(path_list[2], "flp0", 4) == 0){
        drive = 0;
    }else{
        drive = 1;
    }

    if (path_list[3][0] == '\0'){
        if (!dir_buf) kfree(dir_buf);
        dir_buf = kmalloc(ENTRY_SIZE * ROOT_ENTRIES_MAX);
        memcpy(dir_buf, root_dir_buf[drive], ENTRY_SIZE * ROOT_ENTRIES_MAX);
        entries_count = ROOT_ENTRIES_MAX;
    }else{
        kfree(dir_buf);
        dir_buf = kmalloc(ENTRY_SIZE * ROOT_ENTRIES_MAX);
        memcpy(dir_buf, root_dir_buf[drive], ENTRY_SIZE * ROOT_ENTRIES_MAX);
        int count = 224;
        entries_count = count;

        fat12_dir_entry_t entry; 

        int i = 3;
        while (path_list[i][0] != '\0'){
            fat12_dir_entry_t* entries = (fat12_dir_entry_t*)dir_buf;
            fat12_dir_entry_t* found = find_entry(entries, count, path_list[i], 1);
            if (found != NULL) memcpy(&entry, found, sizeof(fat12_dir_entry_t));
            else return -1;

            size = fat12_get_entry_size(drive, entry.first_cluster);

            kfree(dir_buf);
            dir_buf = kmalloc(size);

            count = size / sizeof(fat12_dir_entry_t);
            entries_count = count;

            fat12_load_entry(drive, entry.first_cluster, size, dir_buf);

            i++;
        }
    }
    return 0;
}
