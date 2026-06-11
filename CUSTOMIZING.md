# Customizing this keymap

All your custom code lives in `Jmpn7/`. Oryx owns the key layout; code owns everything else. The cycle is always:

```
edit files → git add . → git commit -m "what you did" → git push
→ rerun "Fetch and build layout" Action → download artifact → flash with Keymapp
```

## The four files

**`rules.mk`** — switches features on/off at build time. One line each:
```make
AUTOCORRECT_ENABLE = yes      # already on
CAPS_WORD_ENABLE = yes        # already on (double-tap shift = caps until space)
REPEAT_KEY_ENABLE = yes       # repeats last key
KEY_OVERRIDE_ENABLE = yes     # remap shift+key combos
```
Full list: https://docs.qmk.fm/features

**`config.h`** — numeric settings and `#define` options:
```c
#define TAPPING_TERM 275          // ms to distinguish tap vs hold (already set)
#define AUTO_SHIFT_TIMEOUT 255    // hold time before auto-shift fires
```
If a value already exists upstream, `#undef NAME` first, then `#define NAME value`.

**`keymap.c`** — actual behavior code. Mostly you add to these hooks:
```c
void keyboard_post_init_user(void) { }            // runs once at boot
bool process_record_user(uint16_t keycode, keyrecord_t *record) { }  // every keypress
bool rgb_matrix_indicators_user(void) { }          // lighting overrides
```
Careful: Oryx regenerates parts of this file, so future merges can conflict.
Keep your edits small and separate from the generated blocks.

**`rgb_matrix_user.inc`** — custom lighting animations (the pool waves live here).
Tweakables: `pool_palette[]` colors, `POOL_RIPPLE_SPEED`, `POOL_RIPPLE_WIDTH`.

## Example: make a key type your email

In `keymap.c`, find `process_record_user` (or add it if missing) and handle a keycode:
```c
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  if (keycode == ST_MACRO_0 && record->event.pressed) {
    SEND_STRING("jlb7487@gmail.com");
    return false;
  }
  return true;
}
```
(Assign the key itself in Oryx as a blank macro, then override it here.)

## Example: add your own autocorrect words

The default dictionary is built in. To use your own, create `autocorrect_dict.txt`:
```
teh -> the
widht -> width
```
then generate the header with QMK's CLI (`qmk generate-autocorrect-data autocorrect_dict.txt -o autocorrect_data.h`)
and put `autocorrect_data.h` in `Jmpn7/`. (Or ask Claude to generate it.)

## Rules of thumb

- Layout/keys/layers → edit in Oryx, then rerun the Action to pull changes in.
- Behavior/timing/lighting/features → edit code here on `main`.
- Never edit the `oryx` branch; the Action owns it.
- If the Action fails on a compile error, the log shows the file and line.
- Keyboard remembers lighting settings in EEPROM across flashes — code that
  should always apply at boot belongs in `keyboard_post_init_user` with
  `_noeeprom` functions.
