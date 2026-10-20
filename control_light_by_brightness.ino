#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include <HTTPClient.h>

constexpr int SENSOR_PIN = A0;

const char* SSID = "";
const char* PASSWORD = "";
const char* NTP_SERVER = "ntp.nict.jp";

// 実行時刻（日本時間）
constexpr int TARGET_HOUR = 0;
constexpr int TARGET_MINUTE = 9;

// 最終実行日時 yyyyMMdd
int lastExecutionDate = -1;


void setup() {
  Serial.begin(115200);

  analogReadResolution(12);
  pinMode(SENSOR_PIN, INPUT);
  
  // Wifi接続
  WiFi.begin(SSID, PASSWORD);
  // 接続完了まで待機
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }
  Serial.print("Wifi Connected");

  syncDateTime();
}

void loop() {
  // 時刻同期を待つ
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo, 1000)) {
    Serial.println("Failed to get time");
    delay(1000);
    return; // 時刻の取得に失敗した場合は0を返す
  }

    // 年と通算日を使って日付を識別
  int today = (timeinfo.tm_year + 1900) * 10000 + (timeinfo.tm_mon + 1) * 100 + timeinfo.tm_mday;

  // 指定時刻かつ本日未実行の場合
  if (timeinfo.tm_hour == TARGET_HOUR && timeinfo.tm_min == TARGET_MINUTE && lastExecutionDate != today) {
    // 同日の再実行を防止
    lastExecutionDate = today;

    executeTask();
  }
}

void syncDateTime() {
  // NTPサーバーから時刻を取得
  configTime(9 * 3600, 0, NTP_SERVER);
  // 時刻同期を待つ
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to get time");
  }
}

int getBrightnessSensorValue() {
  // センサーの値を取得
  int lightValue = analogRead(SENSOR_PIN);

  // 確認用
  Serial.println(lightValue);

  // 部屋のシーリングライト 点灯時: 約1200~1600
  // 消灯時: 約5~45
  return lightValue;

}

void executeTask() {
  
  int lightValue = getBrightnessSensorValue();
  
  Serial.print("Light: ");
  Serial.println(lightValue);

  sendHttpGet();

}


void sendHttpGet() {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  http.begin("http://192.168.10.160:3000/discord");

  int httpCode = http.GET();

  if (httpCode > 0) {
    Serial.printf("HTTP Status: %d\n", httpCode);
    Serial.println(http.getString());
  } else {
    Serial.printf("HTTP Error: %s\n",
                  http.errorToString(httpCode).c_str());
  }

  http.end();
}
