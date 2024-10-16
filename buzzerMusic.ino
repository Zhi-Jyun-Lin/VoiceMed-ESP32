int buzzer = 25;

// 定義音符的頻率（以微秒為單位）
const int SI = 956;  // B 音符

// 定義音符的持續時間（以毫秒為單位）
const int halfBeat = 100;  // 半拍
const int fullBeat = 200;  // 一拍

void setup() {
  pinMode(buzzer, OUTPUT);  // 設置蜂鳴器為輸出
}

void loop() {
  // 播放前三個 B 音符（半拍）
  for (int i = 0; i < 3; i++) {
    playTone(SI, halfBeat);  // 播放 B 音符，持續半拍
    delay(halfBeat);  // 每個音符之間休息半拍
  }
  
  // 播放第四個 B 音符（一拍）
  playTone(SI, fullBeat);  // 播放 B 音符，持續一拍
  
  // 休息半拍
  delay(halfBeat);
  
  // 完成旋律後的停頓
  delay(1000);  // 停頓 1 秒
}

// 播放特定頻率的音符
void playTone(int frequency, int duration) {
  int halfPeriod = frequency / 2;  // 計算半周期（正方波）
  long elapsedTime = 0;  // 計算已經過的時間

  // 在持續時間內產生高低電平來模擬聲音
  while (elapsedTime < duration * 1000) {  // duration 轉換為微秒
    digitalWrite(buzzer, HIGH);  // 蜂鳴器高電平
    delayMicroseconds(halfPeriod);  // 等待半周期
    digitalWrite(buzzer, LOW);  // 蜂鳴器低電平
    delayMicroseconds(halfPeriod);  // 等待另一半周期
    elapsedTime += (frequency);  // 累加已經過的時間
  }
}
