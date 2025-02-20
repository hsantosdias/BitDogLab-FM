/*
* Simulação de Áudio para Transmissor FM usando RP2040
* - Gera sinais de áudio simulados para testar a modulação FM
* - Tipos de sinal: Senoidal, Ruído Aleatório, Onda Quadrada
* - Ajuste de Frequência via Botões: UP (GPIO 5) e DOWN (GPIO 6)
*/

#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/timer.h"

// Definindo os pinos
#define FM_OUTPUT_PIN 15   // Saída FM com PWM
#define BTN_UP 5           // Botão para aumentar frequência
#define BTN_DOWN 6         // Botão para diminuir frequência

// Configuração da Frequência FM
float fm_freq = 90.0;      // Frequência inicial em MHz
const float freq_min = 88.0;
const float freq_max = 108.0;
const float freq_step = 0.1; // Incremento de 0.1 MHz

// Parâmetros da simulação de áudio
const uint amostras_por_segundo = 8000; // Frequência de amostragem
uint32_t contador = 0;

// Variável para controle do divisor de clock
float clkdiv_base = 4.0f;

// Seleção da forma de onda: 1 = Senoidal, 2 = Ruído, 3 = Quadrada
const uint8_t tipo_onda = 1;

// Função para simular o sinal de áudio
uint16_t simular_audio() {
    uint16_t valor_audio = 0;

    if (tipo_onda == 1) {
        // 1. Onda Senoidal (Tom Contínuo)
        float frequencia_seno = 440.0; // Frequência do tom (440 Hz = Nota Lá)
        valor_audio = (uint16_t)(2048 + 2047 * sin(2 * M_PI * frequencia_seno * contador / amostras_por_segundo));
    }
    else if (tipo_onda == 2) {
        // 2. Ruído Aleatório (Simulando ruído ambiente)
        valor_audio = rand() % 4096;
    }
    else if (tipo_onda == 3) {
        // 3. Onda Quadrada (Som Digital ou Bip)
        float frequencia_quadrada = 1000.0; // Frequência da onda quadrada
        valor_audio = (contador % (amostras_por_segundo / frequencia_quadrada) < (amostras_por_segundo / frequencia_quadrada) / 2) ? 4095 : 0;
    }

    contador++;
    return valor_audio;
}

// Configuração do PWM para FM
void setup_pwm() {
    gpio_set_function(FM_OUTPUT_PIN, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(FM_OUTPUT_PIN);

    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, clkdiv_base);
    pwm_config_set_wrap(&config, 255);  // Resolução de 8 bits
    pwm_init(slice, &config, true);
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

// Função de modulação da frequência conforme o áudio simulado
void modulate_frequency(uint slice_num, int audio_sample) {
    // Ajuste fino do wrap conforme o áudio
    uint16_t wrap_value = 250 + (audio_sample / 4);
    pwm_set_wrap(slice_num, wrap_value);
}

// Loop principal da transmissão FM
void loop_fm() {
    uint slice_num = pwm_gpio_to_slice_num(FM_OUTPUT_PIN);

    // Define o intervalo entre amostras (em microsegundos)
    uint64_t intervalo_us = 1000000 / amostras_por_segundo;

    while (1) {
        // Simula o valor do áudio (0 a 4095)
        uint16_t audio_simulado = simular_audio();

        // Normaliza para 8 bits (0 a 255)
        int audio_sample = audio_simulado >> 4;

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
    setup_botoes();
    loop_fm();
}
