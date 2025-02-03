#include <xinu.h>

// Defina os pinos dos LEDs
#define LED_VERMELHO_1 13
#define LED_AMARELO_1 12
#define LED_VERDE_1 11

#define LED_VERMELHO_2 10
#define LED_AMARELO_2 9
#define LED_VERDE_2 8

// Constantes
#define HIGH 1
#define LOW 0
#define OUTPUT 1

#define TEMPO_VERDE 5
#define TEMPO_VERMELHO 5
#define TEMPO_AMARELO 2


// Semáforos para sincronização
sid32 sem_rua1;  // Controla a Rua 1
sid32 sem_rua2;  // Controla a Rua 2

// Funções para controle dos LEDs
void pinMode(uint8 pin, uint8 mode) {
    volatile uint8* ddr = (volatile uint8*)0x24;  // DDRB
    if (pin >= 8 && pin <= 13) *ddr |= (mode == OUTPUT) ? (1 << (pin - 8)) : 0;
}

void digitalWrite(uint8 pin, uint8 value) {
    volatile uint8* port = (volatile uint8*)0x25;  // PORTB
    if (pin >= 8 && pin <= 13) {
        *port = (value == HIGH) ? (*port | (1 << (pin - 8))) : (*port & ~(1 << (pin - 8)));
    }
}

// Tarefa da Rua 1
int tarefa_rua1(void) {
    while (1) {
        // Espera a Rua 2 terminar
        wait(sem_rua1);

        // Ciclo da Rua 1: Verde -> Amarelo -> Vermelho
        digitalWrite(LED_VERMELHO_1, LOW);
        digitalWrite(LED_VERDE_1, HIGH);   // Verde ligado
        sleep(TEMPO_VERDE);                       // 5 segundos

        digitalWrite(LED_VERDE_1, LOW);
        digitalWrite(LED_AMARELO_1, HIGH); // Amarelo ligado
        sleep(TEMPO_AMARELO);                       // 2 segundos

        digitalWrite(LED_AMARELO_1, LOW);
        digitalWrite(LED_VERMELHO_1, HIGH); // Vermelho ligado

        // Libera a Rua 2
        signal(sem_rua2);
    }
    return 0;
}

// Tarefa da Rua 2
int tarefa_rua2(void) {
    while (1) {
        // Espera a Rua 1 terminar
        wait(sem_rua2);

        // Ciclo da Rua 2: Verde -> Amarelo -> Vermelho
        digitalWrite(LED_VERMELHO_2, LOW);
        digitalWrite(LED_VERDE_2, HIGH);   // Verde ligado
        sleep(TEMPO_VERDE);                       // 5 segundos

        digitalWrite(LED_VERDE_2, LOW);
        digitalWrite(LED_AMARELO_2, HIGH); // Amarelo ligado
        sleep(TEMPO_AMARELO);                       // 2 segundos

        digitalWrite(LED_AMARELO_2, LOW);
        digitalWrite(LED_VERMELHO_2, HIGH); // Vermelho ligado

        // Libera a Rua 1
        signal(sem_rua1);
    }
    return 0;
}

// Função principal
int main(void) {
    // Configura pinos como saída
    pinMode(LED_VERMELHO_1, OUTPUT);
    pinMode(LED_AMARELO_1, OUTPUT);
    pinMode(LED_VERDE_1, OUTPUT);
    pinMode(LED_VERMELHO_2, OUTPUT);
    pinMode(LED_AMARELO_2, OUTPUT);
    pinMode(LED_VERDE_2, OUTPUT);

    // Inicializa semáforos
    sem_rua1 = semcreate(0);  // Rua 1 bloqueada inicialmente
    sem_rua2 = semcreate(1);  // Rua 2 liberada inicialmente

    // Estado inicial: Rua 1 vermelha, Rua 2 verde
    digitalWrite(LED_VERMELHO_1, HIGH);
    digitalWrite(LED_VERDE_2, HIGH);

    // Cria tarefas
    resume(create((void*)tarefa_rua1, 128, 20, "Rua 1", 0));
    resume(create((void*)tarefa_rua2, 128, 20, "Rua 2", 0));

    while (1) sleep(1000); // Mantém o siste ma ativo

    return 0;
}