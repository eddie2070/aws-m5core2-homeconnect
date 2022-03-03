//**********************************************************************************************************
//*    audioI2S-- I2S audiodecoder for M5Stack Core2                                                       *
//**********************************************************************************************************
//
// first release on May.12/2021
//
//
// THE SOFTWARE IS PROVIDED "AS IS" FOR PRIVATE USE ONLY, IT IS NOT FOR COMMERCIAL USE IN WHOLE OR PART OR CONCEPT.
// FOR PERSONAL USE IT IS SUPPLIED WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHOR
// OR COPYRIGHT HOLDER BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE
//

#include <M5Core2.h>
#include "Audio.h"

#include <Wire.h>
#define SHT30  0x44
#include "UNIT_ENV.h"
#include <MQTTClient.h>
#include <WiFiClientSecure.h>
#include "WiFi.h"
#include <ArduinoJson.h>
#include "secrets.h"
#include "esp32-hal-ledc.h"
#include <FastLED.h>

#include <driver/i2s.h>

#define NUM_LEDS 10

#define DATA_PIN 25

CRGB leds[NUM_LEDS];
float pitch = 0.0F;
float roll  = 0.0F;
float yaw   = 0.0F;


#define COUNT_LOW 7640 //1500
#define COUNT_HIGH 7666 //8500
#define TIMER_WIDTH 16


// Digital I/O used
#define SD_CS          4
#define SD_MOSI       23
#define SD_MISO       38
#define SD_SCK        18
#define I2S_DOUT       2
#define I2S_BCLK      12
#define I2S_LRC        0

Audio audio;
//String ssid =     "Maison";
//String password = "coucoulesamis";

// The MQTT topics that this device should publish/subscribe
#define AWS_IOT_PUBLISH_TOPIC   "esp32/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "sensor/wyeth/#"
//#define AWS_IOT_SUBSCRIBE_TOPIC "sensor/wyeth/temp1"

WiFiClientSecure net = WiFiClientSecure();
MQTTClient client = MQTTClient(256);

QMP6988 qmp6988;

float temperature = 0.0F;

//word distance = 0;
int hotTemp = 0;
int mildTemp = 0;
int coldTemp = 0;

int last_value1 = 1,last_value2 = 1;
int cur_value1 = 1,cur_value2 = 1;


RTC_DateTypeDef DateStruct;
RTC_TimeTypeDef TimeStruct;

unsigned int data[6];

int sleeping = 0;

void messageHandler(String &topic, String &payload) {
  M5.Rtc.GetTime(&TimeStruct);
  Serial.println("incoming:" + topic + " - " + payload);
  Serial.println("incoming: " + payload);

if (topic == "sensor/wyeth/temp1"){
  StaticJsonDocument<200> filter;
  filter["temperature"] = true;

  //StaticJsonDocument<400> doc;
  //deserializeJson(doc, payload, DeserializationOption::Filter(filter));
  //const char* message = doc["message"];
  //Serial.println(payload);
  //serializeJsonPretty(doc, Serial);

  StaticJsonDocument<100> doc;
  deserializeJson(doc, payload, DeserializationOption::Filter(filter));
  float temperature = doc["temperature"];
  Serial.println(temperature);


  M5.Lcd.setTextSize(1); 
  M5.Lcd.setCursor(200, 12);
  M5.Lcd.printf("temp: %.2f at %02d:%02d",temperature,TimeStruct.Hours, TimeStruct.Minutes);
} else if ( topic == "sensor/wyeth/temp1/ext" ) {
  StaticJsonDocument<200> filter;
  filter["dark_temperature"] = true;

  //StaticJsonDocument<400> doc;
  //deserializeJson(doc, payload, DeserializationOption::Filter(filter));
  //const char* message = doc["message"];
  //Serial.println(payload);
  //serializeJsonPretty(doc, Serial);

  StaticJsonDocument<100> doc;
  deserializeJson(doc, payload, DeserializationOption::Filter(filter));
  float dark_temperature = doc["dark_temperature"];
  Serial.println(dark_temperature);


  M5.Lcd.setTextSize(1); 
  M5.Lcd.setCursor(200, 24);
  M5.Lcd.printf("temp ext: %.2f",dark_temperature);
}
  

  sleeping = 1;
  /*
  Serial.println("Going to light sleep for 4minutes 30 seconds.");
  delay(10000);  //delay 2500ms.
  M5.Axp.LightSleep(SLEEP_SEC(260));  //Wake up after 5 seconds of light sleep, the CPU will reboot and the program will start from the beginning.  轻度睡眠10秒后重新启动，程序从下一行继续执行
  Serial.println("Waking up from light sleep.");
*/
//  StaticJsonDocument<200> doc;
//  deserializeJson(doc, payload);
//  const char* message = doc["message"];
}

void connectAWS()
{
  Serial.println("Starting WiFi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(WiFi.status());
    Serial.print(".");
  }
  Serial.print("\n");
  Serial.print(WiFi.localIP());  
  Serial.print("\n");

  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.begin(AWS_IOT_ENDPOINT, 8883, net);

  // Create a message handler
  client.onMessage(messageHandler);

  Serial.print("Connecting to AWS IOT\n");

  while (!client.connect(THINGNAME)) {
    Serial.print(".");
    delay(100);
  }

  if(!client.connected()){
    Serial.println("AWS IoT Timeout!");
    return;
  }

  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);

  Serial.println("AWS IoT Connected!\n");
}

void publishMessage()
{
  StaticJsonDocument<200> doc;
  doc["time"] = millis();
  doc["sensor_a0"] = analogRead(0);
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client

  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}

void printDegree()
{

  M5.Lcd.setTextSize(3);  
  
  //M5.Lcd.print("o");
  M5.Lcd.drawString("o", (int)(M5.Lcd.width()/2+45), (int)(M5.Lcd.height()/2-30), 1);
  M5.Lcd.setTextSize(7);    
  M5.Lcd.setCursor(M5.Lcd.getCursorX()+25,M5.Lcd.getCursorY());   
}


void setup() {
   //connectAWS();
    M5.begin(true, true, true, true);
    pinMode(36, INPUT); // Button 2 set pin mode to input.设置引脚模式为输入模式
    pinMode(26, INPUT);
    qmp6988.init();

    M5.IMU.Init();

    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed
    FastLED.setBrightness(20);

  //Set Date
    //M5.Rtc.begin();  //Initialize the RTC clock.
    DateStruct.WeekDay = 5;
    DateStruct.Month = 2;
    DateStruct.Date = 4;
    DateStruct.Year = 2022;
    //M5.Rtc.SetDate(&DateStruct);
  //Set Time
    TimeStruct.Hours   = 10;
    TimeStruct.Minutes = 20;
    TimeStruct.Seconds = 00;
    //M5.Rtc.SetTime(&TimeStruct);

    M5.Rtc.GetDate(&DateStruct);    //Get the date of the real-time clock. 
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.printf("Date: %04d-%02d-%02d\n",DateStruct.Year, DateStruct.Month,DateStruct.Date);    //Output the date of the current real-time clock on the screen. 
    //M5.Lcd.printf("Week: %d/n",DateStruct.WeekDay);
    M5.Lcd.setCursor(200, 0);
    M5.Lcd.print("Wyeth - Living Room");
    M5.Lcd.setTextSize(7);
    //M5.Lcd.print("67");
    //M5.Lcd.drawString("67", (int)(M5.Lcd.width()/2)-85, (int)(M5.Lcd.height()/2-50), 2);
    printDegree();
    M5.Lcd.drawString("F", (int)(M5.Lcd.width()/2+75), (int)(M5.Lcd.height()/2-20), 1);
  //M5.Lcd.print("F");
  connectAWS();

  ledcSetup(1, 50, TIMER_WIDTH);

  M5.Axp.SetSpkEnable(true);
  //M5.Lcd.fillScreen(BLACK);
  //M5.Lcd.setTextColor(WHITE);
  //M5.Lcd.setTextSize(2);
    
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  SPI.begin(SD_SCK, SD_MISO, SD_MOSI);
  SPI.setFrequency(1000000);
  SD.begin(SD_CS);
  Serial.print(SD.cardSize());


  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(21); // 0...21
  
  //WiFi.mode(WIFI_STA);
  //WiFi.begin(ssid.c_str(), password.c_str());
  //while (!WiFi.isConnected()) {
  //  delay(10);
  //  Serial.print(WiFi.status());
  //  Serial.print(".");
  //}
  Serial.print("\n");
  Serial.print(WiFi.localIP());  
  Serial.print("\n");
  ESP_LOGI(TAG, "Connected");
  ESP_LOGI(TAG, "Starting MP3...\n");

//  audio.connecttoFS(SD, "/320k_test.mp3");
  //audio.connecttoFS(SD, "test.wav");
  //audio.connecttohost("http://air.ofr.fm:8008/jazz/mp3/128");
  //audio.connecttohost("https://wmbr.org/WMBR_live_128.m3u");
  //audio.connecttohost("https://20180302pdfparse.s3.amazonaws.com/Bruno.wav");
//  audio.connecttospeech("Миска вареників з картоплею та шкварками, змащених салом!", "uk-UA");
//audio.connecttospeech("Hello Theodore!", "en-US");
audio.connecttoFS(SD, "Bruno.wav");
}

int scani2caddr()
{
    for (int i = 0; i < 120; i++)
    {
        Wire.beginTransmission(i);
        if (Wire.endTransmission() == 0)
        {
            Serial.printf("%02X   |FIND", i);
            Serial.println(".");
        }
    }
    return 0;
}

void loop() {
    //int counter = 0;
  //scani2caddr();
  //Serial.printf("sleeping value: %d", sleeping);
  // while (sleeping == 1) {
  //   Serial.println("Going to light sleep for 1minute  30 seconds.");
  //   FastLED.clear(true);
  //   delay(60000);  //delay 2500ms.
  //   M5.Axp.DeepSleep(SLEEP_SEC(150));  //Wake up after 5 seconds of light sleep, the CPU will reboot and the program will start from the beginning.  轻度睡眠10秒后重新启动，程序从下一行继续执行
  //   Serial.println("Waking up from light sleep.");
  //   sleeping = 0;
  //   Serial.printf("sleeping value: %d\n", sleeping);
  //   break;
  // }
  Wire.begin();
  //Wire.beginTransmission(SHT30);
  //Wire.endTransmission(false);

 // Start I2C Transmission
  Wire.beginTransmission(0x44);
  // Send 16-bit command byte
  Wire.write(0x2C);
  Wire.write(0x06);
  // Stop I2C transmission
  Wire.endTransmission();
//  delay(100);

M5.IMU.getTempData(&temperature);
//Serial.printf("MTDU temperature: %f\n", temperature);

  int temp = 0;
  int humid = 0;

  Wire.requestFrom(0x44, 6);

  temp = Wire.read() << 8 | Wire.read();
  int tempcrc = Wire.read();
  humid = Wire.read() << 8 | Wire.read();
  //Serial.printf("humid: %d", humid);


  // Convert the data
  //int temp = (data[0] * 256) + data[1];
  float cTemp = -45.0 + (175.0 * temp / 65535.0);
  float fTemp = -49 + (315.0 * temp / 65535.0);

  float pHumid = 100.0 * humid / 65535.0;
  //Serial.printf("phumid: %.1f", pHumid);

  if (fTemp <= 69 && coldTemp == 0) {
    //Serial.println("Blue");
    //fill_solid(leds, 10, CRGB::Blue);
        for(int i = 0; i<5 ; i++){
                leds[i] = CRGB::Blue;
                leds[9-i] = CRGB::Blue;
                FastLED.show();
                delay(400);
            }
        hotTemp = 0;
        mildTemp = 0;
        coldTemp = 1;
  }
  else if (fTemp > 69.2 && fTemp <= 72.8 && mildTemp == 0 ) {
    //Serial.println("Black");
                FastLED.clear(true);
                fill_solid(leds, 10, CRGB::Black);
                hotTemp = 0;
                mildTemp = 1;
                coldTemp = 0;
  }
  else if (fTemp > 73 && hotTemp==0) {
    //Serial.println("Red");
    for(int i = 0; i<5 ; i++){
                leds[i] = CRGB::Red;
                leds[9-i] = CRGB::Red;
                FastLED.show();
                delay(400);
            }
    hotTemp = 1;
    mildTemp = 0;
    coldTemp = 0;
  }

  M5.Lcd.setTextSize(2); 
  M5.Lcd.setCursor(0, 210);
  M5.Lcd.printf("temp: %.1f | humid: %.1f%%",cTemp, pHumid);

  M5.Lcd.setTextSize(10);
  char mystring[10]; // Space for 9 characters + null termination
  sprintf(mystring, "%.1f", fTemp);
  M5.Lcd.drawString(mystring, (int)(M5.Lcd.width()/2)-125, (int)(M5.Lcd.height()/2-20), 1);

  M5.Lcd.setTextSize(1); 
  M5.Rtc.GetTime(&TimeStruct);
  if (TimeStruct.Minutes==00 && TimeStruct.Seconds==00) {
    Serial.println("It is o'clock");
    //ESP.restart();
    //connectAWS();
  }
  M5.Lcd.setCursor(0, 11);
  M5.Lcd.printf("Time: %02d:%02d:%02d",TimeStruct.Hours, TimeStruct.Minutes, TimeStruct.Seconds);
  float batVoltage = M5.Axp.GetBatVoltage();
  float batPercentage = ( batVoltage < 3.2 ) ? 0 : ( batVoltage - 3.2 ) * 100;
  int batperc = floor(batPercentage * 10.) / 10.;
  M5.Lcd.setCursor(0, 21);
  M5.Lcd.printf("Battery:        ");
  M5.Lcd.setCursor(0, 21);
  M5.Lcd.printf("Battery: %d",batperc);
  client.loop();

  //Checking 2 buttons device
  cur_value1 = digitalRead(36); // read the value of BUTTON.  // Blue Button Pin setting
  cur_value2 = digitalRead(26); // Red Button Pin setting
  M5.update(); //Read the press state of the key.  读取按键 A, B, C 的状态
  if (M5.BtnA.wasReleased() || cur_value1 != last_value1) {
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(200, 36);
    if (cur_value1==0){
      Serial.printf("button1 pressed : %d\n", cur_value1);
      M5.Lcd.print("Start servo");
      ledcAttachPin(26, 1);
      pinMode(26, OUTPUT);
      digitalWrite(26,1);
      for(int i = COUNT_LOW; i < COUNT_HIGH; i = i + 1){  
        ledcWrite(1, i);
        delay(100);
      }
      ledcDetachPin(26);
      M5.Lcd.setCursor(200, 36);
      M5.Lcd.print("               ");
      M5.Lcd.setCursor(200, 36);
      M5.Lcd.print("Stop servo");
      pinMode(26, INPUT);
    } else {
      Serial.printf("button1 released : %d\n", cur_value1);
    }
    last_value1 = cur_value1;
  } else if (M5.BtnB.wasReleased() || cur_value2 != last_value2 ) {
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(200, 36);
    if (cur_value2==0){
      Serial.printf("button2 pressed : %d\n", cur_value2);
      M5.Lcd.print("Start rev-servo");
      ledcAttachPin(26, 1);
      pinMode(26, OUTPUT);
      digitalWrite(26,1);
    for(int i = 3000; i < 3060; i = i + 1){  
      ledcWrite(1, i);
      delay(100);
    }
    ledcDetachPin(26);
    M5.Lcd.setCursor(200, 36);
    M5.Lcd.print("                ");
    M5.Lcd.setCursor(200, 36);
    M5.Lcd.print("Stop servo");
    pinMode(26, INPUT);
    } else {
      Serial.printf("button2 released : %d\n", cur_value2);
    }
    last_value2 = cur_value2;
  } else if (M5.BtnC.wasReleased()) { // || M5.BtnC.pressedFor(1000, 200)) {
    M5.Lcd.setCursor(200, 36);
    M5.Lcd.print("           ");
    M5.Lcd.setCursor(200, 36);
    M5.Lcd.print("Playing music");
    audio.setVolume(21); // 0...21
    int incr = 0;
    //audio.connecttospeech("Hello Theodore!", "en-US");
    //delay(200);
    while (incr<2100) {
      audio.loop();
      //audio.stopSong();
      //Serial.printf("loopAAAA\n");
      //Serial.printf("loopDDDD\n");
      Serial.printf("incr: %d\n", incr);
      incr++;
    }
    incr = 0;
    audio.stopSong();
    audio.reset();
    audio.connecttoFS(SD, "Bruno.wav");
    M5.Lcd.setCursor(200, 36);
    M5.Lcd.print("              ");
  } else if (M5.BtnB.wasReleasefor(700)) {
    //M5.Lcd.clear(WHITE);  // Clear the screen and set white to the background color.  清空屏幕并将白色设置为底色
    M5.Lcd.setCursor(0, 0);
  } 
}

// optional
void audio_info(const char *info){
    Serial.print("info        "); Serial.println(info);
}
void audio_id3data(const char *info){  //id3 metadata
    Serial.print("id3data     ");Serial.println(info);
}
void audio_eof_mp3(const char *info){  //end of file
    Serial.print("eof_mp3     ");Serial.println(info);
}
void audio_showstation(const char *info){
    Serial.print("station     ");Serial.println(info);
}
void audio_showstreamtitle(const char *info){
    Serial.print("streamtitle ");Serial.println(info);
}
void audio_bitrate(const char *info){
    Serial.print("bitrate     ");Serial.println(info);
}
void audio_commercial(const char *info){  //duration in sec
    Serial.print("commercial  ");Serial.println(info);
}
void audio_icyurl(const char *info){  //homepage
    Serial.print("icyurl      ");Serial.println(info);
}
void audio_lasthost(const char *info){  //stream URL played
    Serial.print("lasthost    ");Serial.println(info);
}
void audio_eof_speech(const char *info){
    Serial.print("eof_speech  ");Serial.println(info);
}