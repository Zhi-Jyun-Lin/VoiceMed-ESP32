#include <Wire.h>
#include <U8g2lib.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>


U8G2_SH1106_128X64_NONAME_F_HW_I2C oled1(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);// 創建第一個 I2C 地址 0x3C
U8G2_SH1106_128X64_NONAME_F_HW_I2C oled2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);// 創建第一個 I2C 地址 0x3C

const char* ssid = "CHT304";       // WiFi名稱
const char* password = "12345678";   // WiFi密碼

const byte anaPin = 34; // 可變電阻引腳
short val;
short previousVal = -1; // 用來保存之前的可變電阻值

const int redLED = 12; // 紅燈
const int dayPins[] = {18, 2, 4, 19, 5, 17, 16};  // 星期一到星期天對應的引腳

// 按鈕
const int buttonPin = 32 ; // 設定一個按鈕引腳來關閉紅燈
bool buttonState = false;  // 儲存按鈕狀態
bool redLedOn = false;
bool buttonPressed = false;  // 用來記錄按鈕是否已按下
bool acknowledgedAlarm = false;//提醒時段確認
bool lastButtonState = false;  // 儲存上一次按鈕的狀態
bool alarmActive = false;  // 追蹤提醒是否進行中

// NTP 設定
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 28800, 60000); // UTC+8 (台灣時間)

// 設定鬧鐘時間
int alarmHour = 22;  // 設定紅燈亮起的時間 (後續需要傳輸資料進來這邊)
int alarmMinute = 37;

void setup() {
  Serial.begin(115200);
  Wire.begin();

  connectToWiFi();
  timeClient.begin();

  // 設定燈泡輸出
  pinMode(redLED, OUTPUT);
  digitalWrite(redLED, LOW);
  for (int i = 0; i < 7; i++) {
    pinMode(dayPins[i], OUTPUT);
    digitalWrite(dayPins[i], LOW);
  }
  // 設定按鈕
  pinMode(buttonPin, INPUT_PULLUP);  // 使用內部上拉電阻
  Serial.println("按鈕狀態監控啟動");


  oled1.setI2CAddress(0x3C << 1);  // 設置第一塊 OLED 地址為 0x3C
  oled1.begin();
  oled2.setI2CAddress(0x3D << 1);  // 設置第二塊 OLED 地址為 0x3D
  oled2.begin();
  /*
    // 檢查第二個 OLED 連接
    if (checkI2CConnection(0x3D)) {
      Serial.println("OLED display 2 connected successfully!");
    } else {
      Serial.println("Failed to connect to OLED display 2.");
      while (true);  // 停止執行，直到手動重置
    }
  */
  // 初始化第二個 OLED

  oled2.clearBuffer();
  oled2.setFont(u8g2_font_ncenB14_tr);  // 設置字體大小為 24pt
  oled2.drawStr(20, 32, "WELCOME");       // 在畫面中央顯示 "WELCOME"
  oled2.sendBuffer();
  /*
    // 檢查 第一個 OLED連接
    if (checkI2CConnection(0x3C)) {
      Serial.println("OLED display 1 connected successfully!");
    } else {
      Serial.println("Failed to connect to OLED display 1.");
      while (true);  // 停止執行，直到手動重置
    }
  */
  // 初始化第一個 OLED

  oled1.clearBuffer();
  oled1.setFont(u8g2_font_ncenB14_tr);
  oled1.drawStr(0, 32, "VoiceMed");
  oled1.sendBuffer();

  delay(2000);

}

void loop() {

  timeClient.update();

  weekdayLed();
  updateTimeDisplay();

  // 取得當前時間
  int currentHour = timeClient.getHours();
  int currentMinute = timeClient.getMinutes();
  int currentWeekday = timeClient.getDay();  // 0 是星期天，1 是星期一，依此類推

      // 可變電阻調音量
  val = analogRead(anaPin); // 讀取可變電阻的值

  if (!alarmActive) {
  // 檢查音量是否有變化
  if (abs(val - previousVal) > 5) { // 只在變化超過 5 時更新顯示
    previousVal = val; // 更新之前的值

    // 將讀取值轉換為音量百分比
    int volume = map(val, 0, 4095, 0, 100); // 將0-4095的值映射到0-100的範圍
    Serial.print("Volume: ");
    Serial.print(volume);
    Serial.println("%");

    // 顯示音量大小
    oled2.clearBuffer(); // 清除緩衝區
    oled2.setFont(u8g2_font_ncenB10_tr); // 設定字型
    oled2.drawStr(0, 35, "Volume: "); // 顯示文字
    oled2.setCursor(65, 35);
    oled2.print(volume); // 顯示音量
    oled2.drawStr(90, 35, "%"); // 顯示百分比符號
    oled2.sendBuffer();
  } else {
    displayVoiceMed();
  }
  }
  
// 判斷是否到指定時間，點亮紅燈，顯示服藥提醒
  if (!buttonPressed && !acknowledgedAlarm && (currentHour == alarmHour && currentMinute >= alarmMinute) && !redLedOn) {
    alarmActive = true;  // 進入提醒狀態
    digitalWrite(redLED, HIGH);  // 打開紅燈
    redLedOn = true;
    Serial.println("Red LED ON (Alarm Time)");
    
    if (alarmHour >= 5 && alarmHour < 11) {
      oled2.clearBuffer();
      oled2.setFont(u8g2_font_ncenB14_tr);  // 設置字體大小為 24pt
      oled2.drawStr(10, 30, "Breakfast");
      oled2.sendBuffer();
      displayNotYet();

    } else if (alarmHour >= 11 && alarmHour < 17) {
      oled2.clearBuffer();
      oled2.setFont(u8g2_font_ncenB14_tr);  // 設置字體大小為 24pt
      oled2.drawStr(10, 30, "Lunch");
      oled2.sendBuffer();
      displayNotYet();

    } else if (alarmHour >= 17 && alarmHour < 21) {
      oled2.clearBuffer();
      oled2.setFont(u8g2_font_ncenB14_tr);  // 設置字體大小為 24pt
      oled2.drawStr(15, 30, "Dinner");
      oled2.sendBuffer();
      displayNotYet();

    } else {
      oled2.clearBuffer();
      oled2.setFont(u8g2_font_ncenB10_tr);  // 設置字體大小為 24pt
      oled2.drawStr(10, 30, "Before Sleep");
      oled2.sendBuffer();
      displayNotYet();
    }
       
  }

  // 按鈕按下，提示紅燈關閉
    if (alarmActive && digitalRead(buttonPin) == LOW && redLedOn) {
      alarmActive = false;  // 退出提醒狀態
      digitalWrite(redLED, LOW);  // 關閉紅燈
      redLedOn = false;
      buttonPressed = true;  // 設置按鈕為已按下狀態
      acknowledgedAlarm = true;  // 設置提醒已確認
      Serial.println("Red LED OFF (Button Pressed)");
      delay(500);  // 防止按鈕抖動
      oled2.clearBuffer();
      oled2.setFont(u8g2_font_ncenB24_tr);  // 設置字體大小為 24pt
      oled2.drawStr(30, 45, "OK!");
      oled2.sendBuffer();
      delay(2000);
    }

     delay(1000);  // 每秒更新一次
}
  void displayVoiceMed() {
    oled2.clearBuffer();
    oled2.setFont(u8g2_font_ncenB14_tr);  // 設置字體大小為 24pt
    oled2.drawStr(15, 42, "VoiceMed");
    oled2.sendBuffer();
  }
  
  void displayNotYet() {
    String notyet = "NOt yet!";
    int ntWidth = oled2.getStrWidth(notyet.c_str());
    oled2.setFont(u8g2_font_ncenB10_tr);  // 設置字體大小為 24pt
    oled2.drawStr((128 - ntWidth) / 2, 52, notyet.c_str());
    oled2.sendBuffer();
  }
  
  void connectToWiFi() {
    Serial.print("Connecting to WiFi");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.print(".");
    }
    Serial.println("\nWiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }
  void weekdayLed() {
    // 更新 NTP 時間
    timeClient.update();

    // 取得當前時間
    int currentHour = timeClient.getHours();
    int currentMinute = timeClient.getMinutes();
    int currentWeekday = timeClient.getDay();  // 0 是星期天，1 是星期一，依此類推

    // 點亮當天對應的燈泡
    if (currentWeekday > 0 && currentWeekday <= 6) {
      digitalWrite(dayPins[currentWeekday - 1], HIGH);
    } else {
      digitalWrite(dayPins[6], HIGH); // 星期天
    }
  }
  
  void updateTimeDisplay() {
    timeClient.update();
    // 取得小時與分鐘
    String formattedTime = timeClient.getFormattedTime();
    int currentHour = timeClient.getHours();
    int currentMinute = timeClient.getMinutes();

    // 取得 epochTime
    unsigned long epochTime = timeClient.getEpochTime();
    Serial.print("Epoch Time: ");
    Serial.println(epochTime);

    // 檢查 epochTime 是否為 0，重新更新時間
    if (epochTime == 0) {
      timeClient.forceUpdate();
      epochTime = timeClient.getEpochTime();
    }

    // 使用 localtime 來轉換 epochTime
    time_t rawTime = (time_t)epochTime;
    struct tm *ptm = localtime(&rawTime);

    // 獲取當前年份、月份和日期
    int currentYear = ptm->tm_year + 1900;
    int currentMonth = ptm->tm_mon + 1;
    int currentDay = ptm->tm_mday;
    int dayOfWeek = ptm->tm_wday;

    // 將星期幾轉換為字串
    String daysOfWeek[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    String weekDayStr = daysOfWeek[dayOfWeek];

    // 清空 OLED 畫面
    oled1.clearBuffer();

    // 第一排顯示時間 00:00
    String timeStr = (currentHour < 10 ? "0" : "") + String(currentHour) + ":" + (currentMinute < 10 ? "0" : "") + String(currentMinute);
    oled1.setFont(u8g2_font_ncenB24_tr);  // 設置字體大小為 24pt
    int timeWidth = oled1.getStrWidth(timeStr.c_str());
    oled1.drawStr(128 - timeWidth , 25, timeStr.c_str());  // 水平居中顯示時間

    // 第二排顯示日期 2024/10/9
    String dateStr = String(currentYear) + "/" + (currentMonth < 10 ? "0" : "") + String(currentMonth) + "/" + (currentDay < 10 ? "0" : "") + String(currentDay);
    oled1.setFont(u8g2_font_ncenB08_tr);  // 設置字體大小為 8pt
    int dateWidth = oled1.getStrWidth(dateStr.c_str());
    oled1.drawStr(128 - dateWidth, 45, dateStr.c_str());  // 水平居中顯示日期

    // 第三排顯示星期幾
    int weekDayWidth = oled1.getStrWidth(weekDayStr.c_str());
    oled1.drawStr(128 - weekDayWidth, 62, weekDayStr.c_str());  // 水平居中顯示星期幾

    // 傳送畫面內容到 OLED
    oled1.sendBuffer();

    // 調試輸出當前星期幾到序列監視器
    Serial.print("Current day of week: ");
    Serial.println(weekDayStr);

    delay(1000);  // 每秒更新一次
  }

  // 檢查 I2C 連接的函數
  bool checkI2CConnection(uint8_t address) {
    Wire.beginTransmission(address);
    return (Wire.endTransmission() == 0);  // 如果成功，返回 true
  }
