#include <drivers/shell.h>
#include <common.h>
#include <drivers/floppy_disk_controller_driver.h>

void printf();
char* scanf(char* text);

extern void cls();
extern void list_directory();
extern void create_folder();
extern void change_directory();
extern void print_current_directory();
extern int unmount_device();
extern void cat_file();

extern char path[32][13];
extern int path_counter;

short shell_mode =  1 ;

typedef void (*command_func)(const char* args);

typedef struct {
    const char* name;
    command_func func;
    const char* description;
} Command;

void cmd_help(const char* args);
void cmd_clear(const char* args);
void cmd_echo(const char* args);
void cmd_halt(const char* args);
void cmd_info(const char* args);
void cmd_dir(const char* args);
void cmd_cd(const char* args);
void cmd_pwd(const char* args);
void cmd_cat(const char* args);
void cmd_unmount(const char* args);

Command commands[] = {
    {"help", cmd_help, "Show list of commands"},
    {"info", cmd_info, "Show system info"},
    {"clear", cmd_clear, "Clear screen"},
    {"echo", cmd_echo, "Output text"},
    {"halt", cmd_halt, "Halt system"},
    {"fd", detect_floppy_drives, "Show all floppy drives"},
    {"dir", cmd_dir, "Show current directory"},
    {"cd", cmd_cd, "Change directory"},
    {"pwd", cmd_pwd, "Print current path"},
    {"mkdir", create_folder, "Create new directory"},
    {"cat", cmd_cat, "Print file content"},
    {"unmount", cmd_unmount, "Unmount device \"name\""}
};

const int commands_count = sizeof(commands) / sizeof(Command);

void interpret(const char* input) {
    char command[64];
    const char* args = NULL;

    int i = 0;
    while (input[i] && input[i] != ' ' && i < 63) {
        command[i] = input[i];
        i++;
    }
    command[i] = '\0';

    if (input[i] == ' ')
        args = &input[i + 1];

    // Пошук у списку команд
    for (int j = 0; j < commands_count; j++) {
        if (strcmp(command, commands[j].name) == 0) {
            commands[j].func(args ? args : "");
            return;
        }
    }
    if (strcmp(command, "")){
        printf("Command ", RED);
        printf(command, RED);
        printf(" not found!", RED);
    }
}

void read_command() {
    printf("\n", 0);
    printf("kenel:", GREEN);
    print_current_directory();
    char* input = scanf("$ ");
    interpret(input);
}

void cmd_help(const char* args) {
    (void)args;
    printf("\n", WHITE);
    for (int i = 0; i < commands_count; i++) {
        printf("   * ", WHITE);
        printf(commands[i].name, WHITE);
        printf(" --- ", WHITE);
        printf(commands[i].description, GREEN);
        printf("\n", WHITE);
    }
}

void cmd_clear(const char* args) {
    (void)args;
    cls();
}

void cmd_echo(const char* args) {
    printf(args, CYAN);
}

void cmd_halt(const char* args) {
    (void)args;
    printf("Halt...\n", RED);
    asm volatile ("hlt");
}

void cmd_info(const char* args) {
    (void)args;
    printf("\n  /\\       /\\\n /  \"\"\"\"\"/  \\\n|  \\/\\\"\"\"/\\/  |\n`, \"/ ,`\n====== Y ======\n  \\   -^-   /\n   \\       / \\__,\n  /  `````       \\______,\n |    ```         \" \" \"  \\,\n |     `           \" \"     \\\n |            |     \"    \"  \\\n |    _      /            \"  |\n | \" / \\ \"  /              \" |\n |   | |   |\\__ _______\\    \"|\n/ -  | | -  \\ /   \"   \"   \" /\n\\___/   \\___/ \\____________/   \n\n", WHITE);
    printf("CAT OS version - ", WHITE);
    printf(OS_VER, WHITE);
}

void cmd_dir(const char* args){
    (void)args;
    list_directory(path);
}

void cmd_cd(const char* args){
    change_directory(args);
}

void cmd_pwd(const char* args){
    (void)args;
    print_current_directory();
}

void cmd_cat(const char* args){
    cat_file(args);
}

void cmd_unmount(const char* args){
    unmount_device(args);
}