# AGENTS.md

Guidance for AI agents (and humans) working in this repo. **User-facing docs
(install, pairing, config option table) live in `README.md` — this file is the
implementation/working reference.** When they disagree, the code wins; fix both.

## What this repo is

A **ZMK module/shield** named **`dongle_screen`** that renders a status screen on
a **Sitronix ST7789V TFT LCD** attached to a wireless dongle. It is a *module*,
not a standalone firmware: it is consumed by a separate `zmk-config` build via
`-DZMK_EXTRA_MODULES=.../zmk-dongle-screen` and `-DSHIELD="<base_shield> dongle_screen"`.

Supported dongle boards (per-board overlays under `boards/shields/dongle_screen/boards/`):
`seeeduino_xiao_ble` and `nice_nano_v2` (both nRF52840).

## Build & verification — READ FIRST

- **There is no build toolchain in this checkout** (no `west`, no Zephyr/LVGL
  headers). **LSP `file not found` errors for `lvgl.h`, `zephyr/*.h`, `zmk/*.h`
  are EXPECTED — they are not real errors.** Do not "fix" includes to silence them.
- **You cannot compile here.** Local verification is static only:
  byte-check font/glyph escapes, balanced braces, recolor-markup syntax, and
  widget overlap math. **Always state clearly that the real test is an on-device
  build + flash.**
- Real builds happen in the user's `zmk-config` (GitHub Actions `build.yaml`) or
  via a local ZMK toolchain. Documented local command (see `README.md`):
  ```sh
  west build -p -s /workspaces/zmk/app -b seeeduino_xiao_ble \
    -- -DZMK_CONFIG=/workspaces/zmk-config/config \
       -DSHIELD="<base_shield> dongle_screen" \
       -DZMK_EXTRA_MODULES=/path/to/zmk-dongle-screen/
  ```
- `config/west.yml` only imports upstream `zmkfirmware/zmk@main`. No CI workflow
  is checked into this repo.

## Layout & geometry (most-tweaked area)

- **Panel is native 240×280 portrait, `y-offset=20`** (`boards/.../*.overlay`).
  Orientation is set at **runtime** in `src/screen_rotate_init.c`: default
  "horizontal" = `DISPLAY_ORIENTATION_ROTATED_270` ⇒ **effective drawing surface
  is 280 wide × 240 tall (landscape).** Use 280×240 for layout math.
- **All widget placement is in `src/custom_status_screen.c`**, one block per
  widget: `lv_obj_set_size(obj, W, H)` + `lv_obj_align(obj, ANCHOR, x, y)`. An
  ASCII diagram of the zones is kept in that file's comment — update it when you
  move things.
- LVGL offsets: **`+x` = right, `+y` = down**, relative to the anchor. With
  `RIGHT_MID`/`TOP_RIGHT` the inward direction is negative `x`; with `LEFT_MID`,
  `y` is measured from vertical center (negative = up).
- **Current zones (overlap-free):**

  | Zone | Size | Align | Offset | Notes |
  |---|---|---|---|---|
  | Connections (output) | 170×36 | `TOP_RIGHT` | `-14, 6` | right edge x≈266 |
  | Layer roller | 136×150 | `LEFT_MID` | `14, -22` | centered in band [0,195] |
  | Modifier 2×2 | 110×100 | `RIGHT_MID` | `-14, 0` | y[70,170] |
  | Battery bars | 280×45 | `BOTTOM_MID` | `0, 0` | y[195,240] |

- **Overlap guardrail:** roller right edge `= x + width` (currently 150) must
  stay `<` mods left edge `= 280 − right_margin − width` (currently 156) — keep a
  gap (currently 6px). Re-check the table above after any nudge.
- `wpm_status` widget exists but is **not placed** on the screen (config
  `CONFIG_DONGLE_SCREEN_WPM_ACTIVE` is unused in `custom_status_screen.c`).

## Widgets (`src/widgets/`)

| File | Renders | Update model |
|---|---|---|
| `output_status.c` | USB + 2 BLE profile glyphs (one-liner) | **event** (`endpoint_changed`, `ble_active_profile_changed`, `usb_conn_state_changed`) |
| `mod_status.c` | 2×2 modifier grid (Ctrl/Shift/Alt/GUI) | **polling `k_timer`, every 100 ms** |
| `layer_roller.c` | LVGL roller of layer names, selected row emphasized | **event** (`zmk_layer_state_changed`) |
| `battery_bar.c` | per-peripheral battery bars + % | **event** (`peripheral_battery_state_changed`, `split_central_status_changed`) |
| `wpm_status.c` | WPM label (not currently placed) | **event** (`zmk_wpm_state_changed`) |

- **Adding a widget:** create `src/widgets/foo.{c,h}`, **add the `.c` to
  `boards/shields/dongle_screen/CMakeLists.txt`** (widget sources are listed
  explicitly), then init + place it in `custom_status_screen.c` (gate with a
  `CONFIG_*` if optional).

## Fonts (`src/fonts/`, `include/fonts.h`)

- **Font `.c` files are auto-globbed** by CMake
  (`file(GLOB font_sources src/fonts/*.c)`), so a new font `.c` needs **no** CMake
  edit. Declare it in `include/fonts.h` with `LV_FONT_DECLARE(...)`.
- **Each generated font `.c` embeds the exact `lv_font_conv` command on line 4 —
  that comment is the source of truth for its codepoint ranges.** Regenerate with
  `npx lv_font_conv@1.5.3`. Examples (TTF path is user-specific):
  ```sh
  # Connections font (ASCII + USB + BLE circles)
  npx lv_font_conv@1.5.3 --no-compress --no-prefilter --bpp 4 --size 28 \
    --font /Users/thanh/personal/arch-pkgs/fonts/ttf-samsung-sans-nerd/SamsungSansNerdFont-Regular.ttf \
    -r 0x20-0x7E -r 0xF0553 -r 0xF0CA0-0xF0CA3 \
    --format lvgl --force-fast-kern-format --lv-font-name SamsungSans_Regular_28 \
    -o boards/shields/dongle_screen/src/fonts/SamsungSans_Regular_28.c

  # Modifier-icon font (ASCII + mod glyphs)
  # ranges: 0x20-0x7E -r 0xE62A -r 0xF033D -r 0xF0633-0xF0636  (size 40)
  ```
- **Font inventory:**

  | Font | Purpose | Ranges |
  |---|---|---|
  | `SamsungSans_Medium_40` | roller: selected layer | (layer text) |
  | `SamsungSans_Thin_40` | roller: unselected layers | (layer text) |
  | `SamsungSans_Regular_40` | modifier icons + ASCII | `0x20-0x7E`, `E62A`, `F033D`, `F0633-F0636` |
  | `SamsungSans_Regular_28` | connections one-liner | `0x20-0x7E`, `F0553`, `F0CA0-F0CA3` |
  | `SamsungSans_Regular_20` | battery percentage | (digits/ASCII) |

- Glyphs are emitted into C strings as **UTF-8 byte escapes**, e.g.
  `U+E62A` → `"\xEE\x98\xAA"`, `U+F0553` → `"\xF3\xB0\x95\x93"`,
  `U+F0CA0..3` → `"\xF3\xB0\xB2\xA0".."\xA3"`. Each `\x` escape is terminated by
  the next backslash, so adjacent escapes don't run together.
- **To add/extend a glyph:** add its codepoint to the relevant font's `-r`
  range, regenerate, and keep `0x20-0x7E` in the range (required for recolor
  markup — see below).

## Colors (`include/palette.h`)

- Snazzy palette, **except `SNAZZY_BLACK` is forced to `0x000000`** — Snazzy's
  charcoal `#282a36` reads as washed slate-blue on this backlit TFT (burn-in is
  not a concern: it's an LCD, not OLED). Used for screen + roller backgrounds only.
- `*_STR` macros are **lowercase hex strings for LVGL recolor markup**:
  `"#RRGGBB glyph#"`. The single space after the color is LVGL's delimiter and is
  **not** rendered. Recolor markup requires ASCII `0x20-0x7E` present in the font.
- **BLE state-color convention** (in `output_status.c`): color encodes
  pairing/connection state — **green = connected, blue = bonded-but-idle,
  dim = unpaired**; the **filled vs outline circle glyph encodes the selected
  profile**. Per-profile state via `zmk_ble_profile_is_connected(i)` /
  `!zmk_ble_profile_is_open(i)`.

## Config flags (`Kconfig.defconfig`)

- Widget gates used by `custom_status_screen.c`: `CONFIG_DONGLE_SCREEN_OUTPUT_ACTIVE`,
  `_LAYER_ACTIVE`, `_MODIFIER_ACTIVE`, `_BATTERY_ACTIVE` (all default `y`).
  `_WPM_ACTIVE` exists but the WPM widget isn't placed.
- `CONFIG_DONGLE_SCREEN_SYSTEM_ICON`: GUI glyph variant — `0` macOS (default),
  `1` Linux, `2` Windows (`U+E62A`).
- `CONFIG_DONGLE_SCREEN_LAYER_ROLLER_ALL_CAPS`, orientation (`_HORIZONTAL`,
  `_FLIPPED`), brightness/idle/ambient-light options — see the README table for
  the full list.

## LVGL constraints (`Kconfig.defconfig`)

- `LV_COLOR_DEPTH=16`, **`LV_COLOR_16_SWAP=y`** (byte-swapped 16-bit — relevant if
  you ever build raw color buffers).
- **Memory is tight:** `LV_Z_MEM_POOL_SIZE=10000` (~10 KB), `LV_Z_VDB_SIZE=100`,
  refresh period 20 ms. Be frugal with objects/styles.
- Enabled `LV_USE_*`: `LABEL, IMG, CANVAS, ANIMIMG, ANIMATION, LINE, ROLLER,
  DRAW_MASK, BAR, FLEX` (+ `LV_FONT_UNSCII_8`). **No PNG/GIF/BMP decoders** — do
  not rely on image decoding; use fonts/glyphs/canvas.

## Display hardware (`boards/shields/dongle_screen/boards/*.overlay`)

- `sitronix,st7789v` on **`spi3`**; chosen via `zephyr,display = &st7789`.
- Control/RESET/DC and SPI pins differ per board (xiao vs nice_nano) — see each
  overlay. Backlight is a PWM LED node `disp_bl`, driven by `src/brightness.c`.
- Optional ambient-light sensor: `avago,apds9960` (off by default).

## Conventions & workflow

- **Surgical commits: one task = one commit.** Conventional messages:
  `feat|fix|refactor(<scope>): <imperative>` (e.g. `feat(screen): ...`).
- Git: `origin` = `ethan605/zmk-dongle-screen` (fork), `upstream` =
  `janpfischer/zmk-dongle-screen`. Personal customizations target **origin/main**
  PRs, not upstream.
- Plans live in `.plans/` (gitignored) and `.opencode/plans/`.
- After implementing, request a review before claiming done; verify statically and
  flag that on-device flashing is the real confirmation.

## Gotchas

- `lv_obj_create()` containers inherit default theme **bg + padding** — call
  `lv_obj_remove_style_all(obj)` **before** sizing if you want a transparent
  grouping container (regression we already hit).
- Recolor markup (`"#RRGGBB x#"`) needs ASCII `0x20-0x7E` in the font, or it
  renders literally.
- A roller's `visible_row_count` is overridden by an explicit
  `lv_obj_set_size(...)` in the screen file — size wins.
- BLE: a **non-active** profile's connection change may not refresh the
  connections widget until a subscribed event fires (known limitation).
- LSP/include errors locally are expected (no toolchain) — ignore them.
