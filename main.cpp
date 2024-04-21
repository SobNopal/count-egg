#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include <SPI.h>
#include <Adafruit_I2CDevice.h>
#include <RTClib.h>

WiFiClientSecure ESPWifi;
PubSubClient client(ESPWifi);
RTC_DS3231 rtc;

const int pinSensor[12]={5,6,7,15,16,8,9,10,11,12,13,14};
const int pinBuzzer=21;
const char *mqttBroker =  "31.187.75.181"; // "31.187.75.181";
const char *mqttUsername = " ";
const char *mqttPass = " ";
const int mqttPort = 8883; //8883;
const char *mqttTopic = "jatinom/eggs";

unsigned long last, lastClientLoop;
int sen;
String lcdBuff="";
String endChar=String(char(0xff))+String(char(0xff))+String(char(0xff));
String wifiSSID="";
String wifiPassword="";
String param;
char* times;
char* timess;
char timesBuffer [80];
uint16_t year;
uint8_t month;
uint8_t day;
uint8_t hour;
uint8_t minute;
uint8_t second;
uint16_t sensorCount[12];
uint32_t totalCount, lastTotalCount=0;
uint16_t averageCount;
uint16_t offset=100;
uint16_t nChicken=100;
uint16_t nEggs=100;


String bufferEEPROM="";
String readEEPROM();
void writeEEPROM(String word);
void sendDisplay();
bool scanWiFi();
void connectToWiFi();
void syncClock();
void getRTCClock();
void inputTouch();
void reconnect();
void publish();
void countTotal();
void resetCounter();
void callbackMqtt(char* topic, byte* payload, unsigned int length);

const char client_private_key[] PROGMEM = R"EOF(
-----BEGIN PRIVATE KEY-----
MIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQCucxUWXJRhKjt2
JMANHJ6bqUmfycEqyPp/DCo8sgAWOliiIvbigg2xlfvAHww4LgWHYc5K+z/uyd48
EXx/S9MCOqOReNOP1WvfwKo9l4sQNTTVCiuPFpjbUH2bAL1cbC66aCt3Ifh+Zf0W
7xh0nZRh2Ssebf5vvRe+9awwQsnwo79gu/mGA/GqpE3u7lSocD1NQGT0wzLj5VaV
jCMvswf9qelvFBkO9KAKs1Oi5sPu9t5/BfqUb1Ou/canmJRcD40mvgacM/udPbQ1
RUZTvjeDXdoK4W9X+IPU+PhXyYZq6ORphd0P1amsNEDNE2SjIXhD+BiGs++1Dvau
VkSlxTTTAgMBAAECggEATvgTjKreFFoMzr92HLle5zIr/ORUyCxwkMCOAinFtko1
Qzg2sHSVBFXTv7WOT4Qtxoo022Z/G3ZQXrq/s9Q8Md8fmkOq6YqAKarKpdULA4xC
XUocS8q76VO3eEpObh8ezA1J18UN+xHqDs6vMGMA/4ZSaZT7P9PGgxAtIS+qwH5w
NKN1kdIw27rKL1wTHzCq/gPWiaTjROxzf9V5Ujn9/jtcMIA8KAJ0WWvj9b5QBpuo
cKpWcHio16lCAjNj7vjA/KK5YpP19YXL3rFYxsZ8WuTBgvniqfiHM4zMBwnF7BOo
iLlB+6R2UZF9Hk721dlSkMW4XpTkXdxjLeAb405hYQKBgQDbmRTGLfsT2WKQvIV4
a1XW6oZmKhTbEqzaYIcCtNO6ZuF48m6UYDqfVr/2LKBurQcsAFNEdNggdLQc8CLY
Nh3GkT7ZUVwCDXOS/kMQzbjenwGTq3U1Aekp+b7jx5ytzgH+At6hwaXJ9sFKrFbO
1pmbv7dqJhO1AQDEq4rJkWZKkQKBgQDLXhH1TFGAIbwnBepnsz8FNRAOrHvsgdgE
QorF9o8wP4FJ6EYb7ntaLedkqW82DixIJ9A9Rja05Qf9VmM1A1yUIoH2xV/Gdun3
waIuIs0RJ6hwy7kveS3GmYz5qusk9vEYJza6T0MFr213pxUpazo4vmkSpSoqiwsb
FMIYJtxTIwKBgF+yhmsW/qPXyCxq+39Ox0mxSoCbNhuCN/GyvyNeyiYhT4D1pVrj
1Kg7lcwiBog0vztdqOvcP5NlSFiUDZtLeg5enZT277G1Svhz9aYNANODV5ySy7Ed
9A5m19lL0+uqKxQXDA5R5X3uGq4ADJR20Noe4j9P+KaYmU0btI/C3WEBAoGALMsM
g44KyYEksyevaKXndJsqbUD6jq5OySlq7Y2QHl1uebvqbU6K5uMDe32CXFKk2EPE
rype9FsZ9mfntA6IfqxXGeaBYPJiOErzENxjdKrag/WrVVSIi5zYm0lVP7AQjHRe
roy0w3TPVLJ9i8DtcwAlnpC4h+RWTuG0mqM8licCgYEA0aXdiYKxfNieYOQIsI3o
RNMeWJAkNPGrXA2EhyO6b0zzZ80oC/c4pTcNoJm/s3oyVVXbwm8fmWdZEWfMGKMA
ZrtRet/ebmnLBVLfYAO/KDmc8v6atDsFm63pcSCMTlBa5aaDYafZw14NsjwIw5yZ
6K1hZgKj4Mz0EbPYGKIzMw0=
-----END PRIVATE KEY-----
)EOF";

const char client_cert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDZTCCAk0CFCokTyiDi5NrZA1pmLUhV3u3n8SmMA0GCSqGSIb3DQEBCwUAMG0x
CzAJBgNVBAYTAklEMRIwEAYDVQQIDAlFYXN0IEphdmExDzANBgNVBAcMBk1hbGFu
ZzEUMBIGA1UECgwLSmF0aW5vbXhVTU0xCzAJBgNVBAsMAkNBMRYwFAYDVQQDDA0z
MS4xODcuNzUuMTgxMB4XDTIyMTEwNjEwMDAyN1oXDTIzMTEwNjEwMDAyN1owcTEL
MAkGA1UEBhMCSUQxEjAQBgNVBAgMCUVhc3QgSmF2YTEPMA0GA1UEBwwGQmxpdGFy
MRQwEgYDVQQKDAtKYXRpbm9teFVNTTEPMA0GA1UECwwGQ2xpZW50MRYwFAYDVQQD
DA0zMS4xODcuNzUuMTgxMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA
rnMVFlyUYSo7diTADRyem6lJn8nBKsj6fwwqPLIAFjpYoiL24oINsZX7wB8MOC4F
h2HOSvs/7snePBF8f0vTAjqjkXjTj9Vr38CqPZeLEDU01QorjxaY21B9mwC9XGwu
umgrdyH4fmX9Fu8YdJ2UYdkrHm3+b70XvvWsMELJ8KO/YLv5hgPxqqRN7u5UqHA9
TUBk9MMy4+VWlYwjL7MH/anpbxQZDvSgCrNToubD7vbefwX6lG9Trv3Gp5iUXA+N
Jr4GnDP7nT20NUVGU743g13aCuFvV/iD1Pj4V8mGaujkaYXdD9WprDRAzRNkoyF4
Q/gYhrPvtQ72rlZEpcU00wIDAQABMA0GCSqGSIb3DQEBCwUAA4IBAQCd/PNkXUV7
U63s1LPVxJSbJbmqCxdH81P6hxeNc3ngy21tegRj+zmhlZkwmmXuKyEhf1s3/o/c
1YtBYy2Uz5uldTK77sdiHG53ZwMuoEYvxiG1f+erhLs1v+2ONvqA9xcFS9qGXYEi
PdjO0cvWQxSc60KWWESerXOjFiQSRIS5I2H8AO5ugSsoyGbnverxoKl2r5xDj3KJ
FbWHWERK20DlgLvtrZCEH6PLVtYsAYfRAi5Tln2DWiap5XXhwQM2+SYA0sID/U36
5RvlmwTYSX4Qyvzl5xYFNxuhFEmRzlyjlR7Nca/B0L2Jsrsu9YHFs66zYd9bdcF5
0PFGov2vXSd+
-----END CERTIFICATE-----
)EOF";

static const char ca_cert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDuzCCAqOgAwIBAgIUZRPyH/AusVtK7clIe57z7MBpEjIwDQYJKoZIhvcNAQEL
BQAwbTELMAkGA1UEBhMCSUQxEjAQBgNVBAgMCUVhc3QgSmF2YTEPMA0GA1UEBwwG
TWFsYW5nMRQwEgYDVQQKDAtKYXRpbm9teFVNTTELMAkGA1UECwwCQ0ExFjAUBgNV
BAMMDTMxLjE4Ny43NS4xODEwHhcNMjIxMTA2MTAwMDI2WhcNMjMxMTA2MTAwMDI2
WjBtMQswCQYDVQQGEwJJRDESMBAGA1UECAwJRWFzdCBKYXZhMQ8wDQYDVQQHDAZN
YWxhbmcxFDASBgNVBAoMC0phdGlub214VU1NMQswCQYDVQQLDAJDQTEWMBQGA1UE
AwwNMzEuMTg3Ljc1LjE4MTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEB
AMtLq9Ndyj/OD3rqVgS6M44olG6u8Uq6bziesgfYrRJEKbD2KmWP6fNVJr1J7XwK
ZMyPtJcw73MJuiXF0ghTocu6VeBxjBHjU531JlxM5cRANBTwTShn4eZg9nWs+5Xo
numfKf1itIbO+LLg28vdEIHkfagaMX598rMIfhe8HEGTd5pxLkI7uqV/8/f95VHy
aksrSFkhhgCP+dCjBL88lIHDEWNm+jyUDC6ZkFy89bXnwhEJaeHAR1t5jXxP6IZN
wFD5ZCwsudGckuwTW9FXgQylrk8KA83cFRevSssC0AiHoTlO9Rm4tn/mOpbJinxJ
y7sf6Eqe1mIFtBpwQr+9AZUCAwEAAaNTMFEwHQYDVR0OBBYEFFaEd8d8MgpJqLmM
u5qg277lYBGNMB8GA1UdIwQYMBaAFFaEd8d8MgpJqLmMu5qg277lYBGNMA8GA1Ud
EwEB/wQFMAMBAf8wDQYJKoZIhvcNAQELBQADggEBAMMEvXHgSssgUKgVHF0URdLG
kXAlI7+Iwh7mwRrX7uGfB7g+Oqez6HyNgVJN0JnAK8jqBSvh5Xr21srYViyvtRmW
uwTOSQoovhMCKT1bQ9azIk1gSt/cc0qh0TDSggEQ7Oq3qrszOxZA+jljWjZOMe+H
l1mL2L1m7FaVmndKlSITE4kqohM89OKe9TsThCtaGpcXlric/ExE0CU8Jmv/wng5
4G82JY0vX8ajrJbTH245fUJU67i36aWDmY5eWHhNSZAgTZBtSWXGF8/CyG1ibKvX
S2KhrSCLZRT6U+3d2Chb4ytKVUv4Ac7SEU23DUDvWTAR7RpkLv0k9/W3/O0VSCk=
-----END CERTIFICATE-----
)EOF";

void Count0(){sensorCount[0]+=1;}
void Count1(){sensorCount[1]+=1;}
void Count2(){sensorCount[2]+=1;}
void Count3(){sensorCount[3]+=1;}
void Count4(){sensorCount[4]+=1;}
void Count5(){sensorCount[5]+=1;}
void Count6(){sensorCount[6]+=1;}
void Count7(){sensorCount[7]+=1;}
void Count8(){sensorCount[8]+=1;}
void Count9(){sensorCount[9]+=1;}
void Count10(){sensorCount[10]+=1;}
void Count11(){sensorCount[11]+=1;}

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial1.begin(9600,134217756U, 18,17);
  EEPROM.begin(512);
  pinMode(pinBuzzer,OUTPUT);
  pinMode(LED_BUILTIN,OUTPUT);
  delay(100);
  bufferEEPROM = readEEPROM();
  Wire1.begin(36,35,400);
  delay(100);
  rtc.begin(&Wire1);
  
  if(bufferEEPROM!=""){
    String tempSSID, tempPass, tempOffset, tempNChicken,tempNEggs;
    char readChar;
    int i = 0;
    
    while(bufferEEPROM[i]!='#'){
      tempSSID+=bufferEEPROM[i];
      i++;
    }
    i+=1;
    while(bufferEEPROM[i]!='*'){
      tempPass+=bufferEEPROM[i];
      i++;
    }
    i+=1;
    while(bufferEEPROM[i]!='@'){
      tempOffset+=bufferEEPROM[i];
      i++;
    }
    i+=1;
    while(bufferEEPROM[i]!='$'){
      tempNChicken+=bufferEEPROM[i];
      i++;
    }
    i+=1;
    while(bufferEEPROM[i]!='%'){
      tempNEggs+=bufferEEPROM[i];
      i++;
    }
    wifiSSID=tempSSID;
    wifiPassword=tempPass;
    offset=tempOffset.toInt();
    nChicken=tempNChicken.toInt();
    nEggs=tempNEggs.toInt();
    delay(2000);
    Serial1.print("times.txt=\""+String("Offset :")+String(offset)+"\""+endChar);
    delay(2000);
    Serial1.print("times.txt=\""+String("nChicken :")+String(nChicken)+"\""+endChar);
    delay(2000);
    Serial1.print("times.txt=\""+String("nEggs :")+String(nEggs)+"\""+endChar);
    delay(2000);
    Serial1.print("times.txt=\""+String("Connect to: ")+String(wifiSSID)+"\""+endChar);
    delay(2000);
    connectToWiFi();
  }
  void (*(Count[12]))() = {Count0, Count1, Count2, Count3, Count4, Count5, Count6, Count7, Count8, Count9, Count10, Count11};
  for (uint8_t i=0; i<12; i++){
    pinMode(pinSensor[i], INPUT);
    attachInterrupt(digitalPinToInterrupt(pinSensor[i]), Count[i], RISING);
  }
  digitalWrite(pinBuzzer,HIGH);
  digitalWrite(LED_BUILTIN,LOW);
  if (WiFi.status()==WL_CONNECTED){
    Serial1.print("times.txt=\""+String("WiFi Connected")+"\""+endChar);
    ESPWifi.setCACert(ca_cert);
    ESPWifi.setCertificate(client_cert);
    ESPWifi.setPrivateKey(client_private_key);
    client.setServer(mqttBroker, mqttPort);
    client.setCallback(callbackMqtt);
    for(uint8_t i=0; i<5; i++){
      syncClock();
    }
  }
  digitalWrite(pinBuzzer,LOW);
}

void loop() {
  client.loop();
  if(Serial1.available()){
    inputTouch();
  }
  unsigned long now=millis();
  if ((now-lastClientLoop)>2000){
    lastClientLoop=now;
    getRTCClock();
    countTotal();
    sendDisplay();
    if(WiFi.status()==WL_CONNECTED){
      if (!client.connected()) {
        reconnect();
      }else{
        if (((minute%1)==0)&&(second>=2)&&(second<=3)){
          publish();
        }else if((hour==23)&&(minute==2)&&(second>=2)&&(second<=3)){
          resetCounter();
        }
      }
    }
  }
}

void sendDisplay(){
  if(WiFi.status()==WL_CONNECTED){
    Serial1.print("WiFiButton.picc=1"+endChar);
    Serial1.print("times.txt=\""+String(times)+"\""+endChar);
  }else{
    Serial1.print("WiFiButton.picc=0"+endChar);
    Serial1.print("times.txt=\""+String("WiFi Not Connected")+"\""+endChar);
  }
  if(client.connected()){
    Serial1.print("CloudStatus.picc=1"+endChar);
  }else{
    Serial1.print("CloudStatus.picc=0"+endChar);
  }
  for(uint8_t i=0; i<12; i++){
    Serial1.print("n"+String(i)+".val="+String(sensorCount[i])+endChar);
    if((sensorCount[i]-averageCount)<offset){
      Serial1.print("n"+String(i)+".picc=1"+endChar);
    }else{
      Serial1.print("n"+String(i)+".picc=0"+endChar);
    }
  }
  Serial1.print("n12.val="+String(averageCount)+endChar);
  Serial1.print("n13.val="+String(totalCount)+endChar);
}

void syncClock() {
  configTime(7*3600, 0 , "pool.ntp.org", "time.nis.gov");
  int iTime = 0;
  while (time(nullptr) < 1000000000ul && iTime<100) {
    Serial1.print("times.txt=\""+String("Sync Time")+"\""+endChar);
    delay(100);
    iTime++;
  }
  time_t tnow = time(nullptr);
  struct tm *timeinfo;
  timeinfo = localtime (&tnow);
  Serial.print(timeinfo);
  strftime(timesBuffer,80,"%Y-%m-%d %H:%M:%S",timeinfo);
  String NTPTimes=timesBuffer;
  timess=timesBuffer;
  year=NTPTimes.substring(0,4).toInt();
  month=NTPTimes.substring(5,7).toInt();
  day=NTPTimes.substring(8,10).toInt();
  hour=NTPTimes.substring(11,13).toInt();
  minute=NTPTimes.substring(14,17).toInt();
  second=NTPTimes.substring(18,20).toInt();
  rtc.adjust(DateTime(year,month,day,hour,minute,day));
  Serial1.print("times.txt=\""+String("Sync Time Done")+"\""+endChar);
}

void getRTCClock(){
  DateTime now=rtc.now();
  year=now.year();
  month=now.month();
  day=now.day();
  hour=now.hour();
  minute=now.minute();
  second=now.second();
  sprintf(timesBuffer,"%d-%d-%d %d:%d:%d",year,month,day,hour,minute,second);
  times=timesBuffer;
  //times=String(year)+"-"+String(month)+"-"+String(day)+" "+String(hour)+":"+String(minute)+":"+String(second);
}

bool scanWiFi(){
  wifiSSID="";wifiPassword="";
  Serial1.print("times.txt=\""+String("Searching WiFi")+"\""+endChar);
  uint8_t nSSID=WiFi.scanNetworks();
  if(Serial1.available()){
    lcdBuff=Serial1.readStringUntil('#');
    if(lcdBuff.indexOf("Back")>-1){
      lcdBuff="";
      return false;
    }
  }
  if(nSSID==0){
    Serial1.print("times.txt=\""+String("WiFi Not Found")+"\""+endChar);
  }else{
    for (uint8_t i; i< nSSID; i++){
      String SSIDs=WiFi.SSID(i);
      if(lcdBuff.indexOf("Back")>-1){
        lcdBuff="";
        break;
      }
      if(i<12){
        Serial1.print("times.txt=\""+String("Done")+"\""+endChar);
        Serial1.print("b"+String(i)+".txt=\""+SSIDs+"\""+endChar);
      }
    }
  }
  return true;
}

void connectToWiFi(){
  const char *ssid=wifiSSID.c_str();
  const char *pass=wifiPassword.c_str();
  bufferEEPROM=String(wifiSSID)+"#"+String(wifiPassword)+"*"+String(offset)+"@"+String(nChicken)+"$"+String(nEggs)+"%";
  writeEEPROM(bufferEEPROM);
  WiFi.begin(ssid,pass);
  uint8_t wifiCountReconnect=0;
  while (1){
    if(WiFi.status()==WL_CONNECTED){
      sendDisplay();
      Serial1.print("times.txt=\""+String("Connect to: ")+String(wifiSSID)+" "+String(wifiCountReconnect)+"\""+endChar);
      break;
    }else{
      delay(250);
      Serial1.print("times.txt=\""+String("Connect to: ")+String(wifiSSID)+" "+String(wifiCountReconnect)+"\""+endChar);
      delay(250);
      sendDisplay();
      wifiCountReconnect+=1;
      if(wifiCountReconnect==20){
        wifiCountReconnect=0;
        break;
      }
      Serial1.print("WiFiButton.picc=0"+endChar);
      if(Serial1.available()){
        lcdBuff=Serial1.readStringUntil('#');
        if(lcdBuff.indexOf("WiFi")>-1){
          lcdBuff="";
          digitalWrite(LED_BUILTIN,HIGH);
          scanWiFi();
          digitalWrite(LED_BUILTIN,LOW);
          break;
        }
      }
    }
  }
}

void inputTouch(){
  lcdBuff=Serial1.readStringUntil('#');
  if(lcdBuff.indexOf("WiFi")>-1){
    lcdBuff="";
    Serial1.print("times.txt=\""+String(times)+"\""+endChar);
    digitalWrite(LED_BUILTIN,HIGH);
    scanWiFi();
    digitalWrite(LED_BUILTIN,LOW);
  }else if(lcdBuff.indexOf("Back")>-1){
    lcdBuff="";
    digitalWrite(LED_BUILTIN,LOW);
  }else if(lcdBuff.indexOf("Back2t")>-1){
    lcdBuff="";
    Serial1.print("times.txt=\""+String(times)+"\""+endChar);
    digitalWrite(LED_BUILTIN,HIGH);
    scanWiFi();
    digitalWrite(LED_BUILTIN,LOW);
  }else  if(lcdBuff.indexOf("SSID1")>-1){
    lcdBuff="";
    wifiSSID=WiFi.SSID(0);
  }else if(lcdBuff.indexOf("SSID2")>-1){
    lcdBuff="";
    wifiSSID=WiFi.SSID(1);
  }else if(lcdBuff.indexOf("SSID3")>-1){
    lcdBuff="";
    wifiSSID=WiFi.SSID(2);
  }else if(lcdBuff.indexOf("SSID4")>-1){
    lcdBuff="";
    wifiSSID=WiFi.SSID(3);
  }else if(lcdBuff.indexOf("SSID5")>-1){
    lcdBuff="";
    wifiSSID=WiFi.SSID(4);
  }else if(lcdBuff.indexOf("SSID6")>-1){
    lcdBuff="";
    wifiSSID=WiFi.SSID(5);
  }else if(lcdBuff.indexOf("SSID7")>-1){
    lcdBuff="";
    wifiSSID=WiFi.SSID(6);
  }else if(lcdBuff.indexOf("SSID8")>-1){
    lcdBuff="";
    wifiSSID=WiFi.SSID(7);
  }else if(lcdBuff.indexOf("SSID9")>-1){
    lcdBuff="";
    wifiSSID=WiFi.SSID(8);
  }else if(lcdBuff.indexOf("SSID10")>-1){
    lcdBuff="";
    wifiSSID=WiFi.SSID(9);
  }else if(lcdBuff.indexOf("SSID11")>-1){
    lcdBuff="";
    wifiSSID=WiFi.SSID(10);
  }else if(lcdBuff.indexOf("SSID12")>-1){
    lcdBuff="";
    wifiSSID=WiFi.SSID(11);
  }else if(lcdBuff.indexOf("Connect")>-1){
    lcdBuff="";
    wifiPassword=Serial1.readString();
    connectToWiFi();
  }else if(lcdBuff.indexOf("OK")>-1){
    lcdBuff="";
    param=Serial1.readString();
    offset=(param[3]<<32)|(param[2]<<16)|(param[1]<<8)|(param[0]);
    nChicken=(param[7]<<32)|(param[6]<<16)|(param[5]<<8)|(param[4]);
    nEggs=(param[11]<<32)|(param[10]<<16)|(param[9]<<8)|(param[8]);
    Serial1.print("times.txt=\""+String("Offset: ")+String(offset)+"\""+endChar);
    bufferEEPROM=String(wifiSSID)+"#"+String(wifiPassword)+"*"+String(offset)+"@"+String(nChicken)+"$"+String(nEggs)+"%";
    writeEEPROM(bufferEEPROM);
  }
}

void reconnect() {
  char err_buf[256];
  if (!client.connected()) {
    String clientId = "ESPDogTEr";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    Serial1.print("times.txt=\""+String("Reconnect to Server")+"\""+endChar);
    delay(500);
    sendDisplay();
    if (client.connect(clientId.c_str())) {
      //Serial1.print("CloudStatus.picc=1"+endChar);
      client.publish("outTopic", "hello world");
    } else {
      //Serial1.print("CloudStatus.picc=0"+endChar);
      ESPWifi.lastError(err_buf, sizeof(err_buf));
      delay(5000);
    }
  }
}

void writeEEPROM(String word) {
  delay(10);
  for (int i = 0; i < word.length(); ++i) {
    EEPROM.write(i, word[i]);
  }
  EEPROM.write(word.length(), '\0');
  EEPROM.commit();
}

String readEEPROM() {
  String word;
  char readChar;
  int i = 0;
  while (readChar != '\0') {
    readChar = char(EEPROM.read(i));
    delay(10);
    i++;
    if (readChar != '\0') {
      word += readChar;
    }
  }
  return word;
}

void callbackMqtt(char* topic, byte* payload, unsigned int length) {
  //Serial1.print("times.txt=\""+String(payload)+"\""+endChar);
  if ((char)payload[0] == '1') {
    //Serial1.print("CloudStatus.picc=1"+endChar);
  } else {
    //Serial1.print("CloudStatus.picc=0"+endChar);
  }
}

void publish() {
  int deltaCount= totalCount - lastTotalCount;
  lastTotalCount=totalCount;
  char msg[50];
  Serial1.print("times.txt=\""+String("Send Data to Server")+"\""+endChar);
  snprintf(msg,50,"%ld,%s", deltaCount,times);
  client.publish(mqttTopic, msg);
}

void countTotal(){
  uint32_t tempCount=0;
  for (uint8_t i=0; i< 12; i++){
    tempCount+=sensorCount[i];
  }
  totalCount=tempCount;
  averageCount=totalCount/12;
}

void resetCounter(){
  for(uint8_t i=0; i<12;i++){
    sensorCount[i]=0;
  }
  averageCount=0;
  totalCount=0;
  lastTotalCount=0;
}