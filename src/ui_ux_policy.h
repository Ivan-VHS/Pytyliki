#ifndef PYTYLIKI_UI_UX_POLICY_H
#define PYTYLIKI_UI_UX_POLICY_H

#include <stdint.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

/* UI-only accessibility helpers; never call from DSP or processing workers. */
static inline double pl_ui_linear_channel(uint8_t value) {
    const double channel = (double)value / 255.0;
    return channel <= 0.04045
        ? channel / 12.92
        : pow((channel + 0.055) / 1.055, 2.4);
}

static inline double pl_ui_relative_luminance(uint8_t red, uint8_t green, uint8_t blue) {
    return 0.2126 * pl_ui_linear_channel(red)
         + 0.7152 * pl_ui_linear_channel(green)
         + 0.0722 * pl_ui_linear_channel(blue);
}

static inline double pl_ui_contrast_ratio_rgb(
    uint8_t fr, uint8_t fg, uint8_t fb,
    uint8_t br, uint8_t bg, uint8_t bb
) {
    const double foreground = pl_ui_relative_luminance(fr, fg, fb);
    const double background = pl_ui_relative_luminance(br, bg, bb);
    const double lighter = foreground > background ? foreground : background;
    const double darker = foreground > background ? background : foreground;
    return (lighter + 0.05) / (darker + 0.05);
}

static inline int pl_ui_text_contrast_meets(
    uint8_t fr, uint8_t fg, uint8_t fb,
    uint8_t br, uint8_t bg, uint8_t bb,
    int large_text
) {
    return pl_ui_contrast_ratio_rgb(fr, fg, fb, br, bg, bb) >= (large_text ? 3.0 : 4.5);
}

static inline unsigned int pl_ui_preferred_target_px(void) { return 44u; }

static inline int pl_ui_clamp_font_size(int value) {
    if (value < 10) return 10;
    if (value > 20) return 20;
    return value;
}

#ifdef __cplusplus
}
#endif

#endif
