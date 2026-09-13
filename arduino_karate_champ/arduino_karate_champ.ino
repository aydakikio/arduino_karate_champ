#include <U8g2lib.h>

// ===== HARDWARE CONFIGURATION =====
U8G2_SH1106_128X64_NONAME_2_HW_I2C u8g2(U8G2_R0, -1, A5, A4);

#define BTN_PLAYER_1_ACTION 12
#define BTN_PLAYER_1_DOWN 9
#define BTN_PLAYER_1_UP 6
#define BTN_PLAYER_1_RIGHT 10
#define BTN_PLAYER_1_LEFT 7

// ===== DISPLAY CONSTANTS =====
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define GAME_AREA_TOP 8

#define DEBOUNCE_MS 50

// ===== SPRITE'S BITMAP ======
// ==== Player walking passing pose  (13 X 24) ======
const unsigned char player_walking_passing_pose_bits[] PROGMEM = { //Base sprite when idle 
0xE0, 0x01, 0xF0, 0x00, 0xB0, 0x01, 0xFC, 0x00, 0x66, 0x00, 0x43, 0x00, 
  0x49, 0x18, 0xC9, 0x1F, 0x39, 0x07, 0x39, 0x04, 0xDF, 0x03, 0x22, 0x00, 
  0x22, 0x00, 0x46, 0x00, 0x4C, 0x00, 0x88, 0x00, 0x88, 0x00, 0x8C, 0x00, 
  0x42, 0x00, 0x37, 0x00, 0x2F, 0x00, 0x2E, 0x00, 0x1E, 0x00, 0x3C, 0x00, 
};

// ==== Player walking right foot contact pose  (16X23) ======
const unsigned char player_walking_right_foot_contact_pose_bits[] PROGMEM = {
  0x00, 0x0F, 0x80, 0x07, 0x80, 0x0D, 0xC0, 0x07, 0x20, 0x03, 0x10, 0xC2, 
  0x48, 0xE4, 0x68, 0xFB, 0xC8, 0x7B, 0xF0, 0x13, 0x20, 0x0D, 0xA0, 0x05, 
  0xA0, 0x06, 0x60, 0x18, 0xD0, 0x33, 0x90, 0x26, 0x48, 0x2C, 0x24, 0x48, 
  0x22, 0x48, 0x11, 0x48, 0x09, 0x78, 0x07, 0x70, 0x0F, 0xE0, 
};

// ==== Player right foot up  (12 X 23) ======
const unsigned char player_walking_right_foot_up_pose_bits[] PROGMEM = {
0x3C, 0x00, 0x1E, 0x00, 0x36, 0x00, 0x1C, 0x0C, 0x1E, 0x0E, 0x29, 0x0E, 
  0xC9, 0x07, 0x01, 0x06, 0x25, 0x03, 0xD9, 0x00, 0xF1, 0x00, 0xFE, 0x00, 
  0x26, 0x00, 0x45, 0x00, 0xFF, 0x00, 0x32, 0x01, 0x52, 0x01, 0x54, 0x01, 
  0x54, 0x02, 0x52, 0x02, 0x4A, 0x01, 0x8E, 0x01, 0x1E, 0x03,  };

// ===== front kick second frame (14 * 26) =====
//10x23
const unsigned char front_kick_second_frame_bits[] PROGMEM = {
0xF8, 0x00, 0x7C, 0x00, 0xDC, 0x00, 0x78, 0x00, 0x36, 0x00, 0x52, 0x00, 
  0x55, 0x00, 0x95, 0x03, 0xF5, 0x03, 0xC5, 0x01, 0xFE, 0x01, 0x51, 0x00, 
  0x29, 0x00, 0x45, 0x00, 0xCA, 0x00, 0x94, 0x00, 0xB8, 0x00, 0xCE, 0x00, 
  0x6E, 0x00, 0x36, 0x00, 0x12, 0x00, 0x1E, 0x00, 0x3C, 0x00, };


// ========= front kick third frame (16* 26)
const unsigned char front_kick_third_frame_bits[] PROGMEM = {
0x70, 0x00, 0xF8, 0x00, 0x58, 0x00, 0x74, 0x00, 0x22, 0x03, 0xD2, 0x03, 
  0xD2, 0x00, 0xE4, 0x07, 0xC4, 0x04, 0x38, 0x05, 0xA4, 0x04, 0x54, 0x06, 
  0x2C, 0x03, 0xF4, 0x01, 0xE4, 0x00, 0xA4, 0x01, 0x34, 0x00, 0x12, 0x00, 
  0x1A, 0x00, 0x09, 0x00, 0x0F, 0x00, 0x1F, 0x00, };

//15*19
const unsigned char front_kick_fourth_frame_bits[] PROGMEM = {
0x30, 0x00, 0x78, 0x00, 0x28, 0x00, 0x3C, 0x00, 0x12, 0x00, 0xA2, 0x01, 
  0xEA, 0x41, 0xD2, 0x7E, 0xE4, 0x61, 0x78, 0x3C, 0x94, 0x07, 0x7C, 0x00, 
  0x24, 0x00, 0x24, 0x00, 0x14, 0x00, 0x12, 0x00, 0x09, 0x00, 0x09, 0x00, 
  0x1F, 0x00, };
//===========Front pucnch =============
//16X19
const unsigned char front_punch_frame_4_bits[] PROGMEM= {
  0xC0, 0x0F, 0xE0, 0x07, 0xE0, 0x0E, 0xE0, 0xC7, 0xC0, 0xFF, 0xE0, 0xE1, 
  0x30, 0xF0, 0x28, 0x0E, 0x28, 0x02, 0x70, 0x02, 0xE0, 0x03, 0xA0, 0x05, 
  0xB0, 0x0A, 0xC8, 0x09, 0x64, 0x09, 0x32, 0x09, 0x11, 0x09, 0x0F, 0x0F, 
  0x1F, 0x1F,};

//17x 22
const unsigned char walk_stride_bits[] PROGMEM = {//For back kick
  0x80, 0x1F, 0x00, 0xC0, 0x0F, 0x00, 0xC0, 0x0F, 0x00, 0xC0, 0x1D, 0x00, 
  0xC0, 0xCF, 0x01, 0x80, 0xFF, 0x01, 0xF0, 0xE2, 0x01, 0x10, 0x2E, 0x00, 
  0x90, 0x1F, 0x00, 0x90, 0x0F, 0x00, 0xF0, 0x0F, 0x00, 0x50, 0x04, 0x00, 
  0xE0, 0x0F, 0x00, 0x10, 0x0A, 0x00, 0x10, 0x11, 0x00, 0x88, 0x12, 0x00, 
  0x48, 0x14, 0x00, 0x24, 0x14, 0x00, 0x12, 0x14, 0x00, 0x09, 0x14, 0x00, 
  0x0F, 0x3C, 0x00, 0x1E, 0x3C, 0x00, };

//16X23
const unsigned char walk_step_l_bits[] PROGMEM = {
0x00, 0x0F, 0x80, 0x07, 0x80, 0x0D, 0xC0, 0x07, 0x20, 0x03, 0x10, 0xC2, 
  0x48, 0xE4, 0x68, 0xFB, 0xC8, 0x7B, 0xF0, 0x13, 0x20, 0x0D, 0xA0, 0x05, 
  0xA0, 0x06, 0x60, 0x18, 0xD0, 0x33, 0x90, 0x26, 0x48, 0x2C, 0x24, 0x48, 
  0x22, 0x48, 0x11, 0x48, 0x09, 0x78, 0x07, 0x70, 0x0F, 0xE0, };

//12X24
const unsigned char walk_step_r_bits[] PROGMEM = {
  0x3C, 0x00, 0x1E, 0x00, 0x36, 0x00, 0x1E, 0x00, 0x0A, 0x00, 0x71, 0x00, 
  0xC5, 0x0F, 0x05, 0x0E, 0xED, 0x03, 0x79, 0x00, 0x7E, 0x00, 0x69, 0x00, 
  0x91, 0x00, 0x29, 0x01, 0xCF, 0x01, 0x44, 0x01, 0x44, 0x03, 0x22, 0x02, 
  0x32, 0x02, 0x11, 0x01, 0x1B, 0x01, 0x97, 0x00, 0x76, 0x00, 0xEC, 0x00, 
  };

//17X21
const unsigned char trot_bits[] PROGMEM = {
0x80, 0x07, 0x00, 0xC0, 0x03, 0x00, 0xC0, 0x06, 0x00, 0x80, 0x03, 0x00, 
  0x60, 0xB5, 0x01, 0x10, 0xC8, 0x01, 0x10, 0x40, 0x00, 0xC8, 0x36, 0x00, 
  0x88, 0x1F, 0x00, 0x90, 0x07, 0x00, 0xE0, 0x06, 0x00, 0xA0, 0x08, 0x00, 
  0x50, 0x19, 0x00, 0xB0, 0x1E, 0x00, 0x88, 0x11, 0x00, 0xC8, 0x36, 0x00, 
  0x24, 0x24, 0x00, 0x32, 0x24, 0x00, 0x09, 0x24, 0x00, 0x07, 0x3C, 0x00, 
  0x0E, 0x7C, 0x00, };

//17X22
const unsigned char walk_right_leg_contact_bits[] PROGMEM = {
0x00, 0x1E, 0x00, 0x0F, 0x00, 0x1B, 0x80, 0x0F, 0x40, 0x04, 0xA0, 0xC8, 
  0x50, 0xEB, 0x90, 0xFB, 0xF0, 0x3B, 0xC0, 0x1F, 0x40, 0x0A, 0xC0, 0x1D, 
  0xA0, 0x30, 0xA0, 0x61, 0x30, 0x4F, 0x98, 0x49, 0xCC, 0xC8, 0x46, 0x88, 
  0x62, 0x98, 0x31, 0xF0, 0x1F, 0x70, 0x3E, 0xE0, };

//15X23
const unsigned char back_kick_frame_1_bits[] PROGMEM= {
  0x80, 0x07, 0xC0, 0x03, 0xC0, 0x06, 0xF0, 0x03, 0x88, 0x01, 0x88, 0x02, 
  0x24, 0x62, 0x94, 0x7C, 0x8A, 0x31, 0x12, 0x13, 0x76, 0x1F, 0x7C, 0x01, 
  0x8A, 0x01, 0x76, 0x01, 0x44, 0x02, 0xC8, 0x02, 0xC8, 0x02, 0xA4, 0x02, 
  0x52, 0x02, 0x52, 0x02, 0x2B, 0x01, 0xC7, 0x00, 0xCE, 0x03, };

//14X24
const unsigned char back_kick_frame_2_bits[] PROGMEM= {
0xF0, 0x03, 0xE0, 0x07, 0xA0, 0x06, 0xE0, 0x07, 0xD8, 0x03, 0xC4, 0x02, 
  0x84, 0x02, 0x12, 0x02, 0x8A, 0x02, 0x8A, 0x0E, 0x6A, 0x38, 0x72, 0x38, 
  0x9C, 0x0F, 0xFA, 0x00, 0x4E, 0x01, 0x84, 0x02, 0x88, 0x02, 0x44, 0x02, 
  0x42, 0x02, 0x22, 0x01, 0x33, 0x01, 0xAF, 0x00, 0xFC, 0x00, 0xE0, 0x01,  };

//12X22
const unsigned char back_kick_frame_3_bits[] PROGMEM= {
  0xF0, 0x03, 0xE0, 0x07, 0xA0, 0x06, 0xF8, 0x07, 0x64, 0x03, 0xE2, 0x01, 
  0xC9, 0x02, 0x49, 0x02, 0x09, 0x0D, 0xB2, 0x0E, 0xBC, 0x0E, 0xCC, 0x03, 
  0x1A, 0x01, 0x14, 0x01, 0x94, 0x00, 0x88, 0x00, 0x78, 0x00, 0xF4, 0x00, 
  0x24, 0x00, 0x22, 0x00, 0x1E, 0x00, 0x3C, 0x00, };

//17X16
const unsigned char back_kick_frame_4_bits[] PROGMEM= {
  0x00, 0xE0, 0x00, 0x00, 0xC0, 0x01, 0x00, 0x70, 0x01, 0x00, 0xCC, 0x01, 
  0xFC, 0xCB, 0x00, 0x06, 0xEE, 0x00, 0x3F, 0xA8, 0x00, 0xC3, 0xBB, 0x01, 
  0x03, 0xD4, 0x01, 0x00, 0x34, 0x00, 0x00, 0x24, 0x00, 0x00, 0x24, 0x00, 
  0x00, 0x14, 0x00, 0x00, 0x14, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x3C, 0x00, 
  };

//Block animation
//frame 1 =walk_right_leg_contact_bits
//12X23
const unsigned char block_frame_two_bits[] PROGMEM= {
  0xC0, 0x01, 0xE0, 0x01, 0x60, 0x03, 0xE0, 0x01, 0xF0, 0x0C, 0x08, 0x0F, 
  0x08, 0x07, 0xF0, 0x05, 0xF0, 0x05, 0x30, 0x07, 0xE0, 0x03, 0x30, 0x01, 
  0xD0, 0x03, 0xC8, 0x04, 0x48, 0x05, 0x28, 0x0D, 0x24, 0x09, 0x14, 0x09, 
  0x12, 0x09, 0x09, 0x09, 0x09, 0x09, 0x07, 0x07, 0x0F, 0x0F, };

//14X23
const unsigned char block_frame_three_bits[] PROGMEM= {
0xC0, 0x03, 0xE0, 0x01, 0x60, 0x03, 0xE0, 0x01, 0xF0, 0x1F, 0x08, 0x38, 
  0xC8, 0x07, 0xA8, 0x04, 0xF0, 0x06, 0xB0, 0x03, 0x20, 0x02, 0xF0, 0x03, 
  0x28, 0x06, 0xD8, 0x07, 0x90, 0x08, 0x90, 0x11, 0x48, 0x13, 0x44, 0x12, 
  0x24, 0x12, 0x22, 0x12, 0x11, 0x12, 0x09, 0x1E, 0x1E, 0x3C, };

//15X21
const unsigned char block_frame_four_bits[] PROGMEM= {
0x80, 0x07, 0xC0, 0x03, 0xC0, 0x06, 0xC0, 0x63, 0xA0, 0x67, 0x50, 0x6B, 
  0x48, 0x3A, 0x88, 0x65, 0xF0, 0x4D, 0x40, 0x32, 0xE0, 0x07, 0xA0, 0x04, 
  0xF0, 0x0F, 0x90, 0x11, 0x88, 0x12, 0x44, 0x12, 0x22, 0x12, 0x12, 0x12, 
  0x09, 0x12, 0x05, 0x09, 0x0F, 0x1F, };

//Sweep animation
//13X22
const unsigned char sweep_animation_frame_one_bits[] PROGMEM= {
0x78, 0x00, 0xF0, 0x00, 0xD8, 0x00, 0x70, 0x00, 0xFE, 0x00, 0x33, 0x01, 
  0x21, 0x01, 0x85, 0x1A, 0x85, 0x1F, 0x49, 0x0F, 0xBE, 0x01, 0xB5, 0x00, 
  0x8B, 0x00, 0xFB, 0x00, 0x81, 0x00, 0x92, 0x00, 0x92, 0x00, 0x92, 0x00, 
  0x53, 0x00, 0x79, 0x00, 0x3F, 0x00, 0x6E, 0x00, };

//15X18
const unsigned char sweep_animation_frame_two_bits[] PROGMEM= {
0x00, 0x3E, 0x00, 0x74, 0x00, 0x7E, 0x00, 0x7C, 0xF0, 0x7F, 0x18, 0x38, 
  0x08, 0x38, 0x04, 0x28, 0x82, 0x20, 0xC2, 0x21, 0x79, 0x29, 0x09, 0x29, 
  0x87, 0x2E, 0xCC, 0x2A, 0x8F, 0x2A, 0x8F, 0x2A, 0xFF, 0x3B, 0x8E, 0x77, };

//17X12
const unsigned char sweep_animation_frame_three_bits[] PROGMEM= {
0x00, 0x78, 0x00, 0x00, 0xF0, 0x00, 0x00, 0xDE, 0x00, 0x00, 0xF1, 0x00, 
  0xC0, 0x85, 0x01, 0x40, 0xA5, 0x01, 0xA0, 0xA6, 0x01, 0x9C, 0xB7, 0x00, 
  0x43, 0xED, 0x00, 0xB9, 0x4D, 0x00, 0x07, 0xDB, 0x00, 0x0E, 0x00, 0x00, };

//13X21
const unsigned char sweep_animation_frame_four_bits[] PROGMEM= {
0xC0, 0x07, 0x80, 0x0F, 0xF8, 0x0A, 0x84, 0x0F, 0x92, 0x07, 0x12, 0x0B, 
  0x12, 0x09, 0x1A, 0x08, 0x8C, 0x09, 0xDE, 0x0F, 0x79, 0x0E, 0x1B, 0x1A, 
  0x02, 0x1A, 0x1C, 0x1A, 0x70, 0x02, 0x4C, 0x02, 0x22, 0x01, 0x27, 0x01, 
  0x1F, 0x01, 0xF3, 0x01, 0xE6, 0x03, };

//15X21
const unsigned char sweep_animation_frame_five_bits[] PROGMEM= {
0xC0, 0x03, 0x80, 0x07, 0xC0, 0x06, 0x80, 0x07, 0xF8, 0x07, 0xC8, 0x03, 
  0x8C, 0x04, 0x04, 0x64, 0x14, 0x74, 0x9C, 0x38, 0x7C, 0x09, 0x6C, 0x0E, 
  0x04, 0x01, 0x88, 0x02, 0x30, 0x01, 0x58, 0x01, 0x24, 0x01, 0x32, 0x01, 
  0xAB, 0x00, 0xA7, 0x00, 0xEE, 0x01, };

// ===== GAME STATE =====
int player_1_score = 0;
int player_2_score = 0;
uint8_t current_round = 0;
bool game_over = false;

struct Player{
  int health;
  int x;
  uint8_t state;
  uint8_t frame;        
  uint32_t anim_last_time;
  uint8_t prev_state;
  bool locked;          
  uint32_t last_pressed_time;
};

Player player_1;
Player player_2;

static uint8_t flipBuf[66];

//Forward declration
void draw_game();
void setup_game();
void handle_inputs();

void setup() {
  u8g2.begin();

  pinMode(BTN_PLAYER_1_ACTION, INPUT_PULLUP);
  pinMode(BTN_PLAYER_1_DOWN,   INPUT_PULLUP);
  pinMode(BTN_PLAYER_1_UP,     INPUT_PULLUP);
  pinMode(BTN_PLAYER_1_RIGHT,  INPUT_PULLUP);
  pinMode(BTN_PLAYER_1_LEFT,   INPUT_PULLUP);

  setup_game();
}

void setup_game() {
  //init player 1
  player_1.x = 30;
  player_1.health = 100;
  player_1.state = 0;
  player_1.frame = 0;
  player_1.anim_last_time = 0;
  player_1.prev_state = 255;
  player_1.locked = false;
  player_1_score = 0;

  //init player 2
  player_2.x = 100;
  player_2.health = 100;
  player_2.state = 0;
  player_2.frame = 0;
  player_2.anim_last_time = 0;
  player_2.prev_state = 255;
  player_2.locked = false;
  player_2_score = 0;
}

void loop() {
  handle_inputs();
  draw_game();
}

void handle_player_1_inputs() {
  uint32_t now = millis();
  if (player_1.locked) return;

  bool action = digitalRead(BTN_PLAYER_1_ACTION) == LOW;
  bool right  = digitalRead(BTN_PLAYER_1_RIGHT)  == LOW;
  bool left   = digitalRead(BTN_PLAYER_1_LEFT)   == LOW;
  bool up     = digitalRead(BTN_PLAYER_1_UP)     == LOW;
  bool down   = digitalRead(BTN_PLAYER_1_DOWN)   == LOW;

  uint8_t ns;
  if      (right && down && action) ns = 9;
  else if (left  && down && action) ns = 8;
  else if (right && up   && action) ns = 6;
  else if (left  && up   && action) ns = 7;
  else if (right && action)         ns = 4;
  else if (left  && action)         ns = 5;
  else if (action)                  ns = 3;
  else if (right)                   ns = 2;
  else if (left)                    ns = 1;
  else                              ns = 0;

  // only debounce on state *changes*, not every frame
  if (ns != player_1.state) {
    if (now - player_1.last_pressed_time < DEBOUNCE_MS) return;
    player_1.last_pressed_time = now;
  }

  player_1.state = ns;
}

void handle_player_2_inputs(){

}

void handle_inputs(){
  handle_player_1_inputs();
  handle_player_2_inputs();
}

static inline uint8_t reverseBits(uint8_t b) {
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
}

void flipXBMH(const uint8_t* src, uint8_t* buf, uint8_t w, uint8_t h) {
    const uint8_t bpr   = (w + 7) >> 3;
    const uint8_t rem   = w & 7;
    const uint8_t shift = rem ? (8 - rem) : 0;

    for (uint8_t row = 0; row < h; row++) {
        uint8_t* dst = buf + (uint16_t)row * bpr;

        // Step 1+2: reverse byte order, reverse bits within each byte
        for (uint8_t i = 0; i < bpr; i++) {
            dst[i] = reverseBits(
                pgm_read_byte(src + (uint16_t)row * bpr + (bpr - 1 - i))
            );
        }

        // Step 3: right-shift the row to remove padding (LSB-left / XBM format)
        if (shift) {
            uint8_t carry = 0;
            for (int8_t i = (int8_t)bpr - 1; i >= 0; i--) {
                uint8_t spill = dst[i] << (8 - shift);
                dst[i] = (dst[i] >> shift) | carry;
                carry = spill;
            }
        }
    }
}

void draw_player_1_walking(uint8_t frame){
  switch (frame) {
    case 0:
      u8g2.drawXBMP(player_1.x, SCREEN_HEIGHT-23, 12, 24, player_walking_right_foot_up_pose_bits);
    break;
    case 1:
      u8g2.drawXBMP(player_1.x, SCREEN_HEIGHT-24, 13, 24, player_walking_passing_pose_bits);
    break;
    case 2:
      u8g2.drawXBMP(player_1.x, SCREEN_HEIGHT-23, 16, 23, player_walking_right_foot_contact_pose_bits);
    break;
  }
}

void draw_player_1_punching(uint8_t frame){
  switch (frame) {
    
    case 0:
      u8g2.drawXBMP(player_1.x, SCREEN_HEIGHT-21, 17, 21 , trot_bits);
    break;
    case 1:
      u8g2.drawXBMP(player_1.x, SCREEN_HEIGHT-24, 12, 24 , walk_step_r_bits);
    break;
    case 2:
      u8g2.drawXBMP(player_1.x, SCREEN_HEIGHT-24, 13, 24, player_walking_passing_pose_bits);
    break;
    case 3:
      u8g2.drawXBMP(player_1.x, SCREEN_HEIGHT-22, 16, 22, walk_right_leg_contact_bits);
    break;
    case 4: 
      u8g2.drawXBMP(player_1.x, SCREEN_HEIGHT-24, 13, 24, player_walking_passing_pose_bits);
    break;
  }
}

void draw_player_1_front_kick(uint8_t frame){
  switch (frame) {
    case 0:
      u8g2.drawXBMP(player_1.x, SCREEN_HEIGHT-24, 13, 24, player_walking_passing_pose_bits);
    break;
    case 1:
      u8g2.drawXBMP(player_1.x, SCREEN_HEIGHT-23, 10, 23, front_kick_second_frame_bits);
    break;
    case 2:
      u8g2.drawXBMP(player_1.x, SCREEN_HEIGHT-22, 10, 22, front_kick_third_frame_bits);
    break;
    case 3:
      u8g2.drawXBMP(player_1.x, SCREEN_HEIGHT-19, 15, 19, front_kick_fourth_frame_bits);
    break;
    case 4:
      u8g2.drawXBMP(player_1.x, SCREEN_HEIGHT-24, 13, 24, player_walking_passing_pose_bits);
     break;
  }
}

void draw_player_1_back_kick(uint8_t frame){
  switch (frame) {
    case 0:
      u8g2.drawXBMP(player_1.x, SCREEN_HEIGHT-23, 15, 23, back_kick_frame_1_bits);
    break;
    case 1:
      u8g2.drawXBMP(player_1.x, SCREEN_HEIGHT-24, 14, 24, back_kick_frame_2_bits);
    break;
    case 2:
      u8g2.drawXBMP(player_1.x, SCREEN_HEIGHT-22, 12, 22, back_kick_frame_3_bits);
    break;
    case 3:
      u8g2.drawXBMP(player_1.x, SCREEN_HEIGHT-16, 17, 16, back_kick_frame_4_bits);
    break;
    case 4:
      u8g2.drawXBMP(player_1.x, SCREEN_HEIGHT-22, 12, 22, back_kick_frame_3_bits);
    break;
    case 5:
      u8g2.drawXBMP(player_1.x, SCREEN_HEIGHT-24, 14, 24, back_kick_frame_2_bits);
    break;
    case 6:
      u8g2.drawXBMP(player_1.x, SCREEN_HEIGHT-23, 15, 23, back_kick_frame_1_bits);
    break;
  }
}

void draw_right_block(uint8_t frame,int x){
  
  switch(frame){
    case 0:
        u8g2.drawXBMP(x, SCREEN_HEIGHT-22, 16, 22, walk_right_leg_contact_bits);
      break;
    case 1:
        u8g2.drawXBMP(x, SCREEN_HEIGHT-23,12,23,block_frame_two_bits);
      break;
    case 2:
        u8g2.drawXBMP(x, SCREEN_HEIGHT-23,14,23,block_frame_three_bits);
      break;
    case 3:
        u8g2.drawXBMP(x, SCREEN_HEIGHT-21,15,21,block_frame_four_bits);
      break;
  }
}

void draw_left_sweep(uint8_t frame,int x){
  switch (frame) {
    case 0:
      u8g2.drawXBMP(x, SCREEN_HEIGHT-22, 13, 22, sweep_animation_frame_one_bits);
    break;
    case 1:
      u8g2.drawXBMP(x, SCREEN_HEIGHT-18, 15, 18, sweep_animation_frame_two_bits);
    break;
    case 2:
      u8g2.drawXBMP(x, SCREEN_HEIGHT-12, 17, 12, sweep_animation_frame_three_bits);
    break;
    case 3:
      u8g2.drawXBMP(x, SCREEN_HEIGHT-21, 13, 21, sweep_animation_frame_four_bits);
    break;
    case 4:
      u8g2.drawXBMP(x, SCREEN_HEIGHT-21, 15, 21, sweep_animation_frame_five_bits);
    break;
    case 5:
      u8g2.drawXBMP(x, SCREEN_HEIGHT-22, 13, 22, sweep_animation_frame_one_bits);
    break;
  }
}

void draw_player_2_walking(uint8_t frame) {
  switch (frame) {
    case 0:
      flipXBMH(player_walking_right_foot_up_pose_bits, flipBuf, 12, 24);
      u8g2.drawXBM(player_2.x, SCREEN_HEIGHT - 23, 12, 24, flipBuf);
      break;

    case 1:
      flipXBMH(player_walking_passing_pose_bits, flipBuf, 13, 24);
      u8g2.drawXBM(player_2.x, SCREEN_HEIGHT - 24, 13, 24, flipBuf);
      break;

    case 2:
      flipXBMH(player_walking_right_foot_contact_pose_bits, flipBuf, 16, 23);
      u8g2.drawXBM(player_2.x, SCREEN_HEIGHT - 23, 16, 23, flipBuf);
      break;
  }
}

void draw_player_2_punching(uint8_t frame) {
  switch (frame) {
    case 0:
      flipXBMH(trot_bits, flipBuf, 17, 21);
      u8g2.drawXBM(player_2.x, SCREEN_HEIGHT - 21, 17, 21, flipBuf);
      break;

    case 1:
      flipXBMH(walk_step_r_bits, flipBuf, 12, 24);
      u8g2.drawXBM(player_2.x, SCREEN_HEIGHT - 24, 12, 24, flipBuf);
      break;

    case 2:
      flipXBMH(player_walking_passing_pose_bits, flipBuf, 13, 24);
      u8g2.drawXBM(player_2.x, SCREEN_HEIGHT - 24, 13, 24, flipBuf);
      break;

    case 3:
      flipXBMH(walk_right_leg_contact_bits, flipBuf, 16, 22);
      u8g2.drawXBM(player_2.x, SCREEN_HEIGHT - 22, 16, 22, flipBuf);
      break;

    case 4:
      flipXBMH(player_walking_passing_pose_bits, flipBuf, 13, 24);
      u8g2.drawXBM(player_2.x, SCREEN_HEIGHT - 24, 13, 24, flipBuf);
      break;
  }
}

void draw_player_2_front_kick(uint8_t frame) {
  switch (frame) {
    case 0:
      flipXBMH(player_walking_passing_pose_bits, flipBuf, 13, 24);
      u8g2.drawXBM(player_2.x, SCREEN_HEIGHT - 24, 13, 24, flipBuf);
      break;

    case 1:
      flipXBMH(front_kick_second_frame_bits, flipBuf, 10, 23);
      u8g2.drawXBM(player_2.x, SCREEN_HEIGHT - 23, 10, 23, flipBuf);
      break;

    case 2:
      flipXBMH(front_kick_third_frame_bits, flipBuf, 10, 22);
      u8g2.drawXBM(player_2.x, SCREEN_HEIGHT - 22, 10, 22, flipBuf);
      break;

    case 3:
      flipXBMH(front_kick_fourth_frame_bits, flipBuf, 15, 19);
      u8g2.drawXBM(player_2.x, SCREEN_HEIGHT - 19, 15, 19, flipBuf);
      break;

    case 4:
      flipXBMH(player_walking_passing_pose_bits, flipBuf, 13, 24);
      u8g2.drawXBM(player_2.x, SCREEN_HEIGHT - 24, 13, 24, flipBuf);
      break;
  }
}

void draw_player_2_back_kick(uint8_t frame) {
  switch (frame) {
    case 0:
      flipXBMH(back_kick_frame_1_bits, flipBuf, 15, 23);
      u8g2.drawXBM(player_2.x, SCREEN_HEIGHT - 23, 15, 23, flipBuf);
      break;

    case 1:
      flipXBMH(back_kick_frame_2_bits, flipBuf, 14, 24);
      u8g2.drawXBM(player_2.x, SCREEN_HEIGHT - 24, 14, 24, flipBuf);
      break;

    case 2:
      flipXBMH(back_kick_frame_3_bits, flipBuf, 12, 22);
      u8g2.drawXBM(player_2.x, SCREEN_HEIGHT - 22, 12, 22, flipBuf);
      break;

    case 3:
      flipXBMH(back_kick_frame_4_bits, flipBuf, 17, 16);
      u8g2.drawXBM(player_2.x, SCREEN_HEIGHT - 16, 17, 16, flipBuf);
      break;

    case 4:
      flipXBMH(back_kick_frame_3_bits, flipBuf, 12, 22);
      u8g2.drawXBM(player_2.x, SCREEN_HEIGHT - 22, 12, 22, flipBuf);
      break;

    case 5:
      flipXBMH(back_kick_frame_2_bits, flipBuf, 14, 24);
      u8g2.drawXBM(player_2.x, SCREEN_HEIGHT - 24, 14, 24, flipBuf);
      break;

    case 6:
      flipXBMH(back_kick_frame_1_bits, flipBuf, 15, 23);
      u8g2.drawXBM(player_2.x, SCREEN_HEIGHT - 23, 15, 23, flipBuf);
      break;
  }
}

void draw_left_block(uint8_t frame,int x) {
  switch (frame) {
    case 0:
      flipXBMH(walk_right_leg_contact_bits, flipBuf, 16, 22);
      u8g2.drawXBM(x, SCREEN_HEIGHT - 22, 16, 22, flipBuf);
      break;

    case 1:
      flipXBMH(block_frame_two_bits, flipBuf, 12, 23);
      u8g2.drawXBM(x, SCREEN_HEIGHT - 23, 12, 23, flipBuf);
      break;

    case 2:
      flipXBMH(block_frame_three_bits, flipBuf, 14, 23);
      u8g2.drawXBM(x, SCREEN_HEIGHT - 23, 14, 23, flipBuf);
      break;

    case 3:
      flipXBMH(block_frame_four_bits, flipBuf, 15, 21);
      u8g2.drawXBM(x, SCREEN_HEIGHT - 21, 15, 21, flipBuf);
      break;
  }
}

void draw_right_sweep(uint8_t frame,int x) {
  switch (frame) {
    case 0:
      flipXBMH(sweep_animation_frame_one_bits, flipBuf, 13, 22);
      u8g2.drawXBM(x, SCREEN_HEIGHT - 22, 13, 22, flipBuf);
      break;

    case 1:
      flipXBMH(sweep_animation_frame_two_bits, flipBuf, 15, 18);
      u8g2.drawXBM(x, SCREEN_HEIGHT - 18, 15, 18, flipBuf);
      break;

    case 2:
      flipXBMH(sweep_animation_frame_three_bits, flipBuf, 17, 12);
      u8g2.drawXBM(x, SCREEN_HEIGHT - 12, 17, 12, flipBuf);
      break;

    case 3:
      flipXBMH(sweep_animation_frame_four_bits, flipBuf, 13, 21);
      u8g2.drawXBM(x, SCREEN_HEIGHT - 21, 13, 21, flipBuf);
      break;

    case 4:
      flipXBMH(sweep_animation_frame_five_bits, flipBuf, 15, 21);
      u8g2.drawXBM(x, SCREEN_HEIGHT - 21, 15, 21, flipBuf);
      break;

    case 5:
      flipXBMH(sweep_animation_frame_one_bits, flipBuf, 13, 22);
      u8g2.drawXBM(x, SCREEN_HEIGHT - 22, 13, 22, flipBuf);
      break;
  }
}

void play_front_kick(bool is_player_1) {
  Player& player = is_player_1 ? player_1 : player_2;

  if (player.state != player.prev_state) {
    player.frame = 0;
    player.anim_last_time = millis();
    player.prev_state = player.state;
    player.locked = true;          
  }

  if (millis() - player.anim_last_time >= 180) {
    player.frame++;
    player.anim_last_time = millis();

    if (player.frame >= 5) {// animation finished
      player.locked = false;
      player.state = 0;
      player.prev_state = 255;
      player.frame = 0;
      return;
    }
  }

  if (is_player_1) draw_player_1_front_kick(player.frame);
  else             draw_player_2_front_kick(player.frame);
}

void play_walking(bool is_player_1) {
  Player& player = is_player_1 ? player_1 : player_2;

  if (player.state != player.prev_state) {
    player.frame = 0;
    player.anim_last_time = millis();
    player.prev_state = player.state;
  }

  if (millis() - player.anim_last_time >= 180) {
    player.frame = (player.frame + 1) % 3;
    player.anim_last_time = millis();
  }

  if (is_player_1) {

    draw_player_1_walking(player.frame);
    if(player.frame == 2 && player_1.x < 110){
      player_1.x +=1;
    }

  } else {

    draw_player_2_walking(player.frame);
    if(player.frame == 2 && player_2.x > 4){
      player_2.x -=1;
    }
  }

}

void play_walking_backward(bool is_player_1) {
  Player& player = is_player_1 ? player_1 : player_2;

  if (player.state != player.prev_state) {
    player.frame = 0;
    player.anim_last_time = millis();
    player.prev_state = player.state;
  }

  if (millis() - player.anim_last_time >= 180) {
    player.frame = (player.frame + 2) % 3;
    player.anim_last_time = millis();
  }

  if (is_player_1) {
    draw_player_1_walking(player.frame);
    if (player.frame == 0 && player_1.x > 4) player_1.x -= 1;
  } else {
    draw_player_2_walking(player.frame);
    if (player.frame == 0 && player_2.x < 110) player_2.x += 1;
  }
}

void play_punching(bool is_player_1) {
  Player& player = is_player_1 ? player_1 : player_2;

  if (player.state != player.prev_state) {
    player.frame = 0;
    player.anim_last_time = millis();
    player.prev_state = player.state;
    player.locked = true;
  }

  if (millis() - player.anim_last_time >= 180) {
    player.frame++;
    player.anim_last_time = millis();

    if (player.frame >= 5) {
      player.locked = false;
      player.state = 0;
      player.prev_state = 255;
      player.frame = 0;
      return;
    }
  }

  if (is_player_1) draw_player_1_punching(player.frame);
  else             draw_player_2_punching(player.frame);
}

void play_back_kick(bool is_player_1) {
  Player& player = is_player_1 ? player_1 : player_2;

  if (player.state != player.prev_state) {
    player.frame = 0;
    player.anim_last_time = millis();
    player.prev_state = player.state;
    player.locked = true;
  }

  if (millis() - player.anim_last_time >= 180) {
    player.frame++;
    player.anim_last_time = millis();

    if (player.frame >= 7) {
      player.locked = false;
      player.state = 0;
      player.prev_state = 255;
      player.frame = 0;
      return;
    }
  }

  if (is_player_1) draw_player_1_back_kick(player.frame);
  else             draw_player_2_back_kick(player.frame);
}

void play_block(bool is_player_1, bool is_right, int x) {
  Player& player = is_player_1 ? player_1 : player_2;

  if (player.state != player.prev_state) {
    player.frame = 0;
    player.anim_last_time = millis();
    player.prev_state = player.state;
  }

  if (millis() - player.anim_last_time >= 180) {
    player.frame = (player.frame + 1) % 4;
    player.anim_last_time = millis();
  }

  if (is_right) draw_right_block(player.frame, x);
  else          draw_left_block(player.frame, x);
}

void play_sweep(bool is_player_1, bool is_left, int x) {
  Player& player = is_player_1 ? player_1 : player_2;

  if (player.state != player.prev_state) {
    player.frame = 0;
    player.anim_last_time = millis();
    player.prev_state = player.state;
    player.locked = true;
  }

  if (millis() - player.anim_last_time >= 180) {
    player.frame++;
    player.anim_last_time = millis();

    if (player.frame >= 6) {
      player.locked = false;
      player.state = 0;
      player.prev_state = 255;
      player.frame = 0;
      return;
    }
  }

  if (is_left) draw_left_sweep(player.frame, x);
  else         draw_right_sweep(player.frame, x);
}

void draw_player_1(){
  switch (player_1.state) {
    case 0:
      u8g2.drawXBMP(player_1.x, SCREEN_HEIGHT-24, 13, 24, player_walking_passing_pose_bits);    
    break;
    case 1:
      play_walking(true);
    break;
    case 2:
      //rverse the frames
      play_walking_backward(true);
    break;
    case 3:
      play_punching(true);
    break;
    case 4:
      play_front_kick(true);
    break;
    case 5:
      play_back_kick(true);
    break;
    case 6:
      play_block(true,true,player_1.x);
    break;
    case 7:
      //block-left
      play_block(true,false,player_1.x);
    break;
    case 8:
      //sweep-left
      play_sweep(true,true, player_1.x);
    break;
    case 9:
      //sweep-right
      play_sweep(true,false, player_1.x);
    break;
  }
}

void draw_player_2(){
  switch (player_2.state) {
    case 0:
      flipXBMH(player_walking_passing_pose_bits, flipBuf, 13, 24);
      u8g2.drawXBMP(player_2.x, SCREEN_HEIGHT-24, 13, 24, flipBuf);    
    break;
    case 1:
      play_walking(false);
    break;
    case 2:
      //rverse the frames
      play_walking_backward(false);
    break;
    case 3:
      play_punching(false);
    break;
    case 4:
      play_front_kick(false);
    break;
    case 5:
      play_back_kick(false);
    break;
    case 6://block- left (normal)
      play_block(false,true,player_2.x);
    break;
    case 7:
      //block-right
      play_block(false,false,player_2.x);
    break;
    case 8://I need more checks to be sure
      //sweep-right
      play_sweep(false,false, player_2.x);
    break;
    case 9://I need more checks to be sure
      //sweep-left
      play_sweep(false,true, player_2.x);
    break;
  }
}


void draw_healthbars(){
  //draw player 1 health bar
  u8g2.drawBox(0,0,(player_1.health/2),10);

  //draw time board

  //draw player 2 health bar
  u8g2.drawBox(78,0,(player_2.health/2),10);
}

void draw_game(){
  u8g2.firstPage();

  do {
    draw_player_1();
    //draw_player_2();
    draw_healthbars();
  }while (u8g2.nextPage());

}