#include "pico/stdlib.h"

#define BUZZER1 10




void init_buzzer();
void ligar_buzzer();
void desligar_buzzer();
void buzzer_on(uint buzzer_pin, float frequency, uint duration_ms);
void init_pwm_buzzer();
void buzzer_pwm_on(float freq_hz, float duty);
void buzzer_pwm_off();
void tocar_bipe_curto();
void tocar_beep_intermitente();
void tocar_tom_continuo();
void tocar_beep_modo_noturno_pwm();
void parar_buzzer();

