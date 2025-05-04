#include "ledRGB.h"
#include "hardware/pwm.h"

void setupLED(uint led) {
    gpio_init(led);              // Inicializa o pino do LED
    gpio_set_dir(led, GPIO_OUT); // Define o pino como saída
    gpio_put(led, 0);            // Desliga o LED inicialmente
}

void setup_pwm_led(uint led) {
    gpio_set_function(led, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(led);
    pwm_set_wrap(slice_num, 255);
    pwm_set_enabled(slice_num, true);
}