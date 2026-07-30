// ==========================================
// 馬達與編碼器檢測程式 (Motor & Encoder Test)
// ==========================================
// 目的：
// 1. 檢測左右輪最低啟動 PWM (死區 DEAD_ZONE)
// 2. 檢測左右輪轉速 (編碼器每秒 Pulse 數) 與正反轉方向
// 鮑率：115200
// ==========================================

#define M1_PWM_PIN 10
#define M1_DIR_PIN 12
#define M2_PWM_PIN 11
#define M2_DIR_PIN 13

#define M1_ENCODER_A 19
#define M1_ENCODER_B 18
#define M2_ENCODER_A 2
#define M2_ENCODER_B 3

volatile long m1_count = 0;
volatile long m2_count = 0;

int test_pwm = 60;  // 預設測試 PWM 輸出
bool motor_running = false;

void m1_isr() {
  if (digitalRead(M1_ENCODER_A) == digitalRead(M1_ENCODER_B)) m1_count--;
  else m1_count++;
}

void m2_isr() {
  if (digitalRead(M2_ENCODER_A) == digitalRead(M2_ENCODER_B)) m2_count--;
  else m2_count++;
}

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 2000) {}

  pinMode(M1_PWM_PIN, OUTPUT);
  pinMode(M1_DIR_PIN, OUTPUT);
  pinMode(M2_PWM_PIN, OUTPUT);
  pinMode(M2_DIR_PIN, OUTPUT);

  pinMode(M1_ENCODER_A, INPUT_PULLUP);
  pinMode(M1_ENCODER_B, INPUT_PULLUP);
  pinMode(M2_ENCODER_A, INPUT_PULLUP);
  pinMode(M2_ENCODER_B, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(M1_ENCODER_A), m1_isr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(M2_ENCODER_A), m2_isr, CHANGE);

  Serial.println(F("\n================================="));
  Serial.println(F("  Motor & Encoder Speed Tester   "));
  Serial.println(F("================================="));
  Serial.println(F("[1] 正轉測試  [2] 反轉測試  [0] 停止"));
  Serial.println(F("[+] PWM +10   [-] PWM -10"));
  Serial.println(F("[u] PWM +1    [d] PWM -1 (測最低啟動點)"));
  Serial.println(F("=================================\n"));
}

unsigned long last_print = 0;
long last_m1 = 0, last_m2 = 0;

void loop() {
  // Serial 控制指令
  if (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '1') {
      motor_running = true;
      digitalWrite(M1_DIR_PIN, HIGH);
      digitalWrite(M2_DIR_PIN, HIGH);
      Serial.println(F(">>> FORWARD >>>"));
    } else if (c == '2') {
      motor_running = true;
      digitalWrite(M1_DIR_PIN, LOW);
      digitalWrite(M2_DIR_PIN, LOW);
      Serial.println(F("<<< BACKWARD <<<"));
    } else if (c == '0') {
      motor_running = false;
      analogWrite(M1_PWM_PIN, 0);
      analogWrite(M2_PWM_PIN, 0);
      Serial.println(F(">>> STOP <<<"));
    } else if (c == '+') {
      test_pwm = constrain(test_pwm + 10, 0, 255);
      Serial.print(F("PWM = ")); Serial.println(test_pwm);
    } else if (c == '-') {
      test_pwm = constrain(test_pwm - 10, 0, 255);
      Serial.print(F("PWM = ")); Serial.println(test_pwm);
    } else if (c == 'u') {
      test_pwm = constrain(test_pwm + 1, 0, 255);
      Serial.print(F("PWM = ")); Serial.println(test_pwm);
    } else if (c == 'd') {
      test_pwm = constrain(test_pwm - 1, 0, 255);
      Serial.print(F("PWM = ")); Serial.println(test_pwm);
    }
  }

  // 馬達輸出
  if (motor_running) {
    analogWrite(M1_PWM_PIN, test_pwm);
    analogWrite(M2_PWM_PIN, test_pwm);
  } else {
    analogWrite(M1_PWM_PIN, 0);
    analogWrite(M2_PWM_PIN, 0);
  }

  // 每 500ms 計算並印出速度 (脈衝數/0.5s)
  if (millis() - last_print >= 500) {
    last_print = millis();
    long cur_m1 = m1_count;
    long cur_m2 = m2_count;
    long speed_m1 = cur_m1 - last_m1;
    long speed_m2 = cur_m2 - last_m2;
    last_m1 = cur_m1;
    last_m2 = cur_m2;

    if (motor_running) {
      Serial.print(F("PWM: "));
      Serial.print(test_pwm);
      Serial.print(F(" | Left Speed: "));
      Serial.print(speed_m1);
      Serial.print(F(" | Right Speed: "));
      Serial.print(speed_m2);
      Serial.print(F(" | Diff: "));
      Serial.println(abs(speed_m1 - speed_m2));
    }
  }
}
