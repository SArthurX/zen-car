// ==========================================
// Arduino Mega 2560 專用藍芽硬體 Serial 測試程式
// ==========================================
// 硬體優勢說明：
// Arduino Mega 2560 擁有 4 組獨立的【硬體 Serial串口】！
// - Serial  (D0 RX0, D1 TX0): L298P 內建插槽 / USB 電腦傳輸
// - Serial1 (D19 RX1, D18 TX1): Mega 獨立藍芽硬體串口 (最推薦！)
//
// 最佳接線方式 (使用 Serial1，完全免用 SoftwareSerial)：
// - ZS-040 TXD ➔ 接 Mega Pin 19 (RX1)
// - ZS-040 RXD ➔ 接 Mega Pin 18 (TX1)
// - ZS-040 VCC ➔ 接 5V
// - ZS-040 GND ➔ 接 GND
// ==========================================

void setup() {
  // 1. USB 電腦監控視窗 (Serial0: 115200 鮑率)
  Serial.begin(115200);

  // 2. Mega 獨立藍芽硬體串口1 (Serial1: Pin 19/18, 9600 鮑率)
  Serial1.begin(9600);

  pinMode(LED_BUILTIN, OUTPUT);

  // 開機閃爍 5 下，代表 Mega2560 核心運作中
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
  }

  Serial.println(F("\n=============================================="));
  Serial.println(F("  Arduino Mega 2560 Hardware Serial1 Ready!   "));
  Serial.println(F("=============================================="));
}

void loop() {
  // A. 讀取從 Mega 獨立藍芽 Serial1 (Pin 19/18) 傳進來的資料
  while (Serial1.available() > 0) {
    char c = Serial1.read();
    if (c == '\r' || c == '\n') continue;

    // 印在 USB Serial Monitor (115200) 觀察
    Serial.print(F("[Mega Serial1 收到藍芽指令]: '"));
    Serial.print(c);
    Serial.println(F("'"));

    // 回傳給藍芽 (Mac 或 手機)
    Serial1.print(F("SUCCESS! Received: '"));
    Serial1.print(c);
    Serial1.println(F("'"));

    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
  }

  // B. 同步支援 L298P 內建藍芽槽 Serial0 (D0/D1)
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '\r' || c == '\n') continue;

    Serial.print(F("SUCCESS! Received via Serial0: '"));
    Serial.print(c);
    Serial.println(F("'"));

    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
  }
}
