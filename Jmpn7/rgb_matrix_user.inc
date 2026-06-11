// Custom "pool ripple" effect — caustic water look:
//  - Idle: every key independently shimmers through the pool palette at its
//    own phase (no visible loop), with bright foam streaks at wave crests.
//  - Typing: ripples travel outward and inject energy into keys they pass,
//    making those keys cycle/pulse faster — like waves disturbing the water.
RGB_MATRIX_EFFECT(POOL_RIPPLE)

#ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

// ---- Pool palette ----
static const RGB pool_palette[] = {
    {0x0D, 0x78, 0xAA}, // #0D78AA deep pool blue
    {0x02, 0xBA, 0xD6}, // #02BAD6 bright cyan
    {0xA4, 0xF4, 0xF8}, // #A4F4F8 light aqua
    {0xC9, 0xFA, 0xFE}, // #C9FAFE pale ice
    {0x02, 0xBA, 0xD6}, // #02BAD6 back down for smooth looping
};
#define POOL_PALETTE_SIZE (sizeof(pool_palette) / sizeof(pool_palette[0]))
static const RGB pool_foam = {0xE3, 0xFA, 0xFF}; // #E3FAFF foam highlights

#define POOL_RIPPLE_SPEED 6    // lower = ripples travel slower
#define POOL_RIPPLE_WIDTH 50   // how wide the ripple's influence is
#define POOL_PULSE_GAIN 1      // how hard ripples speed up the shimmer (1-4)

// Smoothly sample the palette at position t (0-255, wraps)
static RGB pool_sample(uint8_t t) {
    uint8_t seg_len = 256 / POOL_PALETTE_SIZE;
    uint8_t seg     = t / seg_len;
    if (seg >= POOL_PALETTE_SIZE) seg = POOL_PALETTE_SIZE - 1;
    uint8_t    frac = (t - seg * seg_len) * POOL_PALETTE_SIZE;
    const RGB* a    = &pool_palette[seg];
    const RGB* b    = &pool_palette[(seg + 1) % POOL_PALETTE_SIZE];
    return (RGB){lerp8by8(a->r, b->r, frac), lerp8by8(a->g, b->g, frac), lerp8by8(a->b, b->b, frac)};
}

// Each key carries its own water phase, advanced every frame.
static uint16_t pool_phase[RGB_MATRIX_LED_COUNT];

static bool POOL_RIPPLE(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    if (params->init) {
        // Desync all keys so the idle shimmer never looks uniform
        for (uint8_t i = 0; i < RGB_MATRIX_LED_COUNT; i++) {
            pool_phase[i] = i * 157;
        }
    }

    uint8_t count      = g_last_hit_tracker.count;
    uint8_t base_speed = qadd8(rgb_matrix_config.speed / 16, 3); // idle drift rate

    for (uint8_t i = led_min; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();
        uint8_t x = g_led_config.point[i].x;
        uint8_t y = g_led_config.point[i].y;

        // ---- Ripple energy: how strongly passing waves disturb this key ----
        uint16_t energy = 0;
        for (uint8_t j = 0; j < count; j++) {
            int16_t  dx   = x - g_last_hit_tracker.x[j];
            int16_t  dy   = y - g_last_hit_tracker.y[j];
            uint16_t dist = sqrt16(dx * dx + dy * dy);
            uint16_t tick = scale16by8(g_last_hit_tracker.tick[j], POOL_RIPPLE_SPEED);

            int16_t ring = (int16_t)tick - (int16_t)dist;
            if (ring < 0 || ring > POOL_RIPPLE_WIDTH) continue;
            uint8_t fade = 255 - scale16by8(g_last_hit_tracker.tick[j], 2);
            energy += scale8((POOL_RIPPLE_WIDTH - ring) * 255 / POOL_RIPPLE_WIDTH, fade);
        }
        if (energy > 255) energy = 255;

        // ---- Advance this key's phase: ripples = faster pulsing ----
        pool_phase[i] += base_speed + (energy * POOL_PULSE_GAIN);

        // ---- Two desynced sines per key = organic caustic shimmer ----
        uint8_t offset = i * 37;
        uint8_t s1 = sin8((pool_phase[i] >> 4) + offset);
        uint8_t s2 = sin8((pool_phase[i] >> 5) + offset * 3 + x + y);
        uint8_t t  = (s1 >> 1) + (s2 >> 1);

        RGB c = pool_sample(t);

        // ---- Foam streaks at wave crests (the bright lines in the photo) ----
        uint16_t crest = (uint16_t)qsub8(s1, 200) * 4;
        if (crest > 255) crest = 255;
        // Ripple energy also pushes color toward foam, so pulses flash brighter
        uint8_t foam_mix = (uint8_t)crest | scale8((uint8_t)energy, 140);
        uint8_t r = lerp8by8(c.r, pool_foam.r, foam_mix);
        uint8_t g = lerp8by8(c.g, pool_foam.g, foam_mix);
        uint8_t b = lerp8by8(c.b, pool_foam.b, foam_mix);

        uint8_t max_v = rgb_matrix_config.hsv.v;
        rgb_matrix_set_color(i, scale8(r, max_v), scale8(g, max_v), scale8(b, max_v));
    }
    return rgb_matrix_check_finished_leds(led_max);
}

#endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
