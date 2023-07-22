#include "fbg/fbgraphics.h"
#include "image_background.h"
#include "image_font.h"
#include "assets.h"

struct assets load_assets(struct _fbg *fbg) {
    struct assets assets;

    assets.background_image = fbg_loadImageFromMemory(
            fbg,
            _image_background,
            (int) _image_background_len
    );
    assets.font_image = fbg_loadImageFromMemory(fbg, _image_font, (int) _image_font_len);
    assets.font = fbg_createFont(fbg, assets.font_image, 8, 8, 33);

    return assets;
}

void free_assets(struct assets *assets) {
    fbg_freeImage(assets->background_image);
    fbg_freeImage(assets->font_image);
    fbg_freeFont(assets->font);
}
