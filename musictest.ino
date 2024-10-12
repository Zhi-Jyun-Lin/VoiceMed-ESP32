#include <driver/i2s.h>
#include <Arduino.h>

// I2S 設定
#define I2S_NUM         I2S_NUM_0
#define I2S_SAMPLE_RATE 44100
#define I2S_BUFFER_SIZE 1024

// 自訂 I2S 引腳
#define I2S_BCK_PIN    26 // BCLK
#define I2S_WS_PIN     27 // LRC (WS)
#define I2S_DATA_PIN   25 // DIN

const float NOTE_DURATION = 0.5; // 每個音符的持續時間 (秒)

// 定義音符及其頻率 (Hz)
const float notes[] = {
    261.63, // C4
    293.66, // D4
    329.63, // E4
    349.23, // F4
    392.00, // G4
    440.00, // A4
    493.88, // B4
    523.25  // C5
};

void setup() {
    // 初始化 I2S 配置
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX), // 使用點語法
        .sample_rate = I2S_SAMPLE_RATE,  // 設定取樣率
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
        .dma_buf_count = 8,
        .dma_buf_len = I2S_BUFFER_SIZE,
        .use_apll = false,
    };

    // 設定 I2S 引腳配置
    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_BCK_PIN,
        .ws_io_num = I2S_WS_PIN,
        .data_out_num = I2S_DATA_PIN,
        .data_in_num = I2S_PIN_NO_CHANGE  // 這裡如果不需要輸入，則設為 I2S_PIN_NO_CHANGE
    };

    // I2S 端口初始化
    i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM, &pin_config);
}

void loop() {
    /*
    // 播放音訊資料-隨機數據）
    int16_t buffer[I2S_BUFFER_SIZE];
    for (int i = 0; i < I2S_BUFFER_SIZE; i++) {
        buffer[i] = random(-32768, 32767); // 隨機音訊數據
    }

    size_t bytes_written;
    i2s_write(I2S_NUM, buffer, sizeof(buffer), &bytes_written, portMAX_DELAY);
    
    delay(100); // 讓音訊播放一小段時間
    */
     // 播放旋律 ( C4, D4, E4, F4)
    for (int i = 0; i < 4; i++) {
        playTone(notes[i]);
        delay(NOTE_DURATION * 1000); // 音符持續時間
    }

    while (true); // 停止程式
}

// 播放音符
void playTone(float frequency) {
    int16_t buffer[I2S_BUFFER_SIZE];

    // 生成正弦波音訊數據
    for (int i = 0; i < I2S_BUFFER_SIZE; i++) {
        buffer[i] = (int16_t)(32767 * sin(2 * PI * frequency * (i / (float)I2S_SAMPLE_RATE)));
    }

    size_t bytes_written;
    i2s_write(I2S_NUM, buffer, sizeof(buffer), &bytes_written, portMAX_DELAY);
}
