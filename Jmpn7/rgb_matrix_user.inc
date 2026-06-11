// Custom "pool ripple" effect:
//  - Idle: slow drifting water waves flowing through the pool palette
//  - Typing: keypresses splash foam ripples on top of the waves
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
static const RGB pool_foam = {0xE3, 0xFA, 0xFF}; // #E3FAFF foam for ripple crests

#define POOL_RIPPLE_SPEED 6   // lower = slower ripples
#define POOL_RIPPLE_WIDTH 40  // ring thickness

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

static bool POOL_RIPPLE(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    uint16_t time  = scale16by8(g_rgb_timer, qadd8(rgb_matrix_config.speed / 8, 1));
    uint8_t  count = g_last_hit_tracker.count;

    for (uint8_t i = led_min; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();
        uint8_t x = g_led_config.point[i].x;
        uint8_t y = g_led_config.point[i].y;

        // ---- Idle water: two overlapping waves drifting across the board ----
        uint8_t phase = x + sin8((uint8_t)(time / 2) + y * 2) / 6 + (uint8_t)(time / 3);
        RGB     c     = pool_sample(phase);
        uint8_t shimmer = 192 + sin8(x * 3 + (uint8_t)time) / 4; // subtle sparkle 192..255
        uint16_t r = scale8(c.r, shimmer);
        uint16_t g = scale8(c.g, shimmer);
        uint16_t b = scale8(c.b, shimmer);

        // ---- Typing: foam ripples expanding from each keypress ----
        for (uint8_t j = 0; j < count; j++) {
            int16_t  dx   = x - g_last_hit_tracker.x[j];
            int16_t  dy   = y - g_last_hit_tracker.y[j];
            uint16_t dist = sqrt16(dx * dx + dy * dy);
            uint16_t tick = scale16by8(g_last_hit_tracker.tick[j], POOL_RIPPLE_SPEED);

            int16_t ring = tick - dist;
            if (ring < 0 || ring > POOL_RIPPLE_WIDTH) continue;
            uint8_t fade = 255 - scale16by8(g_last_hit_tracker.tick[j], 2);
            uint8_t v    = scale8((POOL_RIPPLE_WIDTH - ring) * 255 / POOL_RIPPLE_WIDTH, fade);

            r = MIN(255, r + scale8(pool_foam.r, v));
            g = MIN(255, g + scale8(pool_foam.g, v));
            b = MIN(255, b + scale8(pool_foam.b, v));
        }

        uint8_t max_v = rgb_matrix_config.hsv.v;
        rgb_matrix_set_color(i, scale8(r, max_v), scale8(g, max_v), scale8(b, max_v));
    }
    return rgb_matrix_check_finished_leds(led_max);
}

#endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
