#ifndef DUTY_H
#define DUTY_H

#include "driver/ledc.h"
#include <stdbool.h>

// --- Structs ---
typedef struct {
    ledc_channel_t channel;
    gpio_num_t     gpio_num;
    uint32_t       duty_percent;
} pwm_channel_t;

typedef struct {
    ledc_timer_t     timer_num;
    uint32_t         frequency_hz;
    ledc_timer_bit_t resolution_bit;
} pwm_timer_config_t;

typedef struct {
    pwm_channel_t red;
    pwm_channel_t green;
    pwm_channel_t blue;
} rgb_pwm_t;

// --- Init Functions ---
void pwm_timer_init(const pwm_timer_config_t *timer_cfg);
void pwm_channel_init(const pwm_channel_t *ch_cfg, const pwm_timer_config_t *timer_cfg);
void rgb_pwm_init(const rgb_pwm_t *led, const pwm_timer_config_t *timer_cfg);

// --- Control Functions ---
void pwm_set_duty(const pwm_channel_t *ch_cfg, const pwm_timer_config_t *timer_cfg, uint8_t duty_percent);
void pwm_set_raw(const pwm_channel_t *ch_cfg, uint32_t raw_duty); // New function
void rgb_pwm_set_color(const rgb_pwm_t *led, const pwm_timer_config_t *timer_cfg,
                       uint8_t r, uint8_t g, uint8_t b, bool common_anode);

#endif