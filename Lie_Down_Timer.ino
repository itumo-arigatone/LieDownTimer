#include <Arduino.h>
#include <Wire.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MsTimer2.h>

#define SCREEN_WIDTH 128 // OLED display width,  in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

//タイマー用
String numDisp;
int numCount;
bool startFlg = false;
bool timeUpFlg = false;
bool flashFlg = false;
bool clickFlg = false;

// タイマー用LED設定
int threeLED = 4;
int fiveLED = 5;
int tenLED = 6;
int fifteenLED = 7;
int twntyFiveLED = 8;
int thirtyLED = 9;
// beep用(tone()がtimer2と競合するらしいが、timer2をstopした後ならいける？かも？)
int beep = 10;

int leftSwitch = 13;
int rightSwitch = 12;

int timerMode = 0;
int timerModeBefore = 0;

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

    // LEDの初期設定
    pinMode(threeLED, OUTPUT);
    pinMode(fiveLED, OUTPUT);
    pinMode(tenLED, OUTPUT);
    pinMode(fifteenLED, OUTPUT);
    pinMode(twntyFiveLED, OUTPUT);
    pinMode(thirtyLED, OUTPUT);

    pinMode(beep, OUTPUT);

    pinMode(rightSwitch, INPUT_PULLUP); //９番ピンをHighにする（プルアップ）
    pinMode(leftSwitch, INPUT_PULLUP); //８番ピンをHighにする（プルアップ）
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

    // 傾きを確認
    // モジュールが水平(az=1[+-0.15])の場合はタイマーストップ&リセット
    if ((az/16384.0 < 1.15 && az/16384.0 > 0.85)) {
        if (startFlg) {
            startFlg = false;
            timeUpFlg = false;
            // stop timer
            Serial.println("stopTimer");
            MsTimer2::stop();
            // reset timer
            numCount = 0;
            // timerModeBefore = 0;
            // timerMode = 0;
            refreshLED();
        }


        if (digitalRead(leftSwitch) == LOW) {
            clickFlg = true;
            numCount += 60;
        }
        //離すのを待つ
        while(digitalRead(leftSwitch)==LOW){
            if(digitalRead(leftSwitch)==HIGH){
                break;
            }
        }

        if (digitalRead(rightSwitch) == LOW) {
            clickFlg = true;
            numCount += 1;
        }
        //離すのを待つ
        while(digitalRead(rightSwitch)==LOW){
            if(digitalRead(rightSwitch)==HIGH){
                break;
            }
        }
    }
    // モジュールが垂直(az=0[+-0.2])の場合はタイマースタート
    if (az/16384.0 > -0.2 && az/16384.0 < 0.2) {
        countDown(ax, ay);
    }

    String Display = createDisplayLayout(numCount);
    //「OLED」と通信
    oled.clearDisplay(); // clear display
    oled.setTextSize(4);          // text size
    oled.setTextColor(WHITE);     // text color
    oled.setCursor(0, 10);        // position to display
    oled.println(Display); // text to display
    oled.display();               // show on OLED
    
}

/**
 * @brief 
 * タイマー機能
 */
void timer()
{
    numCount -= 1;
}

void refreshLED() {
    digitalWrite(threeLED, LOW);
    digitalWrite(fiveLED, LOW);
    digitalWrite(tenLED, LOW);
    digitalWrite(fifteenLED, LOW);
    digitalWrite(twntyFiveLED, LOW);
    digitalWrite(thirtyLED, LOW);
    digitalWrite(beep, LOW);
}

void flashLED() {
    if (flashFlg) {        
        digitalWrite(threeLED, LOW);
        digitalWrite(fiveLED, LOW);
        digitalWrite(tenLED, LOW);
        digitalWrite(fifteenLED, LOW);
        digitalWrite(twntyFiveLED, LOW);
        digitalWrite(thirtyLED, LOW);
        digitalWrite(beep, LOW);
        flashFlg = false;
    } else {
        digitalWrite(threeLED, HIGH);
        digitalWrite(fiveLED, HIGH);
        digitalWrite(tenLED, HIGH);
        digitalWrite(fifteenLED, HIGH);
        digitalWrite(twntyFiveLED, HIGH);
        digitalWrite(thirtyLED, HIGH);
        digitalWrite(beep, HIGH);
        flashFlg = true;
    }
}

String createDisplayLayout(int count) {
    if (count <= 0 && startFlg && !timeUpFlg) {
        MsTimer2::stop();
        timeUpFlg = true;
        MsTimer2::set(500, flashLED);
        MsTimer2::start();

        return "Time up";
    }
    if (timeUpFlg) {
        return "Time up";
    }
    // mm:ssにする
    String min = "00";
    String sec = "00";
    if (count > 59) {
        min = String(count / 60);
        sec = String(count % 60);
    } else {
        sec = String(count);
    }
    // 0埋め
    min = min.length() > 1 ? min : "0" + min;
    sec = sec.length() > 1 ? sec : "0" + sec;
    return min + ":" + sec;
}

void countDown(int ax, int ay) {
    int clickNum;
    bool SetTimeflg = false;
    if (clickFlg) {
        clickNum = numCount;
    }
    // x y の角度を確認。
    // 3 min x=-1 y=0
    if ((ax/16384.0 > -1.2 && ax/16384.0 < -0.8) &&
    (ay/16384.0 > -0.2 && ay/16384.0 < 0.2)) {
        timerMode = 3;
        if (timerMode != timerModeBefore) {
            refreshLED();
            SetTimeflg = true;
            digitalWrite(threeLED, HIGH);
            numCount = 180;
        }
    }
    // 5 min x=-0.5 y=0.5
    if ((ax/16384.0 > -0.7 && ax/16384.0 < -0.3) &&
    (ay/16384.0 > 0.3 && ay/16384.0 < 0.7)) {
        timerMode = 5;
        if (timerMode != timerModeBefore) {
            refreshLED();
            SetTimeflg = true;
            digitalWrite(fiveLED, HIGH);
            numCount = 300;
        }
    }
    // 10 min x=0.5 y=0.5
    if ((ax/16384.0 > 0.3 && ax/16384.0 < 0.7) &&
    (ay/16384.0 > 0.3 && ay/16384.0 < 0.7)) {
        timerMode = 10;
        if (timerMode != timerModeBefore) {
            refreshLED();
            SetTimeflg = true;
            digitalWrite(tenLED, HIGH);
            numCount = 600;
        }
    }
    // 15 min x=1 y=0
    if ((ax/16384.0 > 0.8 && ax/16384.0 < 1.2) &&
    (ay/16384.0 > -0.2 && ay/16384.0 < 0.2)) {
        timerMode = 15;
        if (timerMode != timerModeBefore) {
            refreshLED();
            SetTimeflg = true;
            digitalWrite(fifteenLED, HIGH);
            numCount = 900;
        }
    }
    // 25 min x=0.5 y=-0.5
    if ((ax/16384.0 > 0.3 && ax/16384.0 < 0.7) &&
    (ay/16384.0 > -0.7 && ay/16384.0 < -0.3)) {
        timerMode = 25;
        if (timerMode != timerModeBefore) {
            refreshLED();
            SetTimeflg = true;
            digitalWrite(twntyFiveLED, HIGH);
            numCount = 1500;
        }
    }
    // 30 min x=-0.5 y=-0.5
    if ((ax/16384.0 > -0.7 && ax/16384.0 < -0.3) &&
    (ay/16384.0 > -0.7 && ay/16384.0 < -0.3)) {
        timerMode = 30;
        if (timerMode != timerModeBefore) {
            refreshLED();
            SetTimeflg = true;
            digitalWrite(thirtyLED, HIGH);
            numCount = 1800;
        }
    }
    if (clickFlg && SetTimeflg) {
        Serial.println("inclick");
        timerModeBefore = timerMode;
        clickFlg = false;
        numCount = clickNum;
        timeUpFlg = false;
        startFlg = true;
        MsTimer2::stop();
        MsTimer2::set(1000, timer);
        MsTimer2::start();
        return;
    }
    if (timerMode != timerModeBefore) {
        Serial.println("changemode");
        timerModeBefore = timerMode;
        timeUpFlg = false;
        startFlg = true;
        MsTimer2::stop();
        MsTimer2::set(1000, timer);
        MsTimer2::start();
        return;
    }
}