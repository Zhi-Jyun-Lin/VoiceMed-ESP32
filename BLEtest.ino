#include <WiFiManager.h>       // 引入 WiFiManager 庫
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h> // 引入 Async Web Server 庫
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

// 音量控制用可變電阻腳位
#define POT_PIN 34 // 可變電阻接腳
int potValue = 0; // 用於存儲可變電阻值

AsyncWebServer server(80); // 設置 Web 伺服器
void setup() {
  Serial.begin(115200);
  
  // 初始化 WiFiManager
  WiFiManager wifiManager;
  wifiManager.autoConnect("VoiceMed_AP"); // 設置 AP 名稱

  Serial.println("Connected to WiFi!");
  
  // 初始化 I2S
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = I2S_SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S,
    .dma_buf_count = 8,
    .dma_buf_len = I2S_BUFFER_SIZE,
    .use_apll = false
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

  // 設置伺服器路徑，處理音樂數據的上傳
  server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL, 
  [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    // 調整音量
    potValue = analogRead(POT_PIN);
    float volume = map(potValue, 0, 4095, 0, 255); // 將可變電阻的值映射到音量範圍

    // 調整音量後的音訊數據處理
    for (int i = 0; i < len; i++) {
      data[i] = data[i] * volume / 255; // 根據音量縮放音訊數據
    }

    // 播放經過音量調整的音訊數據
    size_t bytes_written;
    i2s_write(I2S_NUM, data, len, &bytes_written, portMAX_DELAY);
    Serial.printf("Received and played %d bytes with volume control\n", bytes_written);

    request->send(200, "text/plain", "Data received and played!"); // 回應請求
  });

  server.begin();

}

void loop() {
    // 主循環無需額外邏輯，音訊數據接收和播放由伺服器處理
  delay(100); // 保持 loop 循環，以確保伺服器運行
}
