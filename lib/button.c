#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "button.h"

void setup_button(uint gpio)
{
    gpio_init(gpio);
    gpio_set_dir(gpio, GPIO_IN);
    gpio_pull_up(gpio);
}

bool debounce_button(uint gpio) {
    if (!gpio_get(gpio)) {
        vTaskDelay(pdMS_TO_TICKS(50)); // Aguarda para estabilizar
        if (!gpio_get(gpio)) {
            // Aguarda o botão ser solto de forma não bloqueante
            while (!gpio_get(gpio)) {
                vTaskDelay(pdMS_TO_TICKS(10)); // Libera CPU para outras tasks
            }
            return true;
        }
    }
    return false;
}
