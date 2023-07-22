#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>

#include "console.h"
#include "scanners.h"

bool is_png(const char *filename) {
    return strstr(filename, ".png") != NULL;
}

void read_screenshots_dir(struct dir *dir, const char *path) {
    struct dirent **file_list;

    int file_count = scandir(path, &file_list, NULL, alphasort);
    if (file_count < 0) {
        // TODO: this could be handled
        die(strerror(errno));
    }

    dir->path = malloc(strlen(path) + 1);
    strcpy(dir->path, path);

    if (strcmp(path, SCREENSHOTS_DEFAULT) != 0) {
        dir->can_go_up = true;
    } else {
        dir->can_go_up = false;
    }

    dir->dir_count = 0;
    for (int i = 0; i < file_count; i++) {
        if (strcmp(file_list[i]->d_name, ".") == 0 || strcmp(file_list[i]->d_name, "..") == 0) {
            continue;
        }

        if (file_list[i]->d_type == DT_DIR) {
            dir->dir_count++;
        }
    }

    dir->dir_names = malloc(dir->dir_count * sizeof(char *));
    int dir_index = 0;
    for (int i = 0; i < file_count; i++) {
        if (strcmp(file_list[i]->d_name, ".") == 0 || strcmp(file_list[i]->d_name, "..") == 0) {
            continue;
        }

        if (file_list[i]->d_type == DT_DIR) {
            dir->dir_names[dir_index] = malloc(strlen(file_list[i]->d_name) + 1);
            strcpy(dir->dir_names[dir_index], file_list[i]->d_name);
            dir_index++;
        }
    }

    dir->image_count = 0;
    for (int i = 0; i < file_count; i++) {
        if (file_list[i]->d_type == DT_REG && is_png(file_list[i]->d_name)) {
            dir->image_count++;
        }
    }

    dir->image_names = malloc(dir->image_count * sizeof(char *));
    int image_index = 0;
    for (int i = 0; i < file_count; i++) {
        if (file_list[i]->d_type == DT_REG && is_png(file_list[i]->d_name)) {
            dir->image_names[image_index] = malloc(strlen(file_list[i]->d_name) + 1);
            strcpy(dir->image_names[image_index], file_list[i]->d_name);
            image_index++;
        }
    }

    for (int i = 0; i < file_count; i++) {
        free(file_list[i]);
    }
    free(file_list);
}

void free_screenshots_dir(struct dir *dir) {
    free(dir->path);

    for (int i = 0; i < dir->dir_count; i++) {
        free(dir->dir_names[i]);
    }
    free(dir->dir_names);

    for (int i = 0; i < dir->image_count; i++) {
        free(dir->image_names[i]);
    }
    free(dir->image_names);
}
