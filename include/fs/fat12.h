#ifndef FAT12_H
#define FAT12_H



typedef struct {
    uint8_t  jmp[3];            
    char     oem[8];            
    uint16_t bytes_per_sector;  
    uint8_t  sectors_per_cluster; 
    uint16_t reserved_sectors;  
    uint8_t  fat_count;         
    uint16_t root_entries;      
    uint16_t total_sectors;     
    uint8_t  media_type;        
    uint16_t fat_size_sectors;  
    uint16_t sectors_per_track; 
    uint16_t head_count;        
    uint32_t hidden_sectors;    
    uint32_t large_sector_count;
} __attribute__((packed)) fat12_boot_sector_t;

typedef struct {
    uint16_t bytes_per_sector;
    uint8_t  sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t  fat_count;
    uint16_t root_entries;
    uint16_t total_sectors;
    uint16_t fat_size_sectors;
    uint16_t root_dir_sectors;
    uint32_t fat_start_sector;
    uint32_t root_dir_start_sector;
    uint32_t data_start_sector;
} fat12_info_t;

typedef struct {
    char     filename[8];     
    char     ext[3];          
    uint8_t  attr;            
    uint8_t  reserved[10];    
    uint16_t time;            
    uint16_t date;            
    uint16_t first_cluster;   
    uint32_t size;            
} __attribute__((packed)) fat12_dir_entry_t;




#endif