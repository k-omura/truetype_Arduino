//系统休眠
void esp_sleep(uint32_t minutes)
{
  digitalWrite(12, 0); //关闭电池测量
  pinMode(12, INPUT);  //改为输入状态避免漏电
  display.hibernate(); //关闭电源屏幕并进入深度睡眠 display.powerOff()为仅关闭电源
  pinMode(SD_POW, OUTPUT);
  digitalWrite(SD_POW, 0);
  ESP.deepSleep(minutes * 1000, WAKE_RF_DEFAULT); //WAKE_RF_DEFAULT  WAKE_RFCAL  WAKE_NO_RFCAL  WAKE_RF_DISABLED RF_DISABLED
}

void clearScreen() //清空屏幕
{
  display.init(0, 0, 10, 1);
  display.setFullWindow();
  display.firstPage();
  display.nextPage();
}
