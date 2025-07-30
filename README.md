# Datalogger IMU - Raspberry Pi Pico

Projeto de um datalogger para aquisição de dados de aceleração e giroscópio utilizando o sensor MPU6050, armazenamento em cartão SD (FAT32) e visualização de status em display OLED SSD1306. O sistema utiliza o Raspberry Pi Pico W acoplado a placa BitDogLab.

## Funcionalidades

- **Aquisição contínua** dos dados de aceleração (X, Y, Z) e giroscópio (X, Y, Z) do sensor MPU6050 via I2C.
- **Armazenamento estruturado** dos dados em arquivo `.csv` no cartão SD.
- **Interface visual** com display OLED SSD1306 para status do sistema.
- **Indicação de status** por LED RGB (cores para cada estado do sistema).
- **Controle por botões** para iniciar/parar gravação e montar/desmontar o cartão SD.
- **Feedback sonoro** com buzzer.
- **Modo BOOTSEL** para atualização de firmware via botão.

## Componentes Utilizados

- Raspberry Pi Pico W 
- Sensor IMU MPU6050
- Cartão microSD + módulo adaptador SPI
- Display OLED SSD1306 (I2C)
- LED RGB
- Buzzer
- Botões A e B (para controle de gravação e montagem do SD)

## Esquemático (Resumo)

- **I2C0**: MPU6050 (SDA/SCL)
- **I2C1**: SSD1306 (SDA/SCL)
- **SPI**: Cartão SD
- **GPIOs**: LED RGB, Buzzer, Botões

## Como Compilar e Executar

1. **Clone o repositório** e acesse a pasta do projeto:
   ```sh
   git clone https://github.com/Marianasls/SE_Datalogger_IMU.git
   cd Datalogger_IMU
   ```

2. **Crie uma pasta de build**:
   ```sh
   mkdir build
   cd build
   ```

3. **Configure o projeto com CMake**:
   ```sh
   cmake ..
   ```

4. **Compile o projeto**:
   ```sh
   make
   ```

5. **Grave o arquivo `.uf2`** gerado na Pico (conecte via USB e pressione BOOTSEL, copie o arquivo para a unidade montada).

## Uso

- **Botão A**: Inicia/para a gravação dos dados.
- **Botão B**: Monta/desmonta o cartão SD.
- **LED RGB**: Indica o status do sistema (amarelo, verde, azul, vermelho, roxo).
- **Display OLED**: Mostra mensagens de status.
- **Arquivo CSV**: Os dados são salvos em `imu_data.csv` no cartão SD.

## Observações

- Certifique-se de que o cartão SD está formatado em FAT32.
- O projeto utiliza bibliotecas do SDK Pico e FatFS.
- Ajuste os pinos conforme seu hardware, se necessário.

