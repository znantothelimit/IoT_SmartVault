# IoT 기술이 접목된 스마트 금고 제작

# 팀원
성결대학교 정보통신공학과 23-2 IoT 실습과 응용(수 7-9) 3조 대도

20190895 김찬영 (조장, 총괄, 기획, 프로그래밍)

20190917 신대철 (프로그래밍, 실험 및 성능개선)

20190940 정재호 (부품 구입, 실험 및 성능개선)

20190954 허진환 (프로그래밍, 보고서 작성)


## Overview
 - Linux(Raspbian) 환경에서의 C언어 프로그래밍을 통한 gcc환경에서의 프로그래밍 역량 향상
 - C언어 프로그래밍을 통한 IoT 장치 제어 능력 향상
 - 하드웨어와 소프트웨어가 결합된 프로젝트 진행을 통한 공학적 설계 능력 향상
 - 특정한 기능을 수행하기 위한 시스템 개발인 임베디드 환경에 대한 이해
 - 여러 센서 및 모듈의 응용 능력 향상


## 스마트 금고 동작 방식

### 소프트웨어 기능 블록도

스마트폰 Application을 통해 스마트 금고(Raspberry Pi, Arduino)와 Bluetooth 통신하여 금고 초기 설정
1(지문)차/2(password + 난수 입력)차 잠금 보안성 향상

![S/W block diagram](/img/swblockdg.png?raw=true "Title")

### 초기설정 순서도

![init_flowchart](/img/순서도_라즈베리파이_01초기설정.svg)


### 메인함수 순서도

![main_flowchart1](/img/순서도_라즈베리파이_02메인함수.svg)
![main_flowchart2](/img/순서도_라즈베리파이_03메인함수.svg)


### 아두이노 프로그램 순서도

![arduino_flowchart](/img/순서도_아두이노_프로그램.svg)


## 사용자 정의 함수

![user](/img/userdeffunc.png)


## 제작

스마트 금고 외형
![smartvault](/img/smartvault.jpg)


힌지 제작
![hinge](/img/hinge.jpg)


열렸을 시
![opened](/img/opened.jpg)


## Env

Linux Raspbian gcc compiler

(Before you exec this program, please check your directory/file permission first ; chmod 777 *)
Compile Command : gcc -o iot iot.c -lwiringPi -lwiringPiDev -DRaspberryPi
Exec Command : sudo ./iot