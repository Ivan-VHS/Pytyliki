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
    assert(pl_ui_contrast_ratio_rgb(255,243,226,33,25,19) >= 7.0);
    assert(pl_ui_contrast_ratio_rgb(196,169,138,33,25,19) >= 4.5);
    assert(pl_ui_contrast_ratio_rgb(33,25,19,213,139,61) >= 4.5);
    assert(pl_ui_contrast_ratio_rgb(255,243,226,182,74,64) >= 4.5);
    assert(pl_ui_contrast_ratio_rgb(255,243,226,82,99,71) >= 4.5);
    assert(pl_ui_preferred_target_px() == 44u);
    assert(pl_ui_clamp_font_size(14) == 14);
    assert(pl_ui_clamp_font_size(5) == 10);
    assert(pl_ui_clamp_font_size(50) == 20);
    assert(isfinite(pl_ui_contrast_ratio_rgb(0,0,0,0,0,0)));
    return 0;
}
