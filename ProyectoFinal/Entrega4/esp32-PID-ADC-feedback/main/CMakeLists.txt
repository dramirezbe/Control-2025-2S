idf_component_register(SRCS "main.c" "utils/duty.c" "utils/PID.c" "utils/adc_utils.c"
                    PRIV_REQUIRES esp_driver_ledc esp_driver_uart esp_adc esp_driver_gptimer esp_driver_gpio
                    INCLUDE_DIRS "utils")
