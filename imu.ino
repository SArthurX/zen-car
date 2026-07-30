// ==========================================
// IMU 模組 - 使用 MPU6050_light 函式庫
// ==========================================
#include <Wire.h>
#include <MPU6050_light.h>
#include "config.h"

MPU6050 mpu(Wire);

extern float angleX;
extern float gyro_rateX_current;

void imu_init() {
  Wire.begin();
  Wire.setClock(400000);

  byte status = mpu.begin();
  Serial.print(F("MPU6050 Init: "));
  Serial.println(status == 0 ? F("OK") : F("FAIL"));

  mpu.setFilterGyroCoef(FILTER_GYRO);
}

void imu_calibrate() {
  Serial.println(F("Calibrating gyro... Please keep car still!"));
  // false = 不校正加速度計 (保留重力參考點)
  // true  = 校正陀螺儀零點漂移
  mpu.calcOffsets(false, true);
  Serial.println(F("Gyro Calibration Done!"));
}

void imu_update() {
  mpu.update();
  angleX = mpu.getAngleX();
  gyro_rateX_current = mpu.getGyroX();
}
