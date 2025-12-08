#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gptimer.h"
#include "driver/uart.h"
#include "esp_log.h"

#include "utils/duty.h"
#include "utils/PID.h"
#include "utils/adc_utils.h"

// --- System Definition ---
#define SYSTEM_MAX_VOLTS    12.0f    // Real Power Supply Voltage
#define ADC_MAX_INPUT_V     2.45f    // Max voltage allowed at ESP Pin
#define REF_VAL             6.0f     // Target: 6V Real

// Scaling Factor: (12.0 / 2.45) = 4.8979...
#define VOLTAGE_SCALER      (SYSTEM_MAX_VOLTS / ADC_MAX_INPUT_V)

// --- PID Defaults ---
#define PID_KP_DEF  2.955f
#define PID_KI_DEF  450.0f
#define PID_KD_DEF  0.0021345f
#define PID_TF_DEF  0.0001f    
#define PID_TS_DEF  0.001f  
#define PWM_MAX_BITS 1023.0f

// --- UART ---
#define UART_NUM        UART_NUM_0
#define BUF_SIZE        1024
#define RD_BUF_SIZE     BUF_SIZE

// --- Globals ---
pid_t my_pid;
adc_channel_handle_t my_adc_handle = NULL;
TaskHandle_t pid_task_handle = NULL;
static QueueHandle_t uart_queue;

static const pwm_timer_config_t pwm_timer = {
    .timer_num = LEDC_TIMER_0,
    .frequency_hz = 50000,
    .resolution_bit = LEDC_TIMER_10_BIT
};

static const pwm_channel_t pwm_ch = {
    .channel = LEDC_CHANNEL_0,
    .gpio_num = GPIO_NUM_25,
    .duty_percent = 0
};

// --- ISR ---
static bool IRAM_ATTR on_timer_alarm(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data) {
    BaseType_t high_task_awoken = pdFALSE;
    vTaskNotifyGiveFromISR(pid_task_handle, &high_task_awoken);
    return (high_task_awoken == pdTRUE);
}

// --- Control Task ---
void pid_control_task(void *pvParameter) {
    int raw_adc = 0;
    int adc_mv = 0;
    float real_volts = 0.0f;
    float pid_out_volts = 0.0f;
    uint32_t duty_cycle = 0;

    while (1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        // 1. Read Sensor (mV)
        get_raw_data(my_adc_handle, &raw_adc);
        raw_to_voltage(my_adc_handle, raw_adc, &adc_mv);

        // 2. Scale: ADC Volts -> Real System Volts
        // Example: If ADC reads 1.225V -> (1.225 * 4.89) = 6.0V
        real_volts = ((float)adc_mv / 1000.0f) * VOLTAGE_SCALER;

        // 3. Compute PID (Input: 0-12V, Output: 0-12V)
        pid_out_volts = pid_compute(&my_pid, real_volts);

        // 4. Actuator Map: 12V = 100% Duty
        // Duty = (Output / 12.0) * 1023
        if (pid_out_volts > 0.0f) {
            duty_cycle = (uint32_t)((pid_out_volts / SYSTEM_MAX_VOLTS) * PWM_MAX_BITS);
        } else {
            duty_cycle = 0;
        }

        if (duty_cycle > 1023) duty_cycle = 1023;
        pwm_set_raw(&pwm_ch, duty_cycle);
    }
}

// --- UART Task ---
void uart_rx_task(void *arg) {
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    uart_param_config(UART_NUM, &uart_config);
    uart_set_pin(UART_NUM, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_NUM, RD_BUF_SIZE * 2, BUF_SIZE * 2, 20, &uart_queue, 0);

    uart_event_t event;
    uint8_t *dtmp = (uint8_t *) malloc(RD_BUF_SIZE);

    while(1) {
        if(xQueueReceive(uart_queue, (void * )&event, (TickType_t)portMAX_DELAY)) {
            if (event.type == UART_DATA) {
                uart_read_bytes(UART_NUM, dtmp, event.size, portMAX_DELAY);
                dtmp[event.size] = '\0';
                
                char *cmd = (char *)dtmp;
                float val = 0.0f;

                if (sscanf(cmd, "P%f", &val) == 1) {
                    my_pid.kp = val;
                    ESP_LOGI("CMD", "New Kp: %f", my_pid.kp);
                }
                else if (sscanf(cmd, "I%f", &val) == 1) {
                    my_pid.ki = val;
                    ESP_LOGI("CMD", "New Ki: %f", my_pid.ki);
                }
                else if (sscanf(cmd, "D%f", &val) == 1) {
                    my_pid.kd = val;
                    my_pid.beta = my_pid.kd / (PID_TF_DEF + PID_TS_DEF);
                    ESP_LOGI("CMD", "New Kd: %f", my_pid.kd);
                }
            }
        }
    }
    free(dtmp);
    vTaskDelete(NULL);
}

void app_main(void)
{
    pwm_timer_init(&pwm_timer);
    pwm_channel_init(&pwm_ch, &pwm_timer);

    adc_config_t adc_cfg = {
        .unit_id = ADC_UNIT_1,
        .channel = ADC_CHANNEL_6,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12
    };
    set_adc(&adc_cfg, &my_adc_handle);

    // Init PID: Limits are now 0V to 12V
    pid_init(&my_pid, PID_KP_DEF, PID_KI_DEF, PID_KD_DEF, PID_TF_DEF, PID_TS_DEF, 0.0f, SYSTEM_MAX_VOLTS);
    my_pid.reference = REF_VAL; // 6.0V

    xTaskCreatePinnedToCore(pid_control_task, "PID", 4096, NULL, configMAX_PRIORITIES - 1, &pid_task_handle, 1);
    xTaskCreate(uart_rx_task, "uart_rx", 4096, NULL, 5, NULL);

    gptimer_handle_t gptimer = NULL;
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1000000, 
    };
    gptimer_new_timer(&timer_config, &gptimer);

    gptimer_alarm_config_t alarm_config = {
        .alarm_count = 1000, // 1000us = 1ms
        .flags.auto_reload_on_alarm = true,
    };
    gptimer_set_alarm_action(gptimer, &alarm_config);
    
    gptimer_event_callbacks_t cbs = { .on_alarm = on_timer_alarm };
    gptimer_register_event_callbacks(gptimer, &cbs, NULL);
    
    gptimer_enable(gptimer);
    gptimer_start(gptimer);
}