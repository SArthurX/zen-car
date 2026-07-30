// ==========================================
// 藍芽與 Serial 指令處理模組
// ==========================================
// 支援硬體通道：
// 1. Serial    (USB 電腦 115200 鮑率 / L298P 內建槽)
// 2. BT_SERIAL (Serial2: Pin 17 Pin 16 ZS-040 藍芽模組)
// ==========================================
#include <Arduino.h>
#include "config.h"

extern float Kp, Ki, Kd, target_angle;
extern float integral;
extern bool robot_running;
extern unsigned long last_loop_time;
void imu_update();
void set_motor_speed(int m1_speed, int m2_speed);

void print_both(const String& msg) {
  Serial.print(msg);
  BT_SERIAL.print(msg);
}

void print_both_ln(const String& msg) {
  Serial.println(msg);
  BT_SERIAL.println(msg);
}

void print_serial_menu() {
  String menu = F("\n=== Bluetooth & Serial Commands ===\n"
                  "[0] Emergency Stop  [1] Start\n"
                  "[p/o] Kp +/- 2.0  (或輸入 P35.0 設定)\n"
                  "[d/c] Kd +/- 0.1  (或輸入 D1.2 設定)\n"
                  "[i/u] Ki +/- 0.1  (或輸入 I0.5 設定)\n"
                  "[t/g] Angle +/- 0.2° (或輸入 T23.0 設定)\n"
                  "[?] 查詢目前所有參數與狀態\n"
                  "===================================\n");
  print_both(menu);
}

void parse_single_char_command(char cmd) {
  switch (cmd) {
    case '0': // emergency stop
      robot_running = false;
      set_motor_speed(0, 0);
      print_both_ln(F(">>> [BT] EMERGENCY STOPPED! <<<"));
      break;

    case '1': // Start
      integral = 0.0;
      imu_update();
      last_loop_time = millis();
      robot_running = true;
      print_both_ln(F(">>> [BT] MOTOR RUNNING <<<"));
      break;

    case 'p': Kp += 2.0; print_both(F("[BT] Kp=")); print_both_ln(String(Kp, 1)); break;
    case 'o': Kp -= 2.0; print_both(F("[BT] Kp=")); print_both_ln(String(Kp, 1)); break;
    case 'i': Ki += 0.1; print_both(F("[BT] Ki=")); print_both_ln(String(Ki, 2)); break;
    case 'u': Ki -= 0.1; print_both(F("[BT] Ki=")); print_both_ln(String(Ki, 2)); break;
    case 'd': Kd += 0.1; print_both(F("[BT] Kd=")); print_both_ln(String(Kd, 2)); break;
    case 'c': Kd -= 0.1; print_both(F("[BT] Kd=")); print_both_ln(String(Kd, 2)); break;
    case 't': target_angle += 0.2; print_both(F("[BT] Target=")); print_both_ln(String(target_angle, 2)); break;
    case 'g': target_angle -= 0.2; print_both(F("[BT] Target=")); print_both_ln(String(target_angle, 2)); break;

    case '?':
      print_both_ln(F("--- [BT] Current Params ---"));
      print_both(F("Kp=")); print_both(String(Kp, 1));
      print_both(F(" | Ki=")); print_both(String(Ki, 2));
      print_both(F(" | Kd=")); print_both_ln(String(Kd, 2));
      print_both(F("Target Angle=")); print_both_ln(String(target_angle, 2));
      print_both(F("Status=")); print_both_ln(robot_running ? F("RUNNING") : F("STOPPED"));
      print_both_ln(F("---------------------------"));
      break;

    case 'h':
      print_serial_menu();
      break;
  }
}

void process_incoming_cmd_string(String str) {
  str.trim();
  if (str.length() == 0) return;

  if (str == "0" || str.equalsIgnoreCase("STOP")) {
    parse_single_char_command('0');
    return;
  }
  if (str == "1" || str.equalsIgnoreCase("START") || str.equalsIgnoreCase("RUN")) {
    parse_single_char_command('1');
    return;
  }

  char head = str.charAt(0);
  if ((head == 'P' || head == 'p') && str.length() > 1) {
    float val = str.substring(1).toFloat();
    if (val >= 0) {
      Kp = val;
      print_both(F("[BT] Kp set to ")); print_both_ln(String(Kp, 1));
    }
  }
  else if ((head == 'D' || head == 'd') && str.length() > 1) {
    float val = str.substring(1).toFloat();
    if (val >= 0) {
      Kd = val;
      print_both(F("[BT] Kd set to ")); print_both_ln(String(Kd, 2));
    }
  }
  else if ((head == 'I' || head == 'i') && str.length() > 1) {
    float val = str.substring(1).toFloat();
    if (val >= 0) {
      Ki = val;
      print_both(F("[BT] Ki set to ")); print_both_ln(String(Ki, 2));
    }
  }
  else if ((head == 'T' || head == 't') && str.length() > 1) {
    float val = str.substring(1).toFloat();
    target_angle = val;
    print_both(F("[BT] Target Angle set to ")); print_both_ln(String(target_angle, 2));
  }
  else if (str.length() == 1) {
    parse_single_char_command(head);
  }
}

static String rx_buf_usb = "";
static String rx_buf_bt  = "";

void check_serial_command() {
  // USB Serial 
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (rx_buf_usb.length() > 0) {
        process_incoming_cmd_string(rx_buf_usb);
        rx_buf_usb = "";
      }
    } else {
      rx_buf_usb += c;
    }
  }

  // Bluetooth Serial2
  while (BT_SERIAL.available() > 0) {
    char c = BT_SERIAL.read();
    if (c == '\n' || c == '\r') {
      if (rx_buf_bt.length() > 0) {
        process_incoming_cmd_string(rx_buf_bt);
        rx_buf_bt = "";
      }
    } else {
      rx_buf_bt += c;
    }
  }
}
