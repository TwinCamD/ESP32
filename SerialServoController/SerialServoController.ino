#include "freertos/task.h"

int led = 2;

unsigned char sendbuf[32];
int txden = 32;             // txdenはデジタルピン32にしてみる
int16_t readAngle = 0;

long nowTime = 0, lastTime = 0;
int loopTimer = 20;//20ms,モータコントロール更新ループとデータ送受信ループ

void torque(int sId, int sMode) {
  unsigned char sum;

  // パケット作成
  sendbuf[0] = (unsigned char) (0xFA); // ヘッダー1
  sendbuf[1] = (unsigned char) (0xAF); // ヘッダー2
  sendbuf[2] = (unsigned char) (sId); // サーボID
  sendbuf[3] = (unsigned char) (0x00); // フラグ
  sendbuf[4] = (unsigned char) (0x24); // アドレス(0x24=36)
  sendbuf[5] = (unsigned char) (0x01); // 長さ(4byte)
  sendbuf[6] = (unsigned char) (0x01); // 個数
  sendbuf[7] = (unsigned char)((sMode&0x00FF)); // ON/OFFフラグ

  // チェックサムの計算
  sum = sendbuf[2];
  for (int i = 3; i < 8; i++) {
    sum = (unsigned char) (sum ^ sendbuf[i]);
  }
  sendbuf[8] = sum; // チェックサム

  // 送信
  digitalWrite(txden, HIGH);
  Serial2.write(sendbuf, 9);
  Serial2.flush();              // データ送信完了待ち
  digitalWrite(txden, LOW);
}

void move(int sId, short angle) {
  unsigned char sum;
  int sPos = (int) angle;

  // パケット作成
  sendbuf[0] = (unsigned char) 0xFA; // ヘッダー1
  sendbuf[1] = (unsigned char) 0xAF; // ヘッダー2
  sendbuf[2] = (unsigned char) sId; // サーボID
  sendbuf[3] = (unsigned char) 0x00; // フラグ
  sendbuf[4] = (unsigned char) 0x1E; // アドレス(0x1E=30)
  sendbuf[5] = (unsigned char) 0x02; // 長さ(2byte)
  sendbuf[6] = (unsigned char) 0x01; // 個数
  sendbuf[7] = (unsigned char) (sPos & 0x00FF); // 位置
  sendbuf[8] = (unsigned char) ((sPos & 0xFF00) >> 8); // 位置

  // チェックサムの計算
  sum = sendbuf[2];
  for (int i = 3; i < 9; i++) {
    sum = (unsigned char)(sum ^ sendbuf[i]);
  }
  sendbuf[9] = sum; // チェックサム

  // 送信
  digitalWrite(txden, HIGH);
  Serial2.write(sendbuf, 10);
  Serial2.flush();              // データ送信完了待ち
  digitalWrite(txden,LOW);
}

void setup(){
  Serial.begin(115200);//PCから
  Serial2.begin(115200);//Servoへ RX=16, RX=17

  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);

  pinMode(txden, OUTPUT);
  digitalWrite(txden, LOW);

  torque(1, 1);//ID1のトルクON
}

void loop(){
  if(Serial.available() >= 2){//2bytes来たら
    readAngle = (Serial.read() + (Serial.read() << 8));//short型
    digitalWrite(led,!digitalRead(led));
  }
  if(abs(readAngle) <= 1500){
    move(1, readAngle);
  }
  delay(1);
}
