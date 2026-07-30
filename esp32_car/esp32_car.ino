// ==========================================
// ESP32 藍芽 Master ➔ Arduino Mega 2560 網關 (純數值零 Log 快取極速版)
// ==========================================
// 架構革命：
// 1. 完全不暫存任何 Log 文字！
// 2. 藍芽接收端即時提取純數字 (Angle, Target, MotorPWM, Kp, Kd, Ki)。
// 3. API 直接傳輸極輕量化 JSON (單次回應僅 ~70 bytes)，速度提升 10 倍且零記憶體洩漏！
// ==========================================

#include <WiFi.h>
#include <WebServer.h>
#include <BluetoothSerial.h>
#include "esp_gap_bt_api.h"
#include "web_page.h"
#include "wifi_config"


// ------------------------------------------
// replace to your wifi SSID and Password
// ------------------------------------------
const char* WIFI_SSID = MY_WIFI_SSID;      // 替換為您的 WiFi 名稱
const char* WIFI_PASS = MY_WIFI_PASS;  // 替換為您的 WiFi 密碼
// ------------------------------------------

BluetoothSerial SerialBT;
WebServer server(80);

// 已透過 AT 指令確認之 ZS-040 實體 MAC 地址
String target_bt_name = ZS_040_MAC; 
bool bt_connected = false;
bool is_connecting = false;

// 全域純數值姿態變數 (不存任何 Log 文字字串)
float live_angle  = 23.0;
float live_target = 23.0;
int   live_motor  = 0;
float live_kp     = 20.0;
float live_kd     = 0.5;
float live_ki     = 0.0;

// 行解析臨時緩衝區
char line_acc_buf[128];
uint8_t line_acc_idx = 0;

void set_esp_bt_pin(const char* pin) {
  esp_bt_pin_code_t pin_code;
  int len = strlen(pin);
  for (int i = 0; i < len && i < 16; i++) {
    pin_code[i] = pin[i];
  }
  esp_bt_gap_set_pin(ESP_BT_PIN_TYPE_FIXED, len, pin_code);
}

bool parse_mac(const String& str, uint8_t mac[6]) {
  int values[6];
  if (sscanf(str.c_str(), "%x:%x:%x:%x:%x:%x", &values[0], &values[1], &values[2], &values[3], &values[4], &values[5]) == 6 ||
      sscanf(str.c_str(), "%x-%x-%x-%x-%x-%x", &values[0], &values[1], &values[2], &values[3], &values[4], &values[5]) == 6) {
    for (int i = 0; i < 6; i++) {
      mac[i] = (uint8_t)values[i];
    }
    return true;
  }
  return false;
}

void connect_bt_async(void *pvParameters) {
  is_connecting = true;
  Serial.print(F("正在直連 ZS-040 實體 MAC 地址: "));
  Serial.println(target_bt_name);

  set_esp_bt_pin("1234");

  uint8_t mac[6];
  if (parse_mac(target_bt_name, mac)) {
    bt_connected = SerialBT.connect(mac);
  } else {
    bt_connected = SerialBT.connect(target_bt_name);
  }

  if (bt_connected) {
    Serial.println(F(">>> 成功連線至 Arduino ZS-040 (98:D3:31:F4:20:B8)！ <<<"));
  } else {
    Serial.println(F("藍芽連線失敗，等待重試..."));
  }

  is_connecting = false;
  vTaskDelay(100 / portTICK_PERIOD_MS);
  vTaskDelete(NULL);
}

void trigger_bt_connect() {
  if (is_connecting) return;
  xTaskCreatePinnedToCore(connect_bt_async, "bt_connect_task", 4096, NULL, 1, NULL, 0);
}

// 數據行提取解析器 (T:23.0,A:24.1,M:45,P:20.0,D:0.5,I:0.0)
void process_telemetry_line(char* line) {
  float t, a, p, d, i_val;
  int m;
  if (sscanf(line, "T:%f,A:%f,M:%d,P:%f,D:%f,I:%f", &t, &a, &m, &p, &d, &i_val) >= 3) {
    live_target = t;
    live_angle  = a;
    live_motor  = m;
    live_kp     = p;
    live_kd     = d;
    live_ki     = i_val;
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println(F("\n=============================================="));
  Serial.println(F("  ESP32 Bluetooth Gateway (Pure Numeric STA)"));
  Serial.println(F("=============================================="));

  // 1. 連線至家庭 WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print(F("正在連線至 WiFi: "));
  Serial.println(WIFI_SSID);

  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries < 30) {
    delay(500);
    Serial.print(F("."));
    tries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(F("\n✅ WiFi 連線成功！"));
    Serial.print(F("👉 請在手機/電腦瀏覽器輸入網址: http://"));
    Serial.println(WiFi.localIP());
  } else {
    Serial.println(F("\n⚠️ WiFi 連線逾時，請檢查 WIFI_SSID 與 WIFI_PASS 設定！"));
  }

  // 2. 啟動 ESP32 藍芽 Master 模式
  SerialBT.begin("ESP32_Master_Gateway", true);
  Serial.println(F("ESP32 藍芽 Master 模式已啟動"));

  // 3. Web Server 路由
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", INDEX_HTML);
  });

  server.on("/api/send", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    if (server.hasArg("cmd")) {
      String cmd = server.arg("cmd");
      if (bt_connected) {
        SerialBT.println(cmd);
        Serial.print(F("[WiFi -> BT] 發送: "));
        Serial.println(cmd);
      }
      server.send(200, "application/json", "{\"status\":\"ok\"}");
    } else {
      server.send(400, "application/json", "{\"status\":\"error\"}");
    }
  });

  server.on("/api/status", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.sendHeader("Access-Control-Allow-Origin", "*");

    // 單次 JSON 僅 ~80 字元
    char json_buf[160];
    snprintf(json_buf, sizeof(json_buf),
             "{\"connected\":%s,\"angle\":%.1f,\"target\":%.1f,\"motor\":%d,\"kp\":%.1f,\"kd\":%.2f,\"ki\":%.2f}",
             bt_connected ? "true" : "false",
             live_angle, live_target, live_motor, live_kp, live_kd, live_ki);

    server.send(200, "application/json", json_buf);
  });

  server.begin();
  Serial.println(F("Web Server 啟動完成！"));

  trigger_bt_connect();
}

void loop() {
  server.handleClient();

  // 藍芽斷開時，每 5 秒自動重連一次
  static unsigned long last_reconnect = 0;
  if (!bt_connected && !is_connecting && (millis() - last_reconnect > 5000)) {
    last_reconnect = millis();
    trigger_bt_connect();
  }

  // 藍芽接收端即時提取純數字
  while (bt_connected && SerialBT.available() > 0) {
    char c = SerialBT.read();
    Serial.write(c);

    if (c == '\n' || c == '\r') {
      if (line_acc_idx > 0) {
        line_acc_buf[line_acc_idx] = '\0';
        process_telemetry_line(line_acc_buf);
        line_acc_idx = 0;
      }
    } else {
      if (line_acc_idx < sizeof(line_acc_buf) - 1) {
        line_acc_buf[line_acc_idx++] = c;
      }
    }
  }
}