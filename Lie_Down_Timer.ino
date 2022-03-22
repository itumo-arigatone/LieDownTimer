#include <Arduino.h>
#include <Wire.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width,  in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// declare an SSD1306 display object connected to I2C
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup()
{
	Serial.begin(115200);
    // initialize OLED display with address 0x3C for 128x64
    if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed"));
        while (true);
    }
	Wire.begin();
    Wire.beginTransmission(0x68); //アドレス0x68指定でMPU-6050を選択、送信処理開始
    Wire.write(0x6B);             //MPU6050_PWR_MGMT_1レジスタのアドレス指定
    Wire.write(0x00);             //0を書き込むことでセンサ動作開始
    Wire.endTransmission();       //スレーブデバイスに対する送信を完了

}
void loop()
{
    // スタートアドレスの設定とデータの要求処理
    Wire.beginTransmission(0x68);     //アドレス0x68指定でMPU-6050を選択、送信処理開始
    Wire.write(0x3B);                 //ACCEL_XOUT_Hレジスタのアドレス指定
    Wire.endTransmission(false);      //false設定で接続維持
    Wire.requestFrom(0x68, 14, true); //MPU-6050に対して14byte分データを要求、I2Cバス開放
    int16_t ax, ay, az, gx, gy, gz, Temp; //符号付き整数を格納する16ビットの変数型の設定

    //シフト演算と論理和で16bitのデータを変数に格納
    //ax～gzまで、16bit(2byte) × 7 = 14byte
    ax = Wire.read() << 8 | Wire.read();    //x軸の加速度の読み取り 16bit
    ay = Wire.read() << 8 | Wire.read();    //y軸の加速度の読み取り 16bit
    az = Wire.read() << 8 | Wire.read();    //z軸の加速度の読み取り 16bit
    Temp  = Wire.read() << 8 | Wire.read(); //温度の読み取り 16bit
    gx = Wire.read() << 8 | Wire.read();    //x軸の角速度の読み取り 16bit
    gy = Wire.read() << 8 | Wire.read();    //y軸の角速度の読み取り 16bit
    gz = Wire.read() << 8 | Wire.read();    //z軸の角速度の読み取り 16bit

    //データの表示 1LSBの値はデータシートに記載
    //AFS_SEL設定 = 0, ±2g, 16384LSB/g
    //FS_SEL設定  = 0, ±250deg/s, 131LSB/deg/s 
    Serial.print(ax/16384.0); Serial.print(" g,  ");     //1LSBを加速度(G)に換算してシリアルモニタに表示
    Serial.print(ay/16384.0); Serial.print(" g,  ");     //1LSBを加速度(G)に換算してシリアルモニタに表示
    Serial.print(az/16384.0); Serial.print(" g,  ");     //1LSBを加速度(G)に換算してシリアルモニタに表示
    Serial.print(gx/131.0); Serial.print(" deg/s,  ");   //1LSBを角速度(deg/s)に換算してシリアルモニタに表示
    Serial.print(gy/131.0); Serial.print(" deg/s,  ");   //1LSBを角速度(deg/s)に換算してシリアルモニタに表示
    Serial.print(gz/131.0); Serial.println(" deg/s,  "); //1LSBを角速度(deg/s)に換算してシリアルモニタに表示

    // 2つめのモジュールの「OLED」と通信
    oled.clearDisplay(); // clear display
    oled.setTextSize(3);          // text size
    oled.setTextColor(WHITE);     // text color
    oled.setCursor(0, 10);        // position to display
    oled.println(ax/16384.0); // text to display
    oled.display();               // show on OLED
    delay(500);
}