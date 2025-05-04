#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "hardware/pwm.h"
#include "buzzer.h"

#define BUZZER_PWM_SLICE pwm_gpio_to_slice_num(BUZZER1)



void init_buzzer() {
    gpio_init(BUZZER1);
    gpio_set_dir(BUZZER1, GPIO_OUT);
}

void ligar_buzzer() {
    gpio_put(BUZZER1, 1);
}

// Desliga o buzzer
void desligar_buzzer() {
    gpio_put(BUZZER1, 0);
}

// Função para gerar som no buzzer por um tempo específico e com a frequência desejada
void buzzer_on(uint buzzer_pin, float frequency, uint duration_ms) {
    float period = 1.0f / frequency;
    float half_period_us = (period / 2) * 1e6; // Meio período em microssegundos
    uint cycles = (duration_ms * frequency) / 1000;

    for (uint i = 0; i < cycles; i++) {
        gpio_put(buzzer_pin, 1);
        sleep_us((uint)half_period_us);
        gpio_put(buzzer_pin, 0);
        sleep_us((uint)half_period_us);
    }
}

// Função para inicializar o PWM do buzzer
void init_pwm_buzzer() {
    gpio_set_function(BUZZER1, GPIO_FUNC_PWM); // Define o pino como saída PWM
    uint slice = BUZZER_PWM_SLICE;
    pwm_set_clkdiv(slice, 4.0f); // Divide o clock base (ajuste fino se necessário)
    pwm_set_enabled(slice, true);
}

// Função para ativar o PWM do buzzer com frequência e ciclo de trabalho específicos
void buzzer_pwm_on(float freq_hz, float duty_cycle) {
    uint slice = BUZZER_PWM_SLICE;
    uint32_t clock_hz = 125000000; // Clock padrão do RP2040
    uint32_t wrap = clock_hz / (4 * freq_hz); // PWM_CLK_DIV = 4.0
    if (wrap < 10) wrap = 10;
    if (wrap > 65535) wrap = 65535;

    pwm_set_wrap(slice, wrap);
    pwm_set_chan_level(slice, pwm_gpio_to_channel(BUZZER1), wrap * duty_cycle);
    pwm_set_enabled(slice, true);
}

// Função para desativar o PWM do buzzer
void buzzer_pwm_off() {
    uint slice = BUZZER_PWM_SLICE;
    pwm_set_chan_level(slice, pwm_gpio_to_channel(BUZZER1), 0);
    pwm_set_enabled(slice, false);
}


// SONS
void tocar_bipe_curto() {
    buzzer_pwm_on(2000.0f, 0.5f);
    vTaskDelay(pdMS_TO_TICKS(150));
    buzzer_pwm_off();
    vTaskDelay(pdMS_TO_TICKS(850)); // pausa até completar 1s
}

void tocar_beep_intermitente() {
    const uint total_duration = 900;
    const uint beep = 50, pausa = 50;
    uint tempo = 0;
    while (tempo + beep + pausa <= total_duration) {
        buzzer_pwm_on(2000.0f, 0.5f);
        vTaskDelay(pdMS_TO_TICKS(beep));
        buzzer_pwm_off();
        vTaskDelay(pdMS_TO_TICKS(pausa));
        tempo += beep + pausa;
    }
}

void tocar_tom_continuo() {
    buzzer_pwm_on(1000.0f, 0.5f);
    vTaskDelay(pdMS_TO_TICKS(500));
    buzzer_pwm_off();
    vTaskDelay(pdMS_TO_TICKS(1500));
}

void tocar_beep_modo_noturno_pwm() {
    buzzer_pwm_on(800.0f, 0.4f);        
    vTaskDelay(pdMS_TO_TICKS(250)); 

    buzzer_pwm_off();           
    vTaskDelay(pdMS_TO_TICKS(1750));
}

void parar_buzzer() {
    buzzer_pwm_off();
}

