#include <SoftwareSerial.h>
#include <Adafruit_Fingerprint.h>

// 초음파 센서
#define BT_RXD 8
#define BT_TXD 7

// 아래는 지문인식 센서 설정 단계
#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
// For UNO and others without hardware serial, we must use software serial...
// pin #2 is IN from sensor (GREEN wire)
// pin #3 is OUT from arduino  (WHITE wire)
// Set up the serial port to use softwareserial..
SoftwareSerial mySerial(2, 3);

#else
// On Leonardo/M0/etc, others with hardware serial, use hardware serial!
// #0 is green wire, #1 is white
#define mySerial Serial1

#endif

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

uint8_t id;


// 블루투스 setup
SoftwareSerial bluetooth(BT_RXD, BT_TXD);

int echo = 11; // 초음파
int trig = 12; // cds
int cds = A1; // cds

// 시리얼 통신 setup
void setup()
{
  Serial.begin(9600);
  while (!Serial)
    ; // For Yun/Leo/Micro/Zero/...
  delay(100);

  // set the data rate for the sensor serial port
  finger.begin(57600);
  if (finger.verifyPassword())
  {
  }
  else
  {
    while (1)
    {
      delay(1);
    }
  }
  finger.getParameters();
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  pinMode(cds, INPUT);
}

// 아두이노 메인 동작 구간
void loop()
{
  long password = 0;             // 비밀번호
  int password_count = 0;        // 비밀번호 카운트
  int cdsValue = analogRead(A1); // 조도센서
  float cycletime;
  int distance;
  id = 1;
  int check = 0;
  while (!getFingerprintEnroll())
    ;
  Serial.println("a"); // 지문 등록 완료될 경우 시리얼 송신 'a'

  while (1)
  {
    // 블루투스 -> 아두이노 비밀번호 받기
    bluetooth.begin(9600);
    if (bluetooth.available())
    {
      Serial.println(bluetooth.read() - 48);
      delay(100);
      while (bluetooth.available())
      {
        delay(100);
        Serial.println(bluetooth.read() - 48);
      }

      // 비밀번호 라즈베리파이에 전송 후 센서 코드 실행

      while (1)
      {
        // 지문인식 계속 동작
        Serial.begin(9600);
        while (!Serial)
          ; // For Yun/Leo/Micro/Zero/...
        delay(100);

        // set the data rate for the sensor serial port
        finger.begin(57600);
        delay(5);
        if (finger.verifyPassword())
        {
        }
        else
        {
          while (1)
          {
            delay(1);
          }
        }
        finger.getParameters();
        finger.getTemplateCount();

        // 지문인식 입력받는 부분
        while (getFingerprintID() != 1) // 지문인식이 되지 않을 경우 계속 반복
        {

          digitalWrite(trig, HIGH);
          delay(10);
          digitalWrite(trig, LOW);

          cycletime = pulseIn(echo, HIGH);

          distance = (int)(((340 * cycletime) / 10000) / 2); // 초음파 센서 value
          cdsValue = (int)analogRead(A1); // cds 센서 value
          if (cdsValue < 80 && distance < 13) // cds value 기준치 이하고(어두움) 거리가 13센치 이하일 경우 닫혔다고 판단
          {
            delay(50);
            if (check == 0)
            {
              Serial.println("c"); // 문 닫힘을 판단하고 라즈베리파이에 'c'전송
              check = 1;
            }
            delay(1000);
          }
          else
          {
            check = 0;
          }

          delay(500);

          // don't ned to run this at full speed.
        }

        Serial.println("b");
      }
    }
  }
  return 1;
}

uint8_t getFingerprintEnroll()
{

  int p = -1;
  while (p != FINGERPRINT_OK)
  {
    p = finger.getImage();
    switch (p)
    {
    case FINGERPRINT_OK:
      break;
    case FINGERPRINT_NOFINGER:
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      break;
    case FINGERPRINT_IMAGEFAIL:
      break;
    default:
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p)
  {
  case FINGERPRINT_OK:
    break;
  case FINGERPRINT_IMAGEMESS:
    return p;
  case FINGERPRINT_PACKETRECIEVEERR:
    return p;
  case FINGERPRINT_FEATUREFAIL:
    return p;
  case FINGERPRINT_INVALIDIMAGE:
    return p;
  default:
    return p;
  }

  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER)
  {
    p = finger.getImage();
  }
  p = -1;
  while (p != FINGERPRINT_OK)
  {
    p = finger.getImage();
    switch (p)
    {
    case FINGERPRINT_OK:
      break;
    case FINGERPRINT_NOFINGER:
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      break;
    case FINGERPRINT_IMAGEFAIL:
      break;
    default:
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p)
  {
  case FINGERPRINT_OK:
    break;
  case FINGERPRINT_IMAGEMESS:
    return p;
  case FINGERPRINT_PACKETRECIEVEERR:
    return p;
  case FINGERPRINT_FEATUREFAIL:
    return p;
  case FINGERPRINT_INVALIDIMAGE:
    return p;
  default:
    return p;
  }

  // OK converted!

  p = finger.createModel();
  if (p == FINGERPRINT_OK)
  {
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR)
  {
    return p;
  }
  else if (p == FINGERPRINT_ENROLLMISMATCH)
  {
    return p;
  }
  else
  {
    return p;
  }

  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK)
  {
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR)
  {
    return p;
  }
  else if (p == FINGERPRINT_BADLOCATION)
  {
    return p;
  }
  else if (p == FINGERPRINT_FLASHERR)
  {
    return p;
  }
  else
  {
    return p;
  }

  return true;
}

uint8_t getFingerprintID()
{
  uint8_t p = finger.getImage();
  switch (p)
  {
  case FINGERPRINT_OK:
    break;
  case FINGERPRINT_NOFINGER:
    return p;
  case FINGERPRINT_PACKETRECIEVEERR:
    return p;
  case FINGERPRINT_IMAGEFAIL:
    return p;
  default:
    return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p)
  {
  case FINGERPRINT_OK:
    break;
  case FINGERPRINT_IMAGEMESS:
    return p;
  case FINGERPRINT_PACKETRECIEVEERR:
    return p;
  case FINGERPRINT_FEATUREFAIL:
    return p;
  case FINGERPRINT_INVALIDIMAGE:
    return p;
  default:
    return p;
  }

  // OK converted!
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK)
  {
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR)
  {
    return p;
  }
  else if (p == FINGERPRINT_NOTFOUND)
  {
    return p;
  }
  else
  {
    return p;
  }

  // found a match!

  return finger.fingerID;
}

// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez()
{
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)
    return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)
    return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)
    return -1;

  // found a match!

  return finger.fingerID;
}
