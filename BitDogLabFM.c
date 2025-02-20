/*
* Transmissor FM usando RP2040 com Ajuste de Frequência e Modulação PWM
* - Entrada de Áudio: Microfone no GPIO 28 (ADC2)
* - Saída FM: GPIO 15 (PWM)
* - Ajuste de Frequência: Botão UP (GPIO 5) e DOWN (GPIO 6)
* - Faixa FM: 88.0 a 108.0 MHz
* Melhorias implementadas para estabilidade e performance
*/

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/timer.h"
#include "pico/sync.h"

// Definindo os pinos
#define AUDIO_INPUT_PIN 28    // Microfone (ADC2)
#define FM_OUTPUT_PIN 19      // Saída FM com PWM
#define BTN_UP 5             // Botão para aumentar frequência
#define BTN_DOWN 6           // Botão para diminuir frequência

// Configuração da Frequência FM
volatile float fm_freq = 90.0;  // Frequência inicial em MHz
const float freq_min = 88.0;
const float freq_max = 108.0;
const float freq_step = 0.1;    // Incremento de 0.1 MHz

// Parâmetros do ADC e PWM
const uint8_t avg_samples = 16;   // Aumentado para melhor suavização
const uint amostras_por_segundo = 44100; // Aumentado para melhor qualidade
const uint16_t pwm_wrap_base = 255;  // Valor base para o wrap do PWM

// Buffer circular para média móvel
uint16_t audio_buffer[16];
uint8_t buffer_index = 0;

// Mutex para proteção do acesso à frequência
mutex_t freq_mutex;

// Configuração do PWM para FM
void setup_pwm() {
    gpio_set_function(FM_OUTPUT_PIN, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(FM_OUTPUT_PIN);
    
    // Calcula o clock div para a frequência inicial
    float clkdiv = 1.0f + ((108.0 - fm_freq) / 20.0f);
    
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, clkdiv);
    pwm_config_set_wrap(&config, pwm_wrap_base);
    pwm_config_set_phase_correct(&config, true); // Reduz distorção
    pwm_init(slice, &config, true);
}

// Configuração otimizada do ADC
void setup_adc() {
    adc_init();
    adc_gpio_init(AUDIO_INPUT_PIN);
    adc_select_input(2);
    
    // Configura taxa de amostragem mais alta
    adc_set_clkdiv(500000.0f / amostras_por_segundo);
    
    // Inicializa buffer circular
    for (int i = 0; i < avg_samples; i++) {
        audio_buffer[i] = 0;
    }
}

// Configuração dos Botões com Debounce por Hardware
void setup_botoes() {
    // Configura GPIO com pull-up
    gpio_init(BTN_UP);
    gpio_set_dir(BTN_UP, GPIO_IN);
    gpio_pull_up(BTN_UP);
    
    gpio_init(BTN_DOWN);
    gpio_set_dir(BTN_DOWN, GPIO_IN);
    gpio_pull_up(BTN_DOWN);
    
    // Habilita debounce por hardware
    gpio_set_input_hysteresis_enabled(BTN_UP, true);
    gpio_set_input_hysteresis_enabled(BTN_DOWN, true);
}

// Função de ajuste de frequência com proteção de mutex
void ajustar_frequencia() {
    static absolute_time_t last_time = {0};
    absolute_time_t current_time = get_absolute_time();
    
    // Debounce por software (200ms)
    if (absolute_time_diff_us(last_time, current_time) < 200000) return;
    
    float nova_freq = fm_freq;
    bool mudou = false;
    
    if (!gpio_get(BTN_UP)) {
        nova_freq += freq_step;
        mudou = true;
    }
    if (!gpio_get(BTN_DOWN)) {
        nova_freq -= freq_step;
        mudou = true;
    }
    
    if (mudou) {
        // Limita a frequência
        if (nova_freq < freq_min) nova_freq = freq_min;
        if (nova_freq > freq_max) nova_freq = freq_max;
        
        mutex_enter_blocking(&freq_mutex);
        fm_freq = nova_freq;
        
        // Atualiza divisor de clock do PWM
        float clkdiv = 1.0f + ((108.0 - fm_freq) / 20.0f);
        pwm_set_clkdiv(pwm_gpio_to_slice_num(FM_OUTPUT_PIN), clkdiv);
        mutex_exit(&freq_mutex);
        
        last_time = current_time;
    }
}

// Função otimizada de modulação de frequência
void modulate_frequency(uint slice_num, uint16_t audio_sample) {
    // Aplica filtro passa-baixa simples
    static uint16_t last_sample = 0;
    audio_sample = (audio_sample + last_sample) >> 1;
    last_sample = audio_sample;
    
    // Calcula novo wrap value com range limitado
    uint16_t wrap_value = pwm_wrap_base + (audio_sample >> 4);
    if (wrap_value > pwm_wrap_base + 32) wrap_value = pwm_wrap_base + 32;
    if (wrap_value < pwm_wrap_base - 32) wrap_value = pwm_wrap_base - 32;
    
    pwm_set_wrap(slice_num, wrap_value);
    pwm_set_chan_level(slice_num, PWM_CHAN_A, wrap_value >> 1);
}

// Loop principal otimizado
void loop_fm() {
    uint slice_num = pwm_gpio_to_slice_num(FM_OUTPUT_PIN);
    uint64_t next_sample_time = time_us_64();
    const uint64_t sample_interval = 1000000 / amostras_por_segundo;
    
    while (1) {
        // Leitura do ADC com média móvel otimizada
        audio_buffer[buffer_index] = adc_read();
        buffer_index = (buffer_index + 1) & (avg_samples - 1);
        
        uint32_t sum = 0;
        for (int i = 0; i < avg_samples; i++) {
            sum += audio_buffer[i];
        }
        uint16_t audio_avg = sum / avg_samples;
        
        // Modula a frequência
        modulate_frequency(slice_num, audio_avg);
        
        // Verifica botões
        ajustar_frequencia();
        
        // Timing preciso
        next_sample_time += sample_interval;
        busy_wait_until(next_sample_time);
    }
}

int main() {
    stdio_init_all();
    mutex_init(&freq_mutex);
    
    setup_pwm();
    setup_adc();
    setup_botoes();
    
    printf("Transmissor FM iniciado na frequência %.1f MHz\n", fm_freq);
    
    loop_fm();
}