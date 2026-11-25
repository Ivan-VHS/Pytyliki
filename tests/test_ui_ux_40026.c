#include "ui_ux_policy.h"

#include <assert.h>
#include <math.h>

int main(void) {
    assert(pl_ui_contrast_ratio_rgb(255,255,255,10,30,70) >= 7.0);
    assert(pl_ui_contrast_ratio_rgb(255,255,255,0,57,166) >= 4.5);
    assert(pl_ui_contrast_ratio_rgb(128,128,128,255,255,255) < 4.5);
    assert(pl_ui_text_contrast_meets(255,255,255,0,57,166,0));
    assert(pl_ui_text_contrast_meets(128,128,128,255,255,255,1));
    assert(!pl_ui_text_contrast_meets(128,128,128,255,255,255,0));
    /* Mora Amber: strong body text and legible action/critical surfaces. */
    assert(pl_ui_contrast_ratio_rgb(255,255,255,5,5,5) >= 7.0);
    assert(pl_ui_contrast_ratio_rgb(166,166,166,5,5,5) >= 4.5);
    assert(pl_ui_contrast_ratio_rgb(8,8,8,255,215,0) >= 4.5);
    assert(pl_ui_contrast_ratio_rgb(255,255,255,196,58,69) >= 4.5);
    assert(pl_ui_contrast_ratio_rgb(8,8,8,46,173,104) >= 4.5);
    assert(pl_ui_contrast_ratio_rgb(255,215,0,5,5,5) >= 4.5);
    assert(pl_ui_contrast_ratio_rgb(255,107,114,5,5,5) >= 4.5);
    assert(pl_ui_contrast_ratio_rgb(108,168,255,5,5,5) >= 4.5);
    assert(pl_ui_preferred_target_px() == 44u);
    assert(pl_ui_clamp_font_size(14) == 14);
    assert(pl_ui_clamp_font_size(5) == 10);
    assert(pl_ui_clamp_font_size(50) == 20);
    assert(isfinite(pl_ui_contrast_ratio_rgb(0,0,0,0,0,0)));
    return 0;
}
