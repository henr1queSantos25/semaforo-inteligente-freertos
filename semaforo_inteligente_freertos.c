#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <string.h>

// Bibliotecas locais
#include "lib/ssd1306.h"
#include "lib/led_5x5.h"
#include "lib/button.h"
#include "lib/ledRGB.h"
#include "lib/buzzer.h"

// Definições de hardware
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define ENDERECO_OLED 0x3C
#define BUTTON_A 5
#define LED_GREEN 11
#define LED_RED 13

// Estado global do sistema
volatile bool modoNoturno = false;

// Enum para representar as cores do semáforo
typedef enum {
    COR_VERDE,
    COR_AMARELO,
    COR_VERMELHO,
    COR_NENHUMA
} CorSemaforo;

volatile CorSemaforo corAtual = COR_NENHUMA;

// ============================================================================
//                                TASKS
// ============================================================================

// TASK: Botão para alternar o modo noturno
void vButtonTask() {
    setup_button(BUTTON_A); // Configura o botão A

    while (true) {
        if (debounce_button(BUTTON_A)) {
            modoNoturno = !modoNoturno;
            printf(modoNoturno ? "Modo Noturno\n" : "Modo Normal\n");
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// TASK: Display OLED - mostra estado atual do semáforo ou modo noturno
void vDisplayTask() {
    ssd1306_t ssd;
    setup_I2C(I2C_PORT, I2C_SDA, I2C_SCL); // Configura o barramento I2C
    setup_ssd1306(&ssd, ENDERECO_OLED, I2C_PORT); // Inicializa o display OLED

    bool cor_display = false;

    while (true) {
        ssd1306_fill(&ssd, !cor_display);
        ssd1306_rect(&ssd, 1, 1, 126, 62, cor_display, !cor_display); // Desenha borda no display

        // Exibe mensagens no display com base no estado atual
        if (!modoNoturno) {
            switch (corAtual) {
                case COR_VERDE:
                    ssd1306_draw_string(&ssd, "SINAL ABERTO", 16, 28);
                    break;
                case COR_AMARELO:
                    ssd1306_draw_string(&ssd, "ATENCAO", 35, 28);
                    break;
                case COR_VERMELHO:
                    ssd1306_draw_string(&ssd, "SINAL FECHADO", 13, 28);
                    break;
                default:
                    break;
            }
        } else {
            ssd1306_draw_string(&ssd, "MODO NOTURNO", 16, 28);
        }

        ssd1306_send_data(&ssd); // Atualiza o dispaly
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// TASK: LEDs físicos verde/vermelho com lógica de semáforo + noturno
void vLedRGBTask() {
    setupLED(LED_GREEN); // Configura o LED verde
    setupLED(LED_RED);  // Configura o LED vermelho
    desligar_buzzer(); 

    while (true) {
        if (!modoNoturno) {
            // Ciclo do semáforo: Verde -> Amarelo -> Vermelho
            corAtual = COR_VERDE;
            for (int i = 0; i < 50 && !modoNoturno; i++) {
                gpio_put(LED_GREEN, 1);
                gpio_put(LED_RED, 0);
                vTaskDelay(pdMS_TO_TICKS(100));
            }

            corAtual = COR_AMARELO;
            for (int i = 0; i < 30 && !modoNoturno; i++) {
                gpio_put(LED_GREEN, 1);
                gpio_put(LED_RED, 1);
                vTaskDelay(pdMS_TO_TICKS(100));
            }

            corAtual = COR_VERMELHO;
            for (int i = 0; i < 60 && !modoNoturno; i++) {
                gpio_put(LED_GREEN, 0);
                gpio_put(LED_RED, 1);
                vTaskDelay(pdMS_TO_TICKS(100));
            }

        } else {
            // Modo noturno: pisca amarelo (ambos LEDs acesos)
            corAtual = COR_AMARELO;
            gpio_put(LED_GREEN, 1);
            gpio_put(LED_RED, 1);
            vTaskDelay(pdMS_TO_TICKS(500));

            corAtual = COR_NENHUMA;
            gpio_put(LED_GREEN, 0);
            gpio_put(LED_RED, 0);
            vTaskDelay(pdMS_TO_TICKS(1500));
        }
    }
}

// TASK: Matriz de LED 5x5 RGB mostra a cor atual do semáforo
void vMatrizLedTask() {
    setup_PIO();
    init_buzzer();

    while (true) {
        if (!modoNoturno) {
            // Atualiza a matriz de LEDs com base na cor atual
            if (corAtual == COR_VERDE) {
                for (int i = 0; i < 50 && !modoNoturno; i++) {
                    desenharCorNaMatriz(0.0, 0.09, 0.0);
                    vTaskDelay(pdMS_TO_TICKS(100));
                }
            }

            else if (corAtual == COR_AMARELO) {
                for (int i = 0; i < 30 && !modoNoturno; i++) {
                    desenharCorNaMatriz(0.09, 0.09, 0.0);
                    vTaskDelay(pdMS_TO_TICKS(100));
                }
            }

            else if (corAtual == COR_VERMELHO) {
                for (int i = 0; i < 25 && !modoNoturno; i++) {
                    desenharCorNaMatriz(0.09, 0.0, 0.0);
                    vTaskDelay(pdMS_TO_TICKS(240));
                }
            }

        } else {
            // Modo noturno: Pisca amarelo na matriz
            for (int i = 0; i < 10 && corAtual == COR_AMARELO; i++) {
                desenharCorNaMatriz(0.09, 0.09, 0.0);
                vTaskDelay(pdMS_TO_TICKS(50));
            }

            apagarMatriz();
            vTaskDelay(pdMS_TO_TICKS(1500));
        }
    }
}

// TASK: Buzzer emite som diferente para cada cor
void vBuzzerTask() {
    init_pwm_buzzer(); // Inicializa o PWM do buzzer
    CorSemaforo ultimaCor = COR_NENHUMA; // Armazena a última cor processada

    while (true) {
        if (!modoNoturno) {
            if (corAtual != ultimaCor) {
                ultimaCor = corAtual;
                parar_buzzer();
                vTaskDelay(pdMS_TO_TICKS(300));
            }

            switch (corAtual) {
                case COR_VERDE:
                    tocar_bipe_curto();
                    break;
                case COR_AMARELO:
                    tocar_beep_intermitente();
                    break;
                case COR_VERMELHO:
                    tocar_tom_continuo();
                    break;
                default:
                    parar_buzzer();
                    vTaskDelay(pdMS_TO_TICKS(100));
                    break;
            }

        } else {
            if (corAtual == COR_AMARELO) {
                tocar_beep_modo_noturno_pwm();
            } else {
                parar_buzzer();
            }
        }
    }
}

// ============================================================================
//                               FUNÇÃO PRINCIPAL
// ============================================================================

int main() {
    stdio_init_all();  // Inicializa a comunicação serial via USB

    // Criação das tasks com diferentes prioridades
    xTaskCreate(vButtonTask, "Button Task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, NULL);
    xTaskCreate(vDisplayTask, "Display Task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(vLedRGBTask, "LED RGB Task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, NULL);
    xTaskCreate(vMatrizLedTask, "LED 5x5 Task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, NULL);
    xTaskCreate(vBuzzerTask, "Buzzer Task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);

    // Inicia o escalonador do FreeRTOS
    vTaskStartScheduler();

    // Caso o scheduler falhe (por algum motivo)
    panic_unsupported();
}