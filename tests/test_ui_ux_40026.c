#include "ui_ux_policy.h"

#include <assert.h>
#include <math.h>

int main(void) {
    const double white_navy = pl_ui_contrast_ratio_rgb(
        255, 255, 255, 10, 30, 70
    );
    const double white_blue = pl_ui_contrast_ratio_rgb(
        255, 255, 255, 0, 57, 166
    );
    const double grey_white = pl_ui_contrast_ratio_rgb(
        128, 128, 128, 255, 255, 255
    );

    assert(white_navy >= 7.0);
    assert(white_blue >= 4.5);
    assert(grey_white < 4.5);
    assert(pl_ui_text_contrast_meets(255, 255, 255, 0, 57, 166, 0));
    assert(pl_ui_text_contrast_meets(128, 128, 128, 255, 255, 255, 1));
    assert(!pl_ui_text_contrast_meets(128, 128, 128, 255, 255, 255, 0));
    assert(pl_ui_preferred_target_px() == 44u);
    assert(isfinite(pl_ui_contrast_ratio_rgb(0, 0, 0, 0, 0, 0)));
    return 0;
}
