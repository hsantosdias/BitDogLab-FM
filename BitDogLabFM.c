/*
* Transmissor FM usando RP2040 com Ajuste de Frequência e Modulação PWM
* - Entrada de Áudio: Microfone no GPIO 28 (ADC2)
* - Saída FM: GPIO 15 (PWM)
* - Ajuste de Frequência: Botão UP (GPIO 5) e DOWN (GPIO 6)
* - Faixa FM: 88.0 a 108.0 MHz
*/

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/timer.h"

// Definindo os pinos
#define AUDIO_INPUT_PIN 28  // Microfone (ADC2)
#define FM_OUTPUT_PIN 19   // Saída FM com PWM
#define BTN_UP 5           // Botão para aumentar frequência
#define BTN_DOWN 6         // Botão para diminuir frequência

// Configuração da Frequência FM
float fm_freq = 90.0;      // Frequência inicial em MHz
const float freq_min = 88.0;
const float freq_max = 108.0;
const float freq_step = 0.1; // Incremento de 0.1 MHz

// Parâmetros do ADC
const uint8_t avg_samples = 10;   // Média móvel com 10 amostras
const uint amostras_por_segundo = 8000; // Frequência de amostragem

// Variável para controle do divisor de clock
float clkdiv_base = 4.0f;

// Configuração do PWM para FM
void setup_pwm() {
    gpio_set_function(FM_OUTPUT_PIN, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(FM_OUTPUT_PIN);

    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, clkdiv_base);
    pwm_config_set_wrap(&config, 255);  // Resolução de 8 bits
    pwm_init(slice, &config, true);
}

// Configuração do ADC para o microfone
void setup_adc() {
    adc_init();
    adc_gpio_init(AUDIO_INPUT_PIN);  // Configura GPIO28 como entrada ADC
    adc_select_input(2);             // Seleciona o canal 2 (GPIO28)
}

// Configuração dos Botões com Pull-up Interno
void setup_botoes() {
    gpio_init(BTN_UP);
    gpio_set_dir(BTN_UP, GPIO_IN);
    gpio_pull_up(BTN_UP);   // Ativa o pull-up interno

    gpio_init(BTN_DOWN);
    gpio_set_dir(BTN_DOWN, GPIO_IN);
    gpio_pull_up(BTN_DOWN); // Ativa o pull-up interno
}

// Função para ajustar a frequência base com os botões
void ajustar_frequencia() {
    static uint32_t last_time = 0;
    if (to_ms_since_boot(get_absolute_time()) - last_time < 200) return; // Debounce de 200ms

    if (!gpio_get(BTN_UP)) { // Botão UP pressionado
        fm_freq += freq_step;
        last_time = to_ms_since_boot(get_absolute_time());
    }
    if (!gpio_get(BTN_DOWN)) { // Botão DOWN pressionado
        fm_freq -= freq_step;
        last_time = to_ms_since_boot(get_absolute_time());
    }

    // Limita a frequência dentro da faixa FM permitida
    if (fm_freq < freq_min) fm_freq = freq_min;
    if (fm_freq > freq_max) fm_freq = freq_max;

    // Ajusta o divisor de clock conforme a frequência desejada
    clkdiv_base = 1.0f + ((108.0 - fm_freq) / 20.0f);
    pwm_set_clkdiv(pwm_gpio_to_slice_num(FM_OUTPUT_PIN), clkdiv_base);
}

// Função de modulação da frequência conforme o áudio
void modulate_frequency(uint slice_num, int audio_sample) {
    // Ajuste fino do wrap conforme o áudio
    uint16_t wrap_value = 250 + (audio_sample / 4);
    pwm_set_wrap(slice_num, wrap_value);
}

// Loop principal da transmissão FM
void loop_fm() {
    uint slice_num = pwm_gpio_to_slice_num(FM_OUTPUT_PIN);
    uint16_t audio_avg = 0;

    // Define o intervalo entre amostras (em microsegundos)
    uint64_t intervalo_us = 1000000 / amostras_por_segundo;

    while (1) {
        // Leitura do ADC com média móvel
        audio_avg = 0;
        for (int i = 0; i < avg_samples; i++) {
            audio_avg += adc_read();
            sleep_us(125); // Ajuste fino para suavizar o áudio
        }
        audio_avg /= avg_samples;

        // Normaliza para 8 bits (0 a 255)
        int audio_sample = audio_avg >> 4;

        // Modula a frequência conforme o valor do áudio
        modulate_frequency(slice_num, audio_sample);

        // Ajusta a frequência base conforme os botões
        ajustar_frequencia();

        sleep_us(intervalo_us); // Controla a taxa de amostragem
    }
}

int main() {
    stdio_init_all();
    setup_pwm();
    setup_adc();
    setup_botoes();
    loop_fm();
}
