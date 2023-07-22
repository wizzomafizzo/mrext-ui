#ifndef RADICAL_ASSETS_H
#define RADICAL_ASSETS_H

struct assets {
    struct _fbg_img *background_image;
    struct _fbg_img *font_image;
    struct _fbg_font *font;
};

struct assets load_assets(struct _fbg *fbg);

void free_assets(struct assets *assets);

#endif //RADICAL_ASSETS_H
