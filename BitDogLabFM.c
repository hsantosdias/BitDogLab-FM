
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"

#define AUDIO_INPUT_PIN 28
#define FM_OUTPUT_PIN 13
#define BTN_UP 5
#define BTN_DOWN 6

float fm_freq = 90.0; // Frequência inicial em MHz

void setup_fm_transmitter() {
    gpio_set_function(FM_OUTPUT_PIN, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(FM_OUTPUT_PIN);
    
    pwm_set_clkdiv(slice, 1.0);  
    pwm_set_wrap(slice, 255);  
    pwm_set_enabled(slice, true);
}

void adjust_frequency() {
    if (!gpio_get(BTN_UP)) fm_freq += 0.1;
    if (!gpio_get(BTN_DOWN)) fm_freq -= 0.1;

    pwm_set_clkdiv(pwm_gpio_to_slice_num(FM_OUTPUT_PIN), fm_freq * 10);
}

void loop_fm() {
    while (1) {
        uint16_t audio = adc_read();
        uint16_t freq_shift = audio / 4;
        pwm_set_wrap(pwm_gpio_to_slice_num(FM_OUTPUT_PIN), 255 + freq_shift);

        adjust_frequency();
        sleep_ms(50);
    }
}

int main() {
    stdio_init_all();
    adc_init();
    adc_gpio_init(AUDIO_INPUT_PIN);
    adc_select_input(0);
    
    gpio_init(BTN_UP);
    gpio_set_dir(BTN_UP, GPIO_IN);
    gpio_pull_up(BTN_UP);

    gpio_init(BTN_DOWN);
    gpio_set_dir(BTN_DOWN, GPIO_IN);
    gpio_pull_up(BTN_DOWN);

    setup_fm_transmitter();
    loop_fm();
}
