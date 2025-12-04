#include <U8g2lib.h>

// ===== HARDWARE CONFIGURATION =====
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, -1, A5, A4);

#define BTN_PLAYER_1_ACTION 12
#define BTN_PLAYER_1_DOWN 9
#define BTN_PLAYER_1_UP 6
#define BTN_PLAYER_1_RIGHT 10
#define BTN_PLAYER_1_LEFT 7
#define BTN_PLAYER_1_JUMP 13

#define BTN_PLAYER_2_ACTION 2
#define BTN_PLAYER_2_UP 11
#define BTN_PLAYER_2_DOWN 3
#define BTN_PLAYER_2_RIGHT 8
#define BTN_PLAYER_2_LEFT 5
#define BTN_PLAYER_2_JUMP 4

// ===== DISPLAY CONSTANTS =====
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define GAME_AREA_TOP 8
#define GAME_AREA_HEIGHT (SCREEN_HEIGHT - GAME_AREA_TOP)

// ===== GAME STATE =====
int player_1_score = 0;
int player_2_score = 0;
bool game_over = false;

//game scene states
bool game_first_draw = true;
bool gameover_first_draw = true;

// ===== ANIMATION METADATA =====
unsigned long player_1_last_frame_time = 0;
uint8_t player_1_current_frame = 0; 

// ===== SPRITE'S BITMAP ======

// ==== Player walking passing pose  (14 X 26) ======
const unsigned char player_walking_passing_pose_bits[] PROGMEM = {
  0x00, 0x00, 0x7E, 0x00, 0x3F, 0x00, 0x27, 0x00, 0x6B, 0x00, 0x23, 0x38, 
  0x33, 0x28, 0xEA, 0x2F, 0x0A, 0x34, 0xF9, 0x1D, 0x21, 0x05, 0xFF, 0x03, 
  0x5A, 0x00, 0x7A, 0x00, 0x46, 0x00, 0x86, 0x00, 0x1E, 0x01, 0x14, 0x01, 
  0x1C, 0x01, 0x08, 0x01, 0x84, 0x00, 0x47, 0x00, 0x2D, 0x00, 0x7B, 0x00, 
  0x92, 0x00, 0xFE, 0x00, };

// ==== Player walking right foot contact pose  (17 X 26) ======
const unsigned char player_walking_right_foot_contact_pose_bits[] PROGMEM= {
  0xE0, 0x07, 0x00, 0x70, 0x02, 0x00, 0xB0, 0x06, 0x00, 0x30, 0xC2, 0x01, 
  0xA0, 0x41, 0x01, 0x18, 0x47, 0x01, 0x08, 0xBC, 0x01, 0x28, 0xE0, 0x00, 
  0xC8, 0x23, 0x00, 0x88, 0x22, 0x00, 0xB0, 0x3A, 0x00, 0xF0, 0x0F, 0x00, 
  0x48, 0x02, 0x00, 0x68, 0x06, 0x00, 0xF8, 0x07, 0x00, 0x10, 0x0C, 0x00, 
  0x30, 0x10, 0x00, 0xD0, 0x17, 0x00, 0x50, 0x12, 0x00, 0x48, 0x12, 0x00, 
  0x44, 0x12, 0x00, 0x66, 0x12, 0x00, 0x32, 0x1E, 0x00, 0x17, 0x72, 0x00, 
  0x39, 0x46, 0x00, 0x41, 0x7C, 0x00, };

// ==== Player right foot up  (16 X 26) ======
const unsigned char player_walking_right_foot_up_pose_bits[] PROGMEM = {
  0xE0, 0x07, 0x70, 0x02, 0xB0, 0x06, 0x30, 0x02, 0x90, 0xE3, 0xC8, 0xA1, 
  0x04, 0xB7, 0x14, 0xDC, 0x14, 0x70, 0x34, 0x20, 0xE4, 0x37, 0x4C, 0x1E, 
  0xFC, 0x03, 0x24, 0x06, 0x22, 0x04, 0xFE, 0x07, 0x14, 0x04, 0x34, 0x04, 
  0xE6, 0x08, 0xA2, 0x08, 0x51, 0x04, 0x49, 0x04, 0x4F, 0x02, 0xC9, 0x03, 
  0x53, 0x04, 0xDE, 0x07,  };


// ===== GAME OBJECTS =====
struct Player{
  int health;
  int damage;
  int x;
  int y;
};

Player player_1;
Player player_2;

// forward declaration
void draw_game();
void initialize_game();

void setup() {
  u8g2.begin();
  // intialising player 1 buttons 
  pinMode(BTN_PLAYER_1_ACTION, INPUT_PULLUP);
  pinMode(BTN_PLAYER_1_DOWN, INPUT_PULLUP);
  pinMode(BTN_PLAYER_1_UP, INPUT_PULLUP);
  pinMode(BTN_PLAYER_1_RIGHT, INPUT_PULLUP);
  pinMode(BTN_PLAYER_1_LEFT, INPUT_PULLUP);

}

void loop() {
  // put your main code here, to run repeatedly:
  if (!game_over) {
    draw_game();

  }  
}

// ==== DRAWING SPRITES =====
void draw_lunet_punch(uint8_t frame){
  switch (frame) {
    case 0:
    break;
  }
}

void draw_walking(uint8_t frame){
  switch (frame) {
    case 0:
      u8g2.drawXBMP(32, SCREEN_HEIGHT -26, 16, 26, player_walking_right_foot_up_pose_bits);
    break;
    case 1:
      u8g2.drawXBMP(32, SCREEN_HEIGHT - 26 , 14 , 26, player_walking_passing_pose_bits);
    break;
    case 2:
      u8g2.drawXBMP(32, SCREEN_HEIGHT -26, 17, 26, player_walking_right_foot_contact_pose_bits);
    break;
    case 3:

    break;
  }
}

// ==== RENDERIG GAME ANIMATIONS ===
void play_lunet_punch(bool is_player_1){
  switch (is_player_1) {
    case true:
    break;
    case false:
    break;
  }
}
void play_front_punch(bool is_player_1){
  switch (is_player_1) {
    case true:
    break;
    case false:
    break;
  }
}

void play_walking_animation(){
  unsigned long player_1_walking_animation_now = millis();
  if (player_1_walking_animation_now - player_1_last_frame_time >100 ) {
    player_1_current_frame = (player_1_current_frame + 1) % 3;
    player_1_last_frame_time = player_1_walking_animation_now;
  }

  draw_walking(player_1_current_frame);
}
// ==== RENDERING GAME SCENE =====
void draw_game(){
  /* 
  if (game_first_draw) {
    u8g2.clearBuffer();


    u8g2.sendBuffer();
    game_first_draw= false;
  }
  */

  u8g2.clearBuffer();

  play_walking_animation();



  u8g2.sendBuffer();
}