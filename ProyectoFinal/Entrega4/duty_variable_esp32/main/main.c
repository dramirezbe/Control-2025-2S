// main.c
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_err.h"

// Configuración PWM
#define PWM_GPIO        GPIO_NUM_25
#define PWM_FREQ_HZ     20000           // 20 kHz
#define PWM_RES_BITS    10              // 10 bits -> rango 0..1023
#define PWM_TIMER       LEDC_TIMER_0
#define PWM_MODE        LEDC_HIGH_SPEED_MODE
#define PWM_CHANNEL     LEDC_CHANNEL_0

// Tiempo entre cambios
#define STEP_DELAY_MS   100

static inline uint32_t percent_to_duty(uint8_t percent)
{
    // Clamp por si acaso
    if (percent > 100) percent = 100;
    uint32_t max_duty = (1u << PWM_RES_BITS) - 1u;
    return (max_duty * percent) / 100u;
}

void pwm_init(void)
{
    // Configura timer
    ledc_timer_config_t timer_cfg = {
        .speed_mode = PWM_MODE,
        .duty_resolution = PWM_RES_BITS,
        .timer_num = PWM_TIMER,
        .freq_hz = PWM_FREQ_HZ,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK( ledc_timer_config(&timer_cfg) );

    // Configura canal
    ledc_channel_config_t ch_cfg = {
        .gpio_num = PWM_GPIO,
        .speed_mode = PWM_MODE,
        .channel = PWM_CHANNEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = PWM_TIMER,
        .duty = 0, // arranca en 0, luego lo ponemos
        .hpoint = 0
    };
    ESP_ERROR_CHECK( ledc_channel_config(&ch_cfg) );
}

void pwm_set_percent(uint8_t percent)
{
    uint32_t duty = percent_to_duty(percent);
    ESP_ERROR_CHECK( ledc_set_duty(PWM_MODE, PWM_CHANNEL, duty) );
    ESP_ERROR_CHECK( ledc_update_duty(PWM_MODE, PWM_CHANNEL) );
}

void pwm_task(void *arg)
{
    (void)arg;
    const uint8_t seq[] = {50, 60, 40}; // secuencia de porcentajes
    const size_t n = sizeof(seq) / sizeof(seq[0]);
    size_t i = 0;

    // opcional: partir en 50% al inicio
    pwm_set_percent(seq[0]);
    vTaskDelay(pdMS_TO_TICKS(STEP_DELAY_MS));

    while (1) {
        pwm_set_percent(seq[i]);
        i = (i + 1) % n;
        vTaskDelay(pdMS_TO_TICKS(STEP_DELAY_MS));
    }
}

void app_main(void)
{
    pwm_init();

    // Crea la tarea que hace el cambio cada 100 ms
    xTaskCreatePinnedToCore(pwm_task, "pwm_task", 2048, NULL, 5, NULL, APP_CPU_NUM);
}
