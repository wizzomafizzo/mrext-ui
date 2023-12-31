#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <regex.h>
#include <stdio.h>

#include "fbg/fbgraphics.h"
#include "fbg/fbdev/fbg_fbdev.h"
#include "cwalk/cwalk.h"

#include "console.h"
#include "assets/assets.h"
#include "scanners.h"
#include "input.h"

#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MAX(x, y) (((x) < (y)) ? (y) : (x))

#define TEXT_COLOR 255, 255, 255
#define DIALOG_BG_COLOR 50, 50, 50
#define DIALOG_BORDER_COLOR 100, 100, 100

enum menu_types {
    MENU_TYPE_UP,
    MENU_TYPE_DIR,
    MENU_TYPE_IMAGE,
};

struct menu_entry {
    char display_name[256];
    char data[256];
    enum menu_types type;
};

struct screenshots_state {
    char active_screenshot_path[4097];
    struct _fbg_img *active_screenshot;
    char active_path[4097];
    struct dir active_dir;
    int selected_item;
    int menu_entry_count;
    struct menu_entry **menu_entries;
};

void draw_base(struct _fbg *fbg, struct _fbg_img *background_image) {
    fbg_imageClip(fbg, background_image, 0, 0, 0, 0,
                  MIN(fbg->width, (int) background_image->width),
                  MIN(fbg->height, (int) background_image->height));

    fbg_rect(fbg, 25, 25, fbg->width - 50, fbg->height - 50, DIALOG_BG_COLOR);
    fbg_rect(fbg, fbg->width - 25 - 350 - 8 - 8, 25, 350 + 8 + 8, 295 + 8 + 8, 25, 25, 25);

    fbg_hline(fbg, 25, 25, fbg->width - 50, DIALOG_BORDER_COLOR);
    fbg_hline(fbg, 25, fbg->height - 25, fbg->width - 49, DIALOG_BORDER_COLOR);
    fbg_vline(fbg, 25, 25, fbg->height - 50, DIALOG_BORDER_COLOR);
    fbg_vline(fbg, fbg->width - 25, 25, fbg->height - 50, DIALOG_BORDER_COLOR);

    fbg_vline(fbg, fbg->width - 25 - 350 - 8 - 8, 25, fbg->height - 50, DIALOG_BORDER_COLOR);
}

void draw_items(struct _fbg *fbg, struct screenshots_state *state) {
    fbg_textColor(fbg, TEXT_COLOR);

    int offset_left = 25 + (2 * 8);
    int offset_top = 25 + 8;
    int spacing = 20;

    if (strcmp(state->active_path, SCREENSHOTS_DEFAULT) == 0) {
        spacing = 10;
    }

    regex_t name_match;
    int result;

    result = regcomp(&name_match, "^([0-9]{4})([0-9]{2})([0-9]{2})_([0-9]{2})([0-9]{2})([0-9]{2})-(.+)\\.png$",
                     REG_EXTENDED);
    if (result != 0) {
        die("regcomp");
    }

    for (int i = 0; i < state->menu_entry_count; i++) {
        regmatch_t matches[8];
        result = regexec(&name_match, state->menu_entries[i]->display_name, 8, matches, 0);
        if (result == 0) {
            char new_name[256];

            char year[5], month[3], day[3], hour[3], minute[3], second[3];
            strncpy(year, &state->menu_entries[i]->display_name[matches[1].rm_so], 4);
            year[4] = '\0';
            strncpy(month, &state->menu_entries[i]->display_name[matches[2].rm_so], 2);
            month[2] = '\0';
            strncpy(day, &state->menu_entries[i]->display_name[matches[3].rm_so], 2);
            day[2] = '\0';
            strncpy(hour, &state->menu_entries[i]->display_name[matches[4].rm_so], 2);
            hour[2] = '\0';
            strncpy(minute, &state->menu_entries[i]->display_name[matches[5].rm_so], 2);
            minute[2] = '\0';
            strncpy(second, &state->menu_entries[i]->display_name[matches[6].rm_so], 2);
            second[2] = '\0';

            char game_name[236];
            strncpy(game_name, &state->menu_entries[i]->display_name[matches[7].rm_so],
                    matches[7].rm_eo - matches[7].rm_so);
            game_name[matches[7].rm_eo - matches[7].rm_so] = '\0';

            if (strlen(game_name) > 25) {
                game_name[25] = '\0';
            }

            sprintf(new_name, "%s-%s-%s %s:%s:%s\n%s", year, month, day, hour, minute, second, game_name);


            fbg_write(fbg, new_name, offset_left, offset_top + (i * spacing));
        } else {
            if (state->menu_entries[i]->type == MENU_TYPE_UP) {
                fbg_write(fbg, "..               <UP-DIR>", offset_left, offset_top + (i * spacing));
            } else {
                fbg_write(fbg, state->menu_entries[i]->display_name, offset_left, offset_top + (i * spacing));
            }
        }

    }

    regfree(&name_match);

    fbg_write(fbg, ">", offset_left - 8 - 5, offset_top + (state->selected_item * spacing));
}

void draw_screenshot(struct _fbg *fbg, struct screenshots_state *state) {
    unsigned int max_width = 350;
    unsigned int max_height = 295;

    if (state->active_screenshot != NULL) {
        struct _fbg_img *sc = state->active_screenshot;

        if (sc->width > max_width || sc->height > max_height) {
            float scale_factor = 1;

            if (sc->width > sc->height) {
                scale_factor = (float) max_width / (float) sc->width;
            }

            if (sc->width < sc->height) {
                scale_factor = (float) max_height / (float) sc->height;
            }

            if ((float) sc->width * scale_factor > (float) max_width) {
                scale_factor = (float) max_width / (float) sc->width;
            }

            if ((float) sc->height * scale_factor > (float) max_height) {
                scale_factor = (float) max_height / (float) sc->height;
            }

            unsigned int top =
                    25 + 8 + (max_height - (int) ((float) sc->height * scale_factor)) / 2;
            unsigned int left_margin = (max_width - (int) ((float) sc->width * scale_factor)) / 2;
            unsigned int left = fbg->width - 25 - 8 - max_width + left_margin;

            fbg_imageEx(fbg, sc, (int) left, (int) top, scale_factor, scale_factor,
                        0, 0, (int) sc->width, (int) sc->height);
        } else {
            unsigned int top = 25 + 8 + (max_height - (int) sc->height) / 2;
            unsigned int left_margin = (max_width - (int) sc->width) / 2;
            unsigned int left = fbg->width - 25 - 8 - max_width + left_margin;

            fbg_imageClip(fbg, sc, (int) left, (int) top, 0, 0, (int) sc->width, (int) sc->height);
        }
    }
}

void try_load_image(struct _fbg *fbg, struct screenshots_state *state) {
    if (state->selected_item < 0 || state->selected_item >= state->menu_entry_count) {
        return;
    }

    struct menu_entry *item = state->menu_entries[state->selected_item];

    if (item->type != MENU_TYPE_IMAGE) {
        if (state->active_screenshot != NULL) {
            fbg_freeImage(state->active_screenshot);
            state->active_screenshot = NULL;
        }

        return;
    }

    cwk_path_join(state->active_dir.path, item->data, state->active_screenshot_path, 4097);
    state->active_screenshot = fbg_loadPNG(fbg, state->active_screenshot_path);
}

void build_menu(struct screenshots_state *state) {
    int total_items =
            state->active_dir.dir_count + state->active_dir.image_count + (state->active_dir.can_go_up ? 1 : 0);
    state->menu_entry_count = total_items;

    state->menu_entries = malloc(total_items * sizeof(struct menu_entry *));
    int menu_index = 0;

    if (state->active_dir.can_go_up) {
        state->menu_entries[0] = malloc(sizeof(struct menu_entry));
        strcpy(state->menu_entries[0]->display_name, "..");
        strcpy(state->menu_entries[0]->data, "..");
        state->menu_entries[0]->type = MENU_TYPE_UP;
        menu_index++;
    }

    for (int i = 0; i < state->active_dir.dir_count; i++) {
        state->menu_entries[menu_index] = malloc(sizeof(struct menu_entry));
        strcpy(state->menu_entries[menu_index]->display_name, state->active_dir.dir_names[i]);
        strcpy(state->menu_entries[menu_index]->data, state->active_dir.dir_names[i]);
        state->menu_entries[menu_index]->type = MENU_TYPE_DIR;
        menu_index++;
    }

    for (int i = 0; i < state->active_dir.image_count; i++) {
        state->menu_entries[menu_index] = malloc(sizeof(struct menu_entry));
        strcpy(state->menu_entries[menu_index]->display_name, state->active_dir.image_names[i]);
        strcpy(state->menu_entries[menu_index]->data, state->active_dir.image_names[i]);
        state->menu_entries[menu_index]->type = MENU_TYPE_IMAGE;
        menu_index++;
    }
}

void free_menu(struct screenshots_state *state) {
    for (int i = 0; i < state->menu_entry_count; i++) {
        free(state->menu_entries[i]);
    }
    free(state->menu_entries);
}

void reset_state(struct screenshots_state *state) {
    free_menu(state);
    free_screenshots_dir(&state->active_dir);

    if (state->active_screenshot != NULL) {
        fbg_freeImage(state->active_screenshot);
        state->active_screenshot = NULL;
    }

    state->active_screenshot_path[0] = '\0';
}

void reload_path(struct _fbg *fbg, struct screenshots_state *state) {
    reset_state(state);
    read_screenshots_dir(&state->active_dir, state->active_path);
    build_menu(state);
    state->selected_item = 0;
    try_load_image(fbg, state);
}

void handle_action(enum actions action, struct _fbg *fbg, struct screenshots_state *state) {
    if (action == ACTION_NONE) {
        return;
    }

    if (state->menu_entry_count == 0) {
        return;
    }

    switch (action) {
        case ACTION_UP:
            if (state->selected_item > 0) {
                (state->selected_item)--;
                try_load_image(fbg, state);
            } else if (state->selected_item == 0) {
                (state->selected_item) = state->menu_entry_count - 1;
                try_load_image(fbg, state);
            }
            return;
        case ACTION_DOWN:
            if (state->selected_item < state->menu_entry_count - 1) {
                (state->selected_item)++;
                try_load_image(fbg, state);
            } else if (state->selected_item == state->menu_entry_count - 1) {
                (state->selected_item) = 0;
                try_load_image(fbg, state);
            }
            return;
        case ACTION_CONFIRM:
            if (state->menu_entries[state->selected_item]->type == MENU_TYPE_DIR) {
                cwk_path_join(state->active_path, state->menu_entries[state->selected_item]->data,
                              state->active_path, 4097);
                reload_path(fbg, state);
            } else if (state->menu_entries[state->selected_item]->type == MENU_TYPE_UP) {
                size_t length;
                cwk_path_get_dirname(state->active_path, &length);
                if (length == 0) {
                    return;
                }
                state->active_path[length - 1] = '\0';
                reload_path(fbg, state);
            }
            return;
        default:
            return;
    }
}

void init_fb(struct _fbg *fbg, struct assets *assets) {
    fbg_clear(fbg, 0);
    fbg_flip(fbg);

    *assets = load_assets(fbg);

    fbg_clear(fbg, 0);
    fbg_draw(fbg);
    draw_base(fbg, assets->background_image);
    fbg_flip(fbg);
}

int main(void) {
    system("vmode -r 640 360 rgb32");
    enable_raw_mode();

    struct assets assets;
    struct _fbg *fbg = fbg_fbdevSetup(NULL, 0);
    if (fbg == NULL) {
        die("fbg_fbdevInit");
    }

    init_fb(fbg, &assets);

    struct screenshots_state state = {
            .selected_item = 0,
            .active_path = SCREENSHOTS_DEFAULT,
            .active_screenshot = NULL,
            .menu_entry_count = 0,
    };

    read_screenshots_dir(&state.active_dir, state.active_path);
    build_menu(&state);
    try_load_image(fbg, &state);

    enum actions action;

    while (true) {
        fbg_clear(fbg, 0);
        fbg_draw(fbg);

        draw_base(fbg, assets.background_image);
        draw_items(fbg, &state);
        draw_screenshot(fbg, &state);

        fbg_flip(fbg);

        action = read_key();
        if (action == ACTION_QUIT) {
            break;
        }
        handle_action(action, fbg, &state);
    }

    free_menu(&state);
    free_screenshots_dir(&state.active_dir);

    free_assets(&assets);
    fbg_close(fbg);

    return 0;
}
