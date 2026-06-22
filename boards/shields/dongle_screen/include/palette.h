#pragma once

/* Snazzy color palette (https://github.com/sindresorhus/iterm2-snazzy) */
/* NOTE: background is forced to pure black instead of Snazzy's #282a36 — the
 * charcoal reads as a washed slate-blue on this ST7789 TFT (looks better on OLED). */
#define SNAZZY_BLACK   0x000000
#define SNAZZY_WHITE   0xeff0eb
#define SNAZZY_DIM     0x6272a4 /* muted slate for dim roller rows + inactive mods (tunable) */
#define SNAZZY_RED     0xff5c57
#define SNAZZY_GREEN   0x5af78e
#define SNAZZY_YELLOW  0xf3f99d
#define SNAZZY_BLUE    0x57c7ff
#define SNAZZY_MAGENTA 0xff6ac1
#define SNAZZY_CYAN    0x9aedfe

/* lowercase hex strings for LVGL recolor markup (#RRGGBB text#) */
#define SNAZZY_GREEN_STR "5af78e"
#define SNAZZY_BLUE_STR  "57c7ff"
#define SNAZZY_DIM_STR   "6272a4"
#define SNAZZY_WHITE_STR "eff0eb"
