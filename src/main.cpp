#include <Arduino.h>
#include <LedControl.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//　ブザー設定
const int BUZZER = 12;
int FREQ0 = 528; //　得点アップ
int FREQ1[3] = {}; // ゲームオーバー

// ドットLEDマトリックスのピン設定
const int DIN_PIN = 8;  // データ入力
const int CLK_PIN = 10;  // クロック
const int CS_PIN = 9;   // チップセレクト

// ゲーム設定
const int MATRIX_SIZE = 8;     // ドットLEDマトリックスのサイズ
const int PADDLE_HEIGHT = 2;   // パドルの高さ
const int BALL_DELAY = 150;    // ボールの移動速度（ミリ秒）

// パドルの初期位置
int paddlePosition = 0;

// ボールの初期位置と速度
int ballX = 0;
int ballY = 0;
int ballSpeedX = 1;
int ballSpeedY = 1;

// 得点
int score = 0;
int prev_score = 0;
bool is_gameover = false;

byte lose[] = {
  B10000001,
  B01000010,
  B00100100,
  B00011000,
  B00011000,
  B00100100,
  B01000010,
  B10000001,
};

LedControl lc = LedControl(DIN_PIN, CLK_PIN, CS_PIN, 1);
Adafruit_SSD1306 myDisplay(128, 32);

// パドルの描画
void drawPaddle() {
  lc.clearDisplay(0);
  for (int i = 0; i < PADDLE_HEIGHT; i++) {
    lc.setLed(0, 7, paddlePosition + i, true);
  }
}

// ボールの描画
void drawBall() {
  lc.setLed(0, ballX, ballY, true);
}

void newGame() {
  score = 0;
  Serial.println("Pong");
  Serial.println("START");

  myDisplay.setTextSize(2);
  myDisplay.setTextColor(WHITE);
  myDisplay.setCursor(0, 0);
  myDisplay.println("Pong start");
  myDisplay.display();

  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      lc.setLed(0, j, 7-i, true);
      delay(25);
    }
  }
  delay(1000);
  lc.clearDisplay(0);
  myDisplay.clearDisplay();
  myDisplay.setTextSize(1);
  myDisplay.println("I'm hungry ^_^");
  myDisplay.display();
}

void setSprite(byte *sprite)
{
  for (int i = 0; i < 8; i++)
  {
    lc.setRow(0, i, sprite[i]);
  }
}

void gameOver() {
  Serial.println("GameOver");
  Serial.print("score: ");
  Serial.println(score);

  myDisplay.clearDisplay();
  myDisplay.setTextSize(2);
  myDisplay.setTextColor(WHITE);
  myDisplay.setCursor(0, 0);
  myDisplay.println("Game Over ");
  myDisplay.print("score ");
  myDisplay.print(score);
  myDisplay.display();

  setSprite(lose);
}

void sound() {
  if (score != prev_score) {
    tone(BUZZER, FREQ0);
    delay(50);
    noTone(BUZZER);
  }
  prev_score = score;

  // if (is_gameover) {
  //   for (int i = 0; i < sizeof(FREQ1); i++)
  //   {
  //     tone(BUZZER, FREQ1[i]);
  //     delay(50);
  //   }
  // }
}

void setup() {
  // ドットLEDマトリックスの初期化
  lc.shutdown(0, false);
  lc.setIntensity(0, 8);
  lc.clearDisplay(0);

  // OLEDディスプレイの初期化
  myDisplay.begin(SSD1306_SWITCHCAPVCC, 0x3c);
  myDisplay.clearDisplay();
  myDisplay.display();
  
  // ポテンショメータピンの設定
  pinMode(A0, INPUT);
  // ブザーピンの設定
  pinMode(BUZZER, OUTPUT);

  // シリアル通信の設定
  Serial.begin(9600);

  // ボールのランダム化
  randomSeed(analogRead(0));
  ballX = random(1, 3);
  ballY = random(1, 7);
  do
  {
    ballSpeedX = random(-1, 2);
    ballSpeedY = random(-1, 2);
  } while (ballSpeedX == 0 || ballSpeedY == 0);

  newGame();
}

void loop() {
  // パドルの操作
  int paddleInput = analogRead(A0);
  paddlePosition = map(paddleInput, 0, 1023, 0, 7);
  
  // ボールの移動
  ballX += ballSpeedX;
  ballY += ballSpeedY;
  
  // ボールが上下の壁に当たった場合、反射する
  if (ballY <= 0 || ballY >= 7) {
    ballSpeedY *= -1;
  }

  // ボールが左の壁に当たった場合，反射する
  if (ballX <= 0) {
    ballSpeedX *= -1;
  }

  // ボールがパドルに当たった場合、反射する
  if (ballX == 6 && ballY >= paddlePosition && ballY <= paddlePosition + 1) {
    ballSpeedX *= -1;
    score++;
  }
  
  // ボールが左端に当たった場合、ゲームオーバー
  if (ballX >= 7) {
    ballSpeedX  = 0;
    ballSpeedY = 0;
    is_gameover = true;
  }
  
  // ドットLEDマトリックスの描画
  drawPaddle();
  drawBall();
  sound();

  // ゲームオーバー時の処理
  if (is_gameover) {
    gameOver();
    while (1) {}
  }
  
  // フレームレートの調整
  delay(BALL_DELAY);
}
