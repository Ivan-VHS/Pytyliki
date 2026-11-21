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
    assert(pl_ui_preferred_target_px() == 44u);
    assert(isfinite(pl_ui_contrast_ratio_rgb(0,0,0,0,0,0)));
    return 0;
}
