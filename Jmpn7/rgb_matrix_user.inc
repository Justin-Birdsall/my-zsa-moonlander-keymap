// Custom "pool ripple" effect: keypresses send colored ripples across the board.
// Edit POOL_PALETTE below with your hex codes: {0xRR, 0xGG, 0xBB}
RGB_MATRIX_EFFECT(POOL_RIPPLE)

#ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

// ---- Pool palette ----
static const RGB pool_palette[] = {
    {0x0D, 0x78, 0xAA}, // #0D78AA deep pool blue
    {0xE3, 0xFA, 0xFF}, // #E3FAFF white foam
    {0xC9, 0xFA, 0xFE}, // #C9FAFE pale ice
    {0xA4, 0xF4, 0xF8}, // #A4F4F8 light aqua
    {0x02, 0xBA, 0xD6}, // #02BAD6 bright cyan
};
#define POOL_PALETTE_SIZE (sizeof(pool_palette) / sizeof(pool_palette[0]))

#define POOL_RIPPLE_SPEED 6   // lower = slower ripples
#define POOL_RIPPLE_WIDTH 40  // ring thickness

static bool POOL_RIPPLE(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    uint8_t count = g_last_hit_tracker.count;
    for (uint8_t i = led_min; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();
        uint16_t r = 0, g = 0, b = 0;

        for (uint8_t j = 0; j < count; j++) {
            int16_t dx   = g_led_config.point[i].x - g_last_hit_tracker.x[j];
            int16_t dy   = g_led_config.point[i].y - g_last_hit_tracker.y[j];
            uint16_t dist = sqrt16(dx * dx + dy * dy);
            uint16_t tick = scale16by8(g_last_hit_tracker.tick[j], POOL_RIPPLE_SPEED);

            // ring: bright where |dist - tick| is small, fades as ripple ages
            int16_t ring = tick - dist;
            if (ring < 0 || ring > POOL_RIPPLE_WIDTH) continue;
            uint8_t fade = 255 - scale16by8(g_last_hit_tracker.tick[j], 2);
            uint8_t v    = scale8((POOL_RIPPLE_WIDTH - ring) * 255 / POOL_RIPPLE_WIDTH, fade);

            const RGB* c = &pool_palette[g_last_hit_tracker.index[j] % POOL_PALETTE_SIZE];
            r = MIN(255, r + scale8(c->r, v));
            g = MIN(255, g + scale8(c->g, v));
            b = MIN(255, b + scale8(c->b, v));
        }

        uint8_t max_v = rgb_matrix_config.hsv.v;
        rgb_matrix_set_color(i, scale8(r, max_v), scale8(g, max_v), scale8(b, max_v));
    }
    return rgb_matrix_check_finished_leds(led_max);
}

#endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
