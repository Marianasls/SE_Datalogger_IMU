#include <stdio.h>
#include <string.h>
#include <math.h>

#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"

#include "globals.h"
#include "ssd1306.h"
#include "font.h"
#include "sdcard_fatFS.h"
#include "mpu6050.h"
#include "buzzer.h"


// Trecho para modo BOOTSEL usando o botão B
#include "pico/bootrom.h"

// Nome do arquivo CSV
#define CSV_FILENAME "imu_data1.csv"

// variaveis globais
ssd1306_t ssd;      // Inicializa a estrutura do display
int8_t read_state = 0, mount_state = 0; // Estado de leitura e montagem do cartão SD

// Adicione variáveis globais para debounce
volatile uint32_t last_btn_time_a = 0;
volatile uint32_t last_btn_time_b = 0;
volatile uint32_t last_btn_time_sw = 0;
#define DEBOUNCE_MS 200

// Declaração de funções
void save_csv_data_to_sd(int16_t accel[3], int16_t gyro[3]);
void mpu6050_init();
void gpio_irq_handler(uint gpio, uint32_t events);
int capture_data_and_save();
void gpio_init_all();
void set_rgb_color(const char *color);
void update_display(char *status);

int main() {
    stdio_init_all(); // Inicializa a biblioteca padrão de entrada/saída
    gpio_init_all(); // Inicializa os pinos GPIO

    // Inicializa o sensor MPU6050
    mpu6050_reset();

    // Escreve cabeçalho no arquivo CSV (apenas se arquivo for novo)
    FIL file;
    FRESULT res = f_open(&file, CSV_FILENAME, FA_WRITE | FA_OPEN_APPEND | FA_CREATE_ALWAYS);
    if (res == FR_OK && f_size(&file) == 0) {
        const char *header = "ax,ay,az,gx,gy,gz\n";
        UINT bw;
        f_write(&file, header, strlen(header), &bw);
    }
    if (res == FR_OK) f_close(&file);
    
    printf("Datalogger IMU iniciado.\n Estados iniciais: leitura = %d, montagem = %d\n", read_state, mount_state);
    printf("O LED RGB indica o status do sistema:\n");
    printf(" - Amarelo: Sistema inicializando / Montando cartão SD. \n");
    printf(" - Verde: Sistema pronto para iniciar gravação\n");
    printf(" - Azul: Acessando o cartão para Leitura ou Escrita\n");
    printf(" - Vermelho: Captura de dados em andamento.\n");
    printf(" - Roxo: Erro ao montar/desmontar SD\n");
    
    update_display("Inicializando...");
    bool cor = true;    
    while(1) {
        // Exibição no display
        ssd1306_fill(&ssd, !cor); 
        ssd1306_draw_string(&ssd, "Datalogger", 4, 4);
        update_display("Aguardando...");

        // Captura dados do sensor e salva no cartão SD
        if (mount_state && read_state) {
            printf("Gravando...\n");
            set_rgb_color("vermelho");
            update_display("Gravando...");
            play_buzzer(BUZZER_ALERT_FREQ); // Toca o buzzer ao iniciar a captura
            sleep_ms(500); // Aguarda o buzzer tocar antes de iniciar a captura
            stop_buzzer();

            int samples = capture_data_and_save();
 
            play_buzzer(150);
            sleep_ms(300); // Aguarda o buzzer tocar antes de iniciar a captura
            stop_buzzer();

            printf("Dados salvos!\n");
            set_rgb_color("verde");
            
            char str_samples[20];
            snprintf(str_samples, sizeof(str_samples), "Amostras: %d", samples);
            ssd1306_draw_string(&ssd, "Dados salvos!", 8, 30);
            ssd1306_draw_string(&ssd, str_samples, 8, 40);
            ssd1306_send_data(&ssd);

        }
        sleep_ms(100); // Intervalo de 100ms entre leituras
        stop_buzzer();
    }
}

void update_display(char *status) {
    // char str_msg[20];
    // snprintf(str_msg,  sizeof(str_msg),  "%s", status);
    ssd1306_draw_string(&ssd, status, 8, 20);
    ssd1306_send_data(&ssd);
}

int capture_data_and_save() {
    int16_t accel[3], gyro[3], temp;
    int sample_count = 0; // Contador de amostras coletadas
    while (mount_state && read_state) {
        // Captura dados do sensor
        mpu6050_read_raw(accel, gyro, &temp);

        // Salva dados no arquivo CSV
        save_csv_data_to_sd(accel, gyro);

        sample_count++; // Incrementa o contador de amostras

        printf("Acel: %d,%d,%d | Giro: %d,%d,%d | Amostras: %d\n", accel[0], accel[1], accel[2], gyro[0], gyro[1], gyro[2], sample_count);
        sleep_ms(100); // 10 leituras por segundo | Ajuste o intervalo conforme necessário 
    }
    printf("Total de amostras coletadas: %d\n", sample_count);
    return sample_count;
}

void gpio_irq_handler(uint gpio, uint32_t events) {
    uint32_t now = to_ms_since_boot(get_absolute_time());

    if (gpio == BTNA) {
        if (now - last_btn_time_a < DEBOUNCE_MS) return;
        last_btn_time_a = now;
        // Inicia / finaliza leitura no cartão SD
        read_state = !read_state; // Inverte o estado de leitura
        set_rgb_color("azul");

    } else if (gpio == BTNB) {
        if (now - last_btn_time_b < DEBOUNCE_MS) return;
        last_btn_time_b = now;
        mount_state = !mount_state; // Inverte o estado de leitura
        // monta / desmonta o cartão SD

        if (mount_state) {
            set_rgb_color("amarelo");
            if (run_mount()){
                printf("Cartão SD montado\n");
                update_display("SD montado");
                set_rgb_color("verde");
            } else {
                update_display("Erro ao montar SD");
                printf("Erro ao montar o cartão SD\n");
            }

        } else {
            if(run_unmount())
                printf("Cartão SD desmontado\n");
            else 
                set_rgb_color("roxo");
        }

    } else if (gpio == SW_BNT) {
        if (now - last_btn_time_sw < DEBOUNCE_MS) return;
        last_btn_time_sw = now;
        reset_usb_boot(0, 0);
    }
}

// Função para salvar dados em formato CSV no cartão SD
void save_csv_data_to_sd(int16_t accel[3], int16_t gyro[3]) {
    // Escreve linha CSV: ax,ay,az,gx,gy,gz\n
    char line[128];
    snprintf(line, sizeof(line), "%d,%d,%d,%d,%d,%d\n",
             accel[0], accel[1], accel[2], gyro[0], gyro[1], gyro[2]);
    
    save_data_to_sd(CSV_FILENAME, line); // Usa a função genérica para salvar dados
}

// Função para acender LEDs RGB conforme cor desejada
void set_rgb_color(const char *color) {
    // Apaga todos os LEDs antes de acender a cor desejada
    gpio_put(LED_RED, false);
    gpio_put(LED_GREEN, false);
    gpio_put(LED_BLUE, false);

    if (strcmp(color, "vermelho") == 0) {
        gpio_put(LED_RED, true);
    } else if (strcmp(color, "verde") == 0) {
        gpio_put(LED_GREEN, true);
    } else if (strcmp(color, "azul") == 0) {
        gpio_put(LED_BLUE, true);
    } else if (strcmp(color, "amarelo") == 0) {
        gpio_put(LED_RED, true);
        gpio_put(LED_GREEN, true);
    } else if (strcmp(color, "roxo") == 0) {
        gpio_put(LED_RED, true);
        gpio_put(LED_BLUE, true);
    }
}

void gpio_init_all() {
    // Inicialização do modo BOOTSEL com botão B
    gpio_init(BTNB);
    gpio_set_dir(BTNB, GPIO_IN);
    gpio_pull_up(BTNB);
    gpio_set_irq_enabled_with_callback(BTNB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    gpio_init(BTNA);
    gpio_set_dir(BTNA, GPIO_IN);
    gpio_pull_up(BTNA);
    gpio_set_irq_enabled_with_callback(BTNA, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    // remover apos testes
    gpio_init(SW_BNT);
    gpio_set_dir(SW_BNT, GPIO_IN);
    gpio_pull_up(SW_BNT);
    gpio_set_irq_enabled_with_callback(SW_BNT, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    // Inicializa pinos do led RGB
    gpio_init(LED_GREEN);
    gpio_set_dir(LED_GREEN, GPIO_OUT);
    gpio_init(LED_RED);
    gpio_set_dir(LED_RED, GPIO_OUT);
    gpio_init(LED_BLUE);
    gpio_set_dir(LED_BLUE, GPIO_OUT);

    // Inicialização do sensor MPU6050 no canal I2C0
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    bi_decl(bi_2pins_with_func(I2C_SDA, I2C_SCL, GPIO_FUNC_I2C));

    // Canal I2C1 do Display funcionando em 400Khz.
    i2c_init(I2C_PORT_DISP, 400 * 1000);
    gpio_set_function(I2C_SDA_DISP, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_DISP, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_DISP);
    gpio_pull_up(I2C_SCL_DISP);
    
    ssd1306_init(&ssd, DISPLAY_WIDTH, DISPLAY_HEIGHT, false, DISPLAY_ADDRESS, I2C_PORT_DISP);
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);

    // Limpa o display
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);
}
