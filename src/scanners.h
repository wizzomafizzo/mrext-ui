#ifndef RADICAL_SCANNERS_H
#define RADICAL_SCANNERS_H

#define SCREENSHOTS_DEFAULT "/media/fat/screenshots"

struct dir {
    char *path;
    bool can_go_up;
    int dir_count;
    char **dir_names;
    int image_count;
    char **image_names;
};

bool is_png(const char *path);

void read_screenshots_dir(struct dir *dir, const char *path);

void free_screenshots_dir(struct dir *dir);

#endif //RADICAL_SCANNERS_H
