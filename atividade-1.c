#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"

#define ROWS 4
#define COLS 4

const uint ROW_PINS[4] = {2, 3, 8, 9};     // Linhas: R1, R2, R3, R4
const uint COL_PINS[4] = {20, 19, 18, 17}; // Colunas: C1, C2, C3, C4

// Mapeamento das teclas
const char KEYPAD[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};

// Mapeamento dos caracteres para Código Morse.
const char *morse_code[ROWS][COLS] = {
    {".----", "..---", "...--", ".-"}, // 1, 2, 3, A
    {"....-", ".....", "-....", "-..."}, // 4, 5, 6, B
    {"--...", "---..", "----.", "-.-."}, // 7, 8, 9, C
    {NULL, "-----", NULL, "-.--"}         // *, 0, #, D
};

// GPIO pins for buzzers
#define BUZZER_DOT 10
#define BUZZER_DASH 22

char scan_keypad()
{
    for (int row = 0; row < 4; row++)
    {
        gpio_put(ROW_PINS[row], 0); // Ativa a linha colocando em LOW
        for (int col = 0; col < 4; col++)
        {
            if (!gpio_get(COL_PINS[col]))
            {
                gpio_put(ROW_PINS[row], 1); // Restaura a linha para HIGH
                return KEYPAD[row][col];    // Retorna a tecla pressionada
            }
        }
        gpio_put(ROW_PINS[row], 1); // linha volta para HIGH após a varredura
    }
    return '\0'; // Nenhuma tecla pressionada
}

void setup_keyboard()
{
    // Configuração das linhas do teclado como saída
    for (int i = 0; i < 4; i++)
    {
        gpio_init(ROW_PINS[i]);
        gpio_set_dir(ROW_PINS[i], GPIO_OUT);
        gpio_put(ROW_PINS[i], 1); // nível alto por padrão
    }

    // Configuração das colunas do teclado
    for (int i = 0; i < 4; i++)
    {
        gpio_init(COL_PINS[i]);
        gpio_set_dir(COL_PINS[i], GPIO_IN);
        gpio_pull_up(COL_PINS[i]); // pull-up interno
    }
}

// Inicializa os buzzers a serem utilizados.
void initialization_buzzers() {
    // Inicializa o pino GPIO para uso geral
    gpio_init(BUZZER_DOT);
    // Configura o pino como saída digital
    gpio_set_dir(BUZZER_DOT, GPIO_OUT);
    // Desliga p pino GPIO configurado como saída digital
    gpio_put(BUZZER_DOT, 0);

    // Refaz o mesmos passos para a outra porta definida para o outro Buzzer
    gpio_init(BUZZER_DASH);
    gpio_set_dir(BUZZER_DASH, GPIO_OUT);
    gpio_put(BUZZER_DASH, 0);
}

// Configuração e PWM para os buzzers
void buzzer_pwm(uint gpio, uint16_t frequency, uint16_t duration_ms) {
    // Configura a pino da gpio pasado como saída de modulação por largura de pulso (PWM).
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    // Mapeia um pino GPIO para uma fatia de PWM.
    uint slice = pwm_gpio_to_slice_num(gpio);

    // Define o período de um sinal PWM em uma determinada fatia (slice).
    pwm_set_wrap(slice, 125000000 / frequency);
    // Configura a modulação por largura de pulso (PWM) em um dos canais de um slice.
    pwm_set_chan_level(slice, PWM_CHAN_A, (125000000 / frequency) / 2);
    // Habilita a geração de um sinal PWM em um determinado slice do controlador.
    pwm_set_enabled(slice, true);

    sleep_ms(duration_ms);

    pwm_set_enabled(slice, false);
    gpio_set_function(gpio, GPIO_FUNC_SIO);
    // Seta a direçao da GPIO como saída
    gpio_set_dir(gpio, GPIO_OUT);
    // Define o estado do pino gpio como baixo (0 V)
    gpio_put(gpio, 0);
}

// Emite a sequência de som em código Morse.
void play_morse_code(const char *morse) {
    // Recebe a strig correspondente ao código morse a ser transformado em som.
    for (size_t i = 0; i < strlen(morse); i++) {
        if (morse[i] == '.') {
            buzzer_pwm(BUZZER_DOT, 5280, 100); // Os pontos: 100ms, increased frequency
        } else if (morse[i] == '-') {
            buzzer_pwm(BUZZER_DASH, 5280, 300); // Traços: 300ms, increased frequency
        }
        sleep_ms(100); // intervalos entre pontos e traços
    }
    sleep_ms(300); // intervalos entre letras
}

// Realiza a leitura do caracter do teclado e conversão em código morse.
void execute_morse_in_buzzers() {
  while (true) {
    char key = scan_keypad(); // Ler novo caracter
    if (key == '#'){
        // Se for pressionado o caracter # sai da opção celecionada.
      printf("Saindo...\n");
      break;
    }
    if (key) {
      if (key == '*')
        printf("Key pressed: %c | Não existe código morse para o caracter. \n", key);
      else
        printf("Key pressed: %c | ", key);

        // detecta qual o código morse para o caracter clicado.
        for (int i = 0; i < ROWS; i++) {
            for (int j = 0; j < COLS; j++) {
                if (KEYPAD[i][j] == key && morse_code[i][j] != NULL) {
                    printf("Morse: %s\n", morse_code[i][j]);
                    play_morse_code(morse_code[i][j]);
                }
            }
        }
    }
    sleep_ms(50); // Delay Debounce
  }
}

int main()
{

    stdio_init_all();
    // configuração do teclado
    setup_keyboard();

    while (1)
    {
        // leitura do teclado
        char key = scan_keypad();
        if (key != '\0')
        { // Se alguma tecla foi pressionada
            printf("tecla pressionada: %c\n", key);
            if (key == 'A')
            {
                // e aqui vem a lógica da tecla
            }
        }
        sleep_ms(100);
    }

    return 0;
}
