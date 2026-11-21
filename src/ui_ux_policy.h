#ifndef PYTYLIKI_UI_UX_POLICY_H
#define PYTYLIKI_UI_UX_POLICY_H

#include <stdint.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Shared, UI-only accessibility helpers. Never call these from DSP/worker paths. */
static inline double pl_ui_linear_channel(uint8_t value) {
    const double channel = (double)value / 255.0;
    return channel <= 0.04045
        ? channel / 12.92
        : pow((channel + 0.055) / 1.055, 2.4);
}

static inline double pl_ui_relative_luminance(
    uint8_t red, uint8_t green, uint8_t blue
) {
    return 0.2126 * pl_ui_linear_channel(red)
         + 0.7152 * pl_ui_linear_channel(green)
         + 0.0722 * pl_ui_linear_channel(blue);
}

static inline double pl_ui_contrast_ratio_rgb(
    uint8_t foreground_red, uint8_t foreground_green, uint8_t foreground_blue,
    uint8_t background_red, uint8_t background_green, uint8_t background_blue
) {
    const double foreground = pl_ui_relative_luminance(
        foreground_red, foreground_green, foreground_blue
    );
    const double background = pl_ui_relative_luminance(
        background_red, background_green, background_blue
    );
    const double lighter = foreground > background ? foreground : background;
    const double darker = foreground > background ? background : foreground;
    return (lighter + 0.05) / (darker + 0.05);
}

static inline int pl_ui_text_contrast_meets(
    uint8_t foreground_red, uint8_t foreground_green, uint8_t foreground_blue,
    uint8_t background_red, uint8_t background_green, uint8_t background_blue,
    int large_text
) {
    const double required = large_text ? 3.0 : 4.5;
    return pl_ui_contrast_ratio_rgb(
        foreground_red, foreground_green, foreground_blue,
        background_red, background_green, background_blue
    ) >= required;
}

/* Preferred target size for touch and frequent high-precision actions. */
static inline unsigned int pl_ui_preferred_target_px(void) {
    return 44u;
}

#ifdef __cplusplus
}
#endif

#endif /* PYTYLIKI_UI_UX_POLICY_H */
