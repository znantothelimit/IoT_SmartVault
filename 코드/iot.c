#include <stdio.h>        // 입출력 관련 헤더파일
#include <stdlib.h>       // 표준 라이브러리 헤더파일
#include <string.h>       // 문자열 관련 헤더파일
#include <errno.h>        // 에러 관련 헤더파일
#include <wiringPi.h>     // GPIO Access 라이브러리
#include <wiringSerial.h> // 시리얼 통신 라이브러리
#include <wiringPiSPI.h>  // GPIO Access 라이브러리
#include <softTone.h>     // Buzzer 라이브러리
#include <lcd.h>          // lcd 라이브러리
#include <softPwm.h>      // pwm 이용
#include <time.h>         // 시간 라이브러리
#include <stdint.h>       //uint8_t definitions

// GPIO::키패드
#define KEYPAD_SIZE 12
#define KEYPAD_PB1 2
#define KEYPAD_PB2 3
#define KEYPAD_PB3 4
#define KEYPAD_PB5 17
#define KEYPAD_PB6 27
#define KEYPAD_PB7 22
#define KEYPAD_PB9 10
#define KEYPAD_PB10 9
#define KEYPAD_PB11 11
#define KEYPAD_PB13 5
#define KEYPAD_PB14 6
#define KEYPAD_PB15 13
#define MAX_KEY_BT_NUM 12

// 키패드::버튼Value
#define B1 1
#define B2 2
#define B3 4
#define B4 8
#define B5 16
#define B6 32
#define B7 64
#define B8 128
#define B9 256
#define BD 512 // DOWN
#define B0 1024
#define BE 2048 // ENTER
const int KeypadTable[12] = {KEYPAD_PB1, KEYPAD_PB2, KEYPAD_PB3,
                             KEYPAD_PB5, KEYPAD_PB6, KEYPAD_PB7,
                             KEYPAD_PB9, KEYPAD_PB10, KEYPAD_PB11,
                             KEYPAD_PB13, KEYPAD_PB14, KEYPAD_PB15};
const char number[12] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '>', '0', 'E'};

// Analog Channel
#define CS_MCP3208 8      // MCP3208 CS 핀 정의
#define SPI_CHANNEL 0     // SPI 통신 채널 정의
#define SPI_SPEED 1000000 // SPI 통신 속도 정의

// GPIO::LED
#define LED_RED_1 18
#define LED_RED_2 23
#define LED_RED_3 24
#define LED_RED_4 7
const int LedTable[4] = {LED_RED_1, LED_RED_2, LED_RED_3, LED_RED_4};

// GPIO::Buzzer
#define BUZZER_PIN 19
#define DO_L 523
#define RE 587
#define MI 659
#define FA 698
#define SOL 784
#define RA 880
#define SI 987
#define DO_H 1046

// GPIO::SERVO
#define SERVO 26

// 기호 상수 정의
#define PWLEN 6    // 비밀번호 길이
#define BUFSIZE 16 // 버퍼의 크기

/*
 * serial connection
 */
// Find Serial device on Raspberry with ~ls /dev/tty*
// ARDUINO_UNO "/dev/ttyACM0"
// FTDI_PROGRAMMER "/dev/ttyUSB0"
// HARDWARE_UART "/dev/ttyAMA0"
char device[] = "/dev/ttyACM0";
// filedescriptor
int fd;
unsigned long baud = 9600;
unsigned long serial_time = 0;

// 시리얼 통신 set-up
void setup()
{

    printf("%s \n", "Raspberry Startup!");
    fflush(stdout);

    // get filedescriptor
    if ((fd = serialOpen(device, baud)) < 0)
    {
        fprintf(stderr, "Unable to open serial device: %s\n", strerror(errno));
        exit(1); // error
    }

    // setup GPIO in wiringPi mode
    if (wiringPiSetup() == -1)
    {
        fprintf(stdout, "Unable to start wiringPi: %s\n", strerror(errno));
        exit(1); // error
    }
}

// serial 통신 함수 원형
void loop()
{
    // Pong every 3 seconds
    if (millis() - serial_time >= 3000)
    {
        serialPuts(fd, "Pong!\n");
        // you can also write data from 0-255
        // 65 is in ASCII 'A'
        serialPutchar(fd, 65);
        serial_time = millis();
    }

    // read signal
    if (serialDataAvail(fd))
    {
        char newChar = serialGetchar(fd);
        printf("\n%c\n", newChar);
        fflush(stdout);
    }
}

// 아두이노와 라즈베리파이 간 시리얼 통신, 라즈베리파이는 수신 받은 char형 변수를 return
char loop_getc()
{
    if (serialDataAvail(fd))
    {
        char newChar = serialGetchar(fd);
        fflush(stdout);
        return newChar;
    }
}

// 키패드 입력받는 함수
int KeypadRead(void)
{
    int nKeypadstate = 0;

    for (int i = 0; i < MAX_KEY_BT_NUM; i++)
    {
        if (!digitalRead(KeypadTable[i]))
        {
            nKeypadstate |= (1 << i);
        }
    }

    return nKeypadstate;
}

// 서보모터 컨트롤
void ServoControl(unsigned char str)
{
    if (str == 'C')
    {
        softPwmWrite(SERVO, 15);
    }
    else if (str == 'R')
    {
        softPwmWrite(SERVO, 25);
    }
    else if (str == 'L')
    {
        softPwmWrite(SERVO, 5);
    }
}

// Buzzer 음계 함수
unsigned int SevenScale(unsigned char scale)
{
    unsigned int _ret = 0;

    switch (scale)
    {
    case 0:
        _ret = DO_L;
        break;
    case 1:
        _ret = RE;
        break;
    case 2:
        _ret = MI;
        break;
    case 3:
        _ret = FA;
        break;
    case 4:
        _ret = SOL;
        break;
    case 5:
        _ret = RA;
        break;
    case 6:
        _ret = SI;
        break;
    case 7:
        _ret = DO_H;
        break;
    }

    return _ret;
}

// Buzzer 주파수 변경
void Change_FREQ(unsigned int freq)
{
    softToneWrite(BUZZER_PIN, freq);
}

// Buzzer 주파수 정지
void STOP_FREQ(void)
{
    softToneWrite(BUZZER_PIN, 0);
}

// Buzzer 초기화
void Buzzer_Init(void)
{
    softToneCreate(BUZZER_PIN);
    STOP_FREQ();
}

// 버튼 입력 시 소리 출력
void btnsound(int mute)
{
    if (mute)
    {
        delay(200);
        return;
    }
    else
    {
        Change_FREQ(SevenScale(0));
        delay(200);
        STOP_FREQ();
    }
}

// 정상적으로 비밀번호 입력 시 소리 출력
void Correct_FREQ(int mute)
{
    if (mute == 0)
    {
        Change_FREQ(DO_H);
        delay(100);
        STOP_FREQ();
        Change_FREQ(RE);
        delay(100);
        STOP_FREQ();
        Change_FREQ(MI);
        delay(100);
        STOP_FREQ();
    }
}

// 문이 닫힐 경우 소리 출력
void Close_FREQ(int mute)
{
    if (mute == 0)
    {
        Change_FREQ(MI);
        delay(100);
        STOP_FREQ();
        Change_FREQ(RE);
        delay(100);
        STOP_FREQ();
        Change_FREQ(DO_H);
        delay(100);
        STOP_FREQ();
    }
}

// 틀릴 경우 소리 출력
void Incorrect_FREQ(int mute)
{
    if (mute == 0)
    {
        for (int i = 0; i < 5; i++)
        {
            Change_FREQ(SOL);
            delay(100);
            STOP_FREQ();
        }
    }
}

// 금고의 메인 메뉴 LCD 출력
void lcd_menu(int disp, int lcd_stat) // LCD_STAT에 따라 화면 출력 변경됨
{
    if (lcd_stat % 4 == 0)
    {
        lcdPosition(disp, 0, 0);
        lcdPuts(disp, "[1]Now Time");

        lcdPosition(disp, 0, 1);
        lcdPuts(disp, "[2]Input PW");
    }
    else if (lcd_stat % 4 == 1)
    {
        lcdPosition(disp, 0, 0);
        lcdPuts(disp, "[2]Input PW");

        lcdPosition(disp, 0, 1);
        lcdPuts(disp, "[3]Change PW");
    }
    else if (lcd_stat % 4 == 2)
    {
        lcdPosition(disp, 0, 0);
        lcdPuts(disp, "[3]Change PW");

        lcdPosition(disp, 0, 1);
        lcdPuts(disp, "[4]Sound On/Off");
    }
    else if (lcd_stat % 4 == 3)
    {
        lcdPosition(disp, 0, 0);
        lcdPuts(disp, "[4]Sound On/Off");

        lcdPosition(disp, 0, 1);
        lcdPuts(disp, "[1]Now Time");
    }
    delay(200);
}

// PW 입력하는 LCD
void lcd_inputpw(int disp, int t)
{
    lcdClear(disp);
    lcdPosition(disp, 0, 0);
    lcdPuts(disp, "[Input PW]");

    for (int i = 0; i < t; i++)
    {
        lcdPosition(disp, i, 1);
        lcdPuts(disp, "*");
    }

    delay(100);
}

// LCD 소리 설정 화면 출력 함수
void lcd_sound(int disp, int mute)
{
    lcdClear(disp);
    lcdPosition(disp, 0, 0);

    if (mute)
    {
        lcdPuts(disp, "Sound off");
    }
    else
    {
        lcdPuts(disp, "Sound on");
    }

    delay(2000);
}

// LED 초기화
void init_led(void)
{
    // LED 초기화
    for (int i = 0; i < 4; ++i)
    {
        pinMode(LedTable[i], OUTPUT);
        digitalWrite(LedTable[i], LOW);
    }
}

// LED를 켜는 함수
void setLed(int leds)
{
    for (int i = 0; i < 4; ++i)
    {
        int state = (leds >> i) & 1;      // i번째 비트 추출
        digitalWrite(LedTable[i], state); // 해당 비트의 값으로 LED를 켬 또는 끔
    }
}

// 십진수 숫자를 BCD (Binary-Coded Decimal)로 변환하는 함수
int decimalToBCD(int decimal)
{
    return ((decimal / 10) << 4) | (decimal % 10);
}

// BCD로 표시된 숫자를 LED에 출력하는 함수
void led_BCD(int number)
{
    if (number >= 0 && number <= 9)
    {
        int bcd = decimalToBCD(number);
        setLed(bcd);
        delay(100);
        setLed(0); // 모든 LED를 끄고 초기화
        delay(100);
    }
}

// 시간을 받아오는 함수
void getCurrentTime(struct tm *t, char *displayTime)
{
    time_t timer = time(NULL);
    *t = *localtime(&timer);
    sprintf(displayTime, "%02d:%02d:%02d", t->tm_hour, t->tm_min, t->tm_sec);
}

// 날짜 받아오는 함수
void getCurrentDate(struct tm *t, char *displayTime)
{
    time_t timer = time(NULL);
    *t = *localtime(&timer);
    sprintf(displayTime, "%04d-%02d-%02d", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
}

// 시간(분)에 따른 LED 출력. 현재 시간이 16분일 경우, 숫자 6의 BCD인 0110 형태로 LED가 출력된다.
int time_led(struct tm curr, char buf_lcd[10])
{
    getCurrentTime(&curr, buf_lcd);
    int min_rand = curr.tm_min % 10;
    led_BCD(min_rand); // 체크
    return min_rand;
}

// 디버깅용 비밀번호 콘솔 출력 함수
void pwprint(int str[])
{
    for (int i = 0; i < PWLEN; i++)
    {
        printf("\n%d\n", str[i]);
    }
}

// 정수형 배열 비교 함수. 비밀번호가 맞는지 확인할 때 사용한다. 비밀번호 6자리에 대해서만 동작함.
int pwcmp(int str1[], int str2[])
{
    for (int i = 0; i < PWLEN; i++)
    {
        if (str1[i] == str2[i])
            continue;
        else
            return 0;
    }
    return 1;
}

// 비밀번호 뒤에 입력한 숫자가 현재 시각의 분과 일치하는지 확인하는 함수.
int mincmp(int buf[], int min)
{
    int k = 0;
    if (min == 0)
    {
        k = 0;
    }
    else if ((min == 1) || (min == 2) || (min == 4) || (min == 8))
    {
        k = 1;
    }
    else if ((min == 3) || (min == 5) || (min == 6) || (min == 9))
    {
        k = 2;
    }
    else if (min == 7)
    {
        k = 3;
    }

    for (int i = 0; i < k; i++)
    {
        if (buf[PWLEN + i] == (1 << (min - 1)))
            continue;
        else
            return 0;
    }
    return 1;
}

// 오픈 시 카메라 촬영 함수
void captureImage()
{
    time_t t;
    struct tm *tm_info;

    time(&t);
    tm_info = localtime(&t);

    char filename[50];
    // 파일명 형식 지정: /project/capturelog/openlog/MMDD_HHMM.jpg
    strftime(filename, sizeof(filename), "/project/capturelog/openlog/%m%d_%H%M.jpg", tm_info);

    char command[100];
    // raspistill 명령어로 이미지 캡처
    snprintf(command, sizeof(command), "raspistill -t 3000 -o %s", filename);

    // 시스템 명령어 실행
    if (system(command) == -1)
    {
        perror("Error capturing image");
    }
}

// 실패시 카메라 촬영 함수
void capturefailedImage()
{
    time_t t;
    struct tm *tm_info;

    time(&t);
    tm_info = localtime(&t);

    char filename[50];
    // 파일명 형식 지정: /project/capturelog/failedlog/MMDD_HHMM.jpg
    strftime(filename, sizeof(filename), "/project/capturelog/failedlog/%m%d_%H%M.jpg", tm_info);

    char command[100];
    // raspistill 명령어로 이미지 캡처
    snprintf(command, sizeof(command), "raspistill -t 3000 -o %s", filename);

    // 시스템 명령어 실행
    if (system(command) == -1)
    {
        perror("Error capturing image");
    }
}

// MCP3208ADC READ 함수
int ReadMcp3208ADC(unsigned char adcChannel)
{
    unsigned char buff[3];
    int nAdcValue = 0;
    buff[0] = 0x06 | ((adcChannel & 0x07) >> 2);
    buff[1] = ((adcChannel & 0x07) << 6);
    buff[2] = 0x00;
    digitalWrite(CS_MCP3208, 0);
    wiringPiSPIDataRW(SPI_CHANNEL, buff, 3);
    buff[1] = 0x0F & buff[1];
    nAdcValue = (buff[1] << 8) | buff[2];
    digitalWrite(CS_MCP3208, 1);
    return nAdcValue;
}

/*
 * MAIN FUNCTION
 */
int main(void)
{
    // 라즈베리파이 gpio setup
    if (wiringPiSetupGpio() == -1)
        return 1;
    //if (wiringPiSPISetup(SPI_CHANNEL, SPI_SPEED) == -1)
    //    return 1;

    // text lcd 초기화
    int disp;
    disp = lcdInit(2, 16, 4, 25, 8, 12, 16, 20, 21, 0, 0, 0, 0);
    lcdClear(disp);

    // buzzer 설정
    softPwmCreate(SERVO, 0, 200);
    Buzzer_Init();

    // 키패드 핀을 입력으로 설정
    for (int i = 0; i < MAX_KEY_BT_NUM; i++)
    {
        pinMode(KeypadTable[i], INPUT);
    }

    //pinMode(CS_MCP3208, OUTPUT);

    // 변수 선언
    int password[PWLEN] = {
        // 비밀번호 배열
        B0, B0, B0, B0, B0, B0};
    int lcd_stat = 0;     // lcd 스크롤 기능
    int in_key = 0;       // 메뉴 입력 키(in)
    int past_key = 0;     // 메뉴 입력 키(past)
    int mute = 0;         // 음소거 여부 - 소리 인자로 사용됨
    int buf_key = 0;      // 입력 버퍼용
    int past_buf_key = 0; // 입력 버퍼용(past)
    int buf_str[BUFSIZE] = {
        // int문자열 입력 버퍼용
        0,
    };

    char buf_char1 = '0'; // 통신 시 사용될 버퍼1
    char buf_char2 = '0'; // 통신 시 사용될 버퍼2
    int cnt = 0;          // 비밀번호 입력시 길이 카운트용
    struct tm curr;       // 시간확인용
    char buf_lcd[BUFSIZE] = {
        0, // lcd 출력 버퍼용
    };

    int passwd_cnt = 0; // 비밀번호 틀린 횟수

    init_led();         // led 초기화
    
    //int smokeValue = 0;
    //int smokeChannel = 2;

    // 1. 아두이노 지문등록 여부 확인(등록시 'a' 수신)
    // 아두이노로부터 문자 ‘a’를 수신받기 위한 루프
    setup();
    while (1)
    {
        lcdPosition(disp, 0, 0);
        lcdPuts(disp, "Waiting...");
        lcdPosition(disp, 0, 1);
        lcdPuts(disp, "Fingerprint");
        buf_char1 = 0;
        buf_char1 = loop_getc();
        if (buf_char1 == 'a')
        {
            buf_char2 = buf_char1;
            fprintf(stdout, "\n%c\n", buf_char2);
            lcdClear(disp);
            break;
        }

        delay(1);
    }

    // 2. 아두이노로부터 비밀번호 등록받음(6자리 정수)
    /*
     * 아두이노로부터 6자리의 정수를 수신해 비밀번호를 초기 설정함.
     * 통신 형식은 문자열이지만, 문자열 ASCII Value로부터 49를 빼면 그것이 실제 정수의 값이 된다.
     * 이를 비밀번호 배열에 저장한다.(1 << 수신한 값 – 49 형식으로 라즈베리파이에 저장됨)
    */
    cnt = 0;
    buf_char1 = 0;
    while (cnt < 6)
    {
        lcdPosition(disp, 0, 0);
        lcdPuts(disp, "Waiting...");
        lcdPosition(disp, 0, 1);
        lcdPuts(disp, "Register PW");
        buf_char1 = loop_getc();
        if (buf_char1 >= '0' && buf_char1 <= '9') // 숫자일 경우
        {
            password[cnt] = (1 << buf_char1 - 49);
            fprintf(stdout, "\n%d\n", password[cnt]);
            printf("\ncnt:%d\n", cnt);
            cnt++;
            delay(100);
        }
        else if (buf_char1 == '*')
        {
            cnt = 0;
            buf_char1 = 0;
        }
    }
    buf_char1 = 0;
    lcdClear(disp);

    /* 비밀번호 등록 이후 동작
     * 메인 프로그램 동작 구간
     */
    while (1)
    {
        buf_char1 = 0;

        memset(buf_str, -1, sizeof(int) * BUFSIZE);
        lcd_menu(disp, lcd_stat);
        in_key = KeypadRead();
        // 버튼이 인식되는 순간(in_key가 000....에서 0이 아닌 값이 되는 순간)
        if ((in_key != 0) && (past_key == 0))
        {
            btnsound(mute);

            // 버튼DOWN : scroll
            if (in_key == BD)
            {
                lcd_stat++;
                lcdClear(disp);
            }

            // 버튼1 : 시간 확인
            else if (in_key == B1)
            {
                while (1)
                {
                    buf_key = KeypadRead();
                    if (buf_key == BE) // 버튼Enter 입력 시 종료
                    {
                        btnsound(mute);
                        break;
                    }
                    time_led(curr, buf_lcd);
                    lcdClear(disp);          // lcd 화면 clear
                    lcdPosition(disp, 0, 1); // lcd 위치 조정
                    lcdPuts(disp, buf_lcd);
                    getCurrentDate(&curr, buf_lcd);
                    lcdPosition(disp, 0, 0); // lcd 위치 조정
                    lcdPuts(disp, buf_lcd);
                    delay(10);
                }
            }

            // 버튼2 : 비밀번호 입력
            else if (in_key == B2)
            {

                // 아두이노로부터 지문인식결과 일치할 경우 'b'를 수신
                cnt = 0;
                while (1)
                {
                    lcdPosition(disp, 0, 0);
                    lcdPuts(disp, "Please Scan");
                    lcdPosition(disp, 0, 1);
                    lcdPuts(disp, "Fingerprint");
                    buf_char1 = 0;
                    buf_char1 = loop_getc();
                    if (buf_char1 == 'b')
                    {
                        buf_char2 = buf_char1;
                        fprintf(stdout, "\n%c\n", buf_char2);
                        lcdClear(disp);
                        break;
                    }

                    delay(1);
                }

                // 키패드에서 비밀번호 입력함
                cnt = 0;
                fprintf(stdout, "\ninput pw mode\n");

                int min = time_led(curr, buf_lcd);
                while (1)
                {
                    min = time_led(curr, buf_lcd); // LED 출력
                    lcd_inputpw(disp, cnt);
                    buf_key = KeypadRead();
                    if ((buf_key != 0) && (past_buf_key == 0)) // 버튼이 인식되는 순간
                    {
                        btnsound(mute);
                        // 숫자가 입력되었을 경우
                        if ((buf_key > 0 && buf_key <= B9) || buf_key == B0)
                        {

                            buf_str[cnt] = buf_key; // 버퍼 문자열에 저장
                            fprintf(stdout, "\nbufkey%d, bufstr%d\n", buf_key, buf_str[cnt]);
                            cnt++;
                        }
                        // 엔터가 입력되었을 경우 비밀번호 확인
                        else if (buf_key == BE)
                        {
                            pwprint(buf_str);
                            if (pwcmp(buf_str, password) && mincmp(buf_str, min))
                            {
                                strcpy(buf_lcd, "pw correct");
                                printf("\npw, min correct\n");
                                lcdClear(disp);          // lcd 화면 clear
                                lcdPosition(disp, 0, 0); // lcd 위치 조정
                                lcdPuts(disp, buf_lcd);
                                delay(1000);
                                passwd_cnt = 0;
                                Correct_FREQ(mute);
                                ServoControl('R');
                                captureImage();

                                delay(3000); // 사용자가 문을 여는 시간을 고려하여 delay
                                buf_char1 = 0;
                                cnt = 0;
                                while (1)
                                {
                                    lcdClear(disp);
                                    lcdPosition(disp, 0, 0);
                                    lcdPuts(disp, "Door Opened");
                                    buf_char1 = 0;
                                    delay(1000);
                                    buf_char1 = loop_getc();
                                    fprintf(stdout, "\n%c\n", buf_char1);
                                    // 닫힐 경우 아래 동작
                                    if (buf_char1 == 'c')
                                    {
                                        lcdClear(disp);
                                        lcdPosition(disp, 0, 0);
                                        lcdPuts(disp, "Door Closed");
                                        delay(300);
                                        ServoControl('C');
                                        Close_FREQ(mute);
                                        delay(3000);
                                        buf_char1 = 0;
                                        break;
                                    }

                                    delay(1);
                                }
                                // 초음파센서와 조도센서로 문이 닫혔는지 감지 후
                                // 닫혔으면 ServoControl('C');
                            }
                            else
                            {
                                passwd_cnt++;
                                Incorrect_FREQ(mute);
                                strcpy(buf_lcd, "pw incorrect");
                                printf("\npw, min incorrect\n");
                                lcdClear(disp);          // lcd 화면 clear
                                lcdPosition(disp, 0, 0); // lcd 위치 조정
                                lcdPuts(disp, buf_lcd);
                                sprintf(buf_lcd, "invalid cnt %d", passwd_cnt);
                                lcdPosition(disp, 0, 1);
                                lcdPuts(disp, buf_lcd);
                                if (passwd_cnt >= 5)
                                {
                                    capturefailedImage();
                                    delay(5000);
                                    passwd_cnt = 0;
                                }
                                delay(1000);
                                lcdClear(disp);
                            }

                            break;
                        }
                    }

                    past_buf_key = buf_key;
                }
            }

            // 버튼3::비밀번호 변경
            else if (in_key == B3)
            {

                // 아두이노로부터 지문인식결과 일치할 경우 'b'를 수신
                cnt = 0;
                while (1)
                {
                    lcdPosition(disp, 0, 0);
                    lcdPuts(disp, "Please Scan");
                    lcdPosition(disp, 0, 1);
                    lcdPuts(disp, "Fingerprint");
                    buf_char1 = 0;
                    buf_char1 = loop_getc();
                    if (buf_char1 == 'b')
                    {
                        buf_char2 = buf_char1;
                        fprintf(stdout, "\n%c\n", buf_char2);
                        lcdClear(disp);
                        break;
                    }

                    delay(1);
                }

                // 키패드에서 비밀번호 입력함
                cnt = 0;
                fprintf(stdout, "\ninput pw mode\n");

                int min = time_led(curr, buf_lcd);
                while (1)
                {
                    lcd_inputpw(disp, cnt);
                    buf_key = KeypadRead();
                    if ((buf_key != 0) && (past_buf_key == 0)) // 버튼이 인식되는 순간
                    {
                        btnsound(mute);
                        // 숫자가 입력되었을 경우
                        if ((buf_key > 0 && buf_key <= B9) || buf_key == B0)
                        {

                            buf_str[cnt] = buf_key; // 버퍼 문자열에 저장
                            fprintf(stdout, "\nbufkey%d, bufstr%d\n", buf_key, buf_str[cnt]);
                            cnt++;
                        }
                        // 엔터가 입력되었을 경우 비밀번호 확인
                        else if (buf_key == BE)
                        {
                            pwprint(buf_str);
                            if (pwcmp(buf_str, password)) // 비밀번호 맞을 경우 변경함
                            {
                                strcpy(buf_lcd, "pw correct");
                                printf("\npw, min correct\n");
                                lcdClear(disp);          // lcd 화면 clear
                                lcdPosition(disp, 0, 0); // lcd 위치 조정
                                lcdPuts(disp, buf_lcd);
                                delay(1000);
                                passwd_cnt = 0;
                                cnt = 0;
                                while (1)
                                {
                                    lcd_inputpw(disp, cnt);
                                    buf_key = KeypadRead();
                                    if ((buf_key != 0) && (past_buf_key == 0))
                                    {
                                        btnsound(mute);
                                        if ((buf_key > 0 && buf_key <= B9) || buf_key == B0)
                                        {
                                            buf_str[cnt] = buf_key; // 버퍼 문자열에 저장
                                            fprintf(stdout, "\nbufkey%d, bufstr%d\n", buf_key, buf_str[cnt]);
                                            cnt++;
                                        }
                                        else if (buf_key == BE)
                                        {
                                            pwprint(buf_str);
                                            if (cnt < PWLEN)
                                                break;
                                            for (int i = 0; i < PWLEN; i++)
                                            {
                                                password[i] = buf_str[i];
                                            }
                                            lcdClear(disp);
                                            lcdPosition(disp, 0, 0);
                                            lcdPuts(disp, "PW Changed!");
                                            delay(1000);
                                            lcdClear(disp);
                                            break;
                                        }
                                    }
                                }
                            }
                            else // 틀린 경우
                            {
                                passwd_cnt++;
                                Incorrect_FREQ(mute);
                                strcpy(buf_lcd, "pw incorrect");
                                printf("\npw, min incorrect\n");
                                lcdClear(disp);          // lcd 화면 clear
                                lcdPosition(disp, 0, 0); // lcd 위치 조정
                                lcdPuts(disp, buf_lcd);
                                sprintf(buf_lcd, "invalid cnt %d", passwd_cnt);
                                lcdPosition(disp, 0, 1);
                                lcdPuts(disp, buf_lcd);
                                if (passwd_cnt >= 5)
                                {
                                    capturefailedImage();
                                    delay(5000);
                                    passwd_cnt = 0;
                                }
                                delay(1000);
                                lcdClear(disp);
                            }
                            break;
                        }
                    }

                    past_buf_key = buf_key;
                }
            }

            // 버튼4 : 음소거 기능
            else if (in_key == B4)
            {
                time_led(curr, buf_lcd);
                mute = !mute;
                lcd_sound(disp, mute);
                lcdClear(disp);
            }
        }
        past_key = in_key;
    }

    return 0;
}
