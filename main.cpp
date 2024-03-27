#include <cstdlib>
#include <ctime>
#include "console.h"
#include <string>

#define BOARD_SIZE 11 
#define FIELD_SIZE (BOARD_SIZE - 2) * (BOARD_SIZE - 2)

#define MOVE_DELAY 30

#define LOSE_TEXT "YOU LOSE"
#define WIN_TEXT "YOU WIN"
#define TRY_TEXT "Try again? (Enter)"

#define WALL_VERTICAL_STRING "┃"
#define WALL_HORIZONTAL_STRING "━"
#define WALL_RIGHT_TOP_STRING "┓"
#define WALL_LEFT_TOP_STRING "┏"
#define WALL_RIGHT_BOTTOM_STRING "┛"
#define WALL_LEFT_BOTTOM_STRING "┗"
#define SNAKE_STRING "■"
#define SNAKE_BODY_STRING "■"
#define APPLE_STRING "●"

enum Direction {
  Left = 0,
  Right,
  Up,
  Down,
};

struct Point {
  int x;
  int y;
};

// L R U D
const int dirDiff[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

Direction tempDir = Right;
Direction realDir = Right;

/*
  0 ~ snakeBodyLength-1: snake point
  snakeBodyLength ~ FIELD_SIZE: blank field point
*/ 
Point field[FIELD_SIZE];
Point feedPoint = {-1, -1};

int x = (BOARD_SIZE - 1) / 2;
int y = (BOARD_SIZE - 1) / 2;

int snakeBodyLength = 1;
bool isOver = false;

void reset() {
  realDir = Right;
  tempDir = Right;

  x = (BOARD_SIZE - 1) / 2 - dirDiff[realDir][0];
  y = (BOARD_SIZE - 1) / 2 - dirDiff[realDir][1];

  feedPoint.x = -1;
  feedPoint.y = -1;

  snakeBodyLength = 1;

  field[0].x = x;
  field[0].y = y;

  isOver = false;
}
void handleInput() {
  if (console::key(console::K_LEFT) && (snakeBodyLength == 1 || realDir != Right))
    tempDir = Left;
  else if (console::key(console::K_RIGHT) && (snakeBodyLength == 1 || realDir != Left))
    tempDir = Right;
  else if (console::key(console::K_UP) && (snakeBodyLength == 1 || realDir!= Down))
    tempDir = Up;
  else if (console::key(console::K_DOWN) && (snakeBodyLength == 1 || realDir != Up))
    tempDir = Down;
  else if (console::key(console::K_ENTER) && isOver)
    reset();
  else if (console::key(console::K_ESC))
    exit(0);
}

void drawSnake() {
  console::draw(field[0].x, field[0].y, SNAKE_STRING);
  for(int i = 1; i < snakeBodyLength; i++){
    console::draw(field[i].x, field[i].y, SNAKE_BODY_STRING);
  }
}

void setSnakePoint() {
  for(int i = snakeBodyLength-1; i > 0; i--){
    field[i].x = field[i-1].x;
    field[i].y = field[i-1].y;
  }
  field[0].x = x;
  field[0].y = y;
}

void fillBlankFieldPoint() {
  /* 
  temp의 index는 field의 각 번호와 같습니다. (Border를 제외한 순수 영역을 번호로 표현)
    0 1 2 3 
    4 5 6 7
    8 9 10 11
  */

 /*
  field의 번호에 뱀이 있는지 확인
 */
  int temp[FIELD_SIZE] = {0};
  for(int i = 0; i < snakeBodyLength; i++){
    const int fieldNumber = (field[i].y - 1) * (BOARD_SIZE - 2) + (field[i].x - 1);
    if (fieldNumber < 0 || FIELD_SIZE <= fieldNumber) {
      isOver = true;
      return;
    }
    temp[fieldNumber] ++;
  }

  // 뱀이 있는 field 번호를 제외한 곳 (빈칸) 정보를 field에 저장합니다
  int index = snakeBodyLength;
  for(int i = 0; i < FIELD_SIZE; i++) {
    if (temp[i] > 0) continue;

    field[index].x = i % (BOARD_SIZE - 2) + 1;
    field[index].y = i / (BOARD_SIZE - 2) + 1;
    index++;
  }
}

bool isCollision() {
  if (x <= 0 || x >= BOARD_SIZE - 1 || y <= 0 || y >= BOARD_SIZE - 1) {
    return true;
  }

  for(int i = 1; i < snakeBodyLength; i++) {
    if (field[i].x == x && field[i].y == y) {
      return true;
    }
  }
  return false;
}

void snakeController() {
  x += dirDiff[realDir][0];
  y += dirDiff[realDir][1];

  // 먹이를 먹었을때
  if (feedPoint.x == x && feedPoint.y == y) {
    snakeBodyLength++;
    feedPoint.x = -1;
    feedPoint.y = -1;
  }

  setSnakePoint();
  drawSnake();
}

void drawBoardBorder() {
  console::draw(0, 0, WALL_LEFT_TOP_STRING);
  console::draw(BOARD_SIZE - 1, BOARD_SIZE - 1, WALL_RIGHT_BOTTOM_STRING);
  console::draw(BOARD_SIZE - 1, 0, WALL_RIGHT_TOP_STRING);
  console::draw(0, BOARD_SIZE - 1, WALL_LEFT_BOTTOM_STRING);

  for (int i = 1; i < BOARD_SIZE - 1; i++) {
    console::draw(i, 0, WALL_HORIZONTAL_STRING);
    console::draw(i, BOARD_SIZE - 1, WALL_HORIZONTAL_STRING);
    console::draw(0, i, WALL_VERTICAL_STRING);
    console::draw(BOARD_SIZE - 1, i, WALL_VERTICAL_STRING);
  }
}

void makeFeed() {
  if (snakeBodyLength == FIELD_SIZE) {
    return;
  }
  /*
    빈칸 영역 중 랜덤한 위치에 먹이를 생성합니다
  */
  if (feedPoint.x == -1 && feedPoint.y == -1) {
    int feedIndex = (rand() % (FIELD_SIZE - snakeBodyLength)) + snakeBodyLength;
    feedPoint.x = field[feedIndex].x;
    feedPoint.y = field[feedIndex].y;
  }
  console::draw(feedPoint.x, feedPoint.y, APPLE_STRING);
}

int getCentorXPoint(std::string str) {
  int dx = (BOARD_SIZE - str.length()) / 2;
  return dx < 0 ? 0 : dx;
}

void drawScore() {
  int score = (snakeBodyLength - 1) * 10;
  std::string text = "Score: " + std::to_string(score);
  console::draw(getCentorXPoint(text), BOARD_SIZE, text);
}

void game() {
  reset();
  int frame = -1;
  console::init();
  while (true) {
    frame++;
    handleInput();
    
    if (isOver) {
      std::string text = snakeBodyLength == FIELD_SIZE ? WIN_TEXT : LOSE_TEXT;
      console::draw(getCentorXPoint(text), BOARD_SIZE / 2, text);
      console::draw(getCentorXPoint(TRY_TEXT), BOARD_SIZE / 2 + 1, TRY_TEXT);
      console::wait();
    }

    if (isCollision() || snakeBodyLength == FIELD_SIZE) {
      isOver = true;
      continue;
    }

    if (0 < frame && frame < MOVE_DELAY) {
      console::wait();
      continue;
    }

    //랜더 되기 전에 입력한 방향을 적용합니다.
    realDir = tempDir;

    console::clear();

    /*
      1. 테두리 만들기
      2. 뱀 움직이거나 몸 추가
      3. blank field 채우기
      3. 먹이 생성
      4. 스코어 출력
    */
    drawBoardBorder();
    snakeController();
    fillBlankFieldPoint();
    makeFeed();
    drawScore();

    console::wait();
    frame = 0;
  }
}

int main() {
  srand(time(NULL));
  game();
  return 0;
}
