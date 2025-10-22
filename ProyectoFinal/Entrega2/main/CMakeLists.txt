idf_component_register(SRCS "main.c" "utils/tim_ch_duty.c"
                       INCLUDE_DIRS "utils"   # si tus headers están en main/utils
                       REQUIRES driver)
