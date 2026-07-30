// ==========================================
// ZS-040 / HC-05 藍芽 AT 指令模式查詢與設定工具 (Arduino Mega 2560)
// ==========================================
// 進入 AT 模式步驟 (重要！)：
// 1. 斷開 ZS-040 的電源 (拔掉 5V 紅線)。
// 2. 按住 ZS-040 模組上的【微型按鈕 (KEY 按鈕)】不放。
// 3. 插回 5V 電源，然後放開按鈕。
// 4. 觀察 ZS-040 紅燈：若變為「每 2 秒慢閃一次」，代表成功進入 AT 模式！
//
// 接線方式 (接 Mega2560 的 Serial1)：
// - ZS-040 TXD ➔ 接 Mega Pin 19 (RX1)
// - ZS-040 RXD ➔ 接 Mega Pin 18 (TX1)
// - ZS-040 VCC ➔ 接 5V
// - ZS-040 GND ➔ 接 GND
// ==========================================

void setup() {
  // 1. 開啟電腦 USB 序列埠 (開 Arduino IDE 的 Serial Monitor，設定 115200 鮑率 + Both NL & CR)
  Serial.begin(115200);

  // 2. AT 模式固定鮑率為 38400
  Serial1.begin(38400);

  delay(1000);
  Serial.println(F("\n=============================================="));
  Serial.println(F("  HC-05 / ZS-040 AT Mode Auto-Query Tool     "));
  Serial.println(F("=============================================="));
  Serial.println(F("提示：請確認 Arduino IDE 序列埠監控視窗右下角設為："));
  Serial.println(F(" 👉 [Both NL & CR ( NL 和 CR )] 與 [115200 baud]\n"));

  // 自動測試並查詢核心參數
  sendATCommand("AT");
  sendATCommand("AT+NAME?");
  sendATCommand("AT+PSWD?");
  sendATCommand("AT+UART?");
  sendATCommand("AT+ROLE?");
  sendATCommand("AT+ADDR?");
}

void sendATCommand(const char* cmd) {
  Serial.print(F("[發送 AT 指令]: "));
  Serial.println(cmd);
  Serial1.println(cmd);
  
  unsigned long start = millis();
  while (millis() - start < 1000) {
    while (Serial1.available() > 0) {
      char c = Serial1.read();
      Serial.write(c);
    }
  }
  Serial.println(F("\n----------------------------------------------"));
}

void loop() {
  // 將電腦 Serial Monitor 輸入的 AT 指令轉發給 ZS-040
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd.length() > 0) {
      Serial.print(F("[手動發送]: "));
      Serial.println(cmd);
      Serial1.println(cmd);
    }
  }

  // 將 ZS-040 回傳的 AT 結果印在電腦 Serial Monitor
  if (Serial1.available()) {
    while (Serial1.available()) {
      char c = Serial1.read();
      Serial.write(c);
    }
  }
}
