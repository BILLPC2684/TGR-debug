//###########################//
//###########################//
//#      THE GAME RAZER     #//
//#     MAIN SYSTEM FILE    #//
//#     **DO NOT EDIT**     #//
//###########################//
//###########################//
//###########################//

//
#include <TGR.h>
static void System_Error( int Err, int Inst, int IP, int ID, char Name[]);

bool     showInfo = false;
int      slowdown = 0;
int      FPSLimmit = 60;
int      delay = 0;
int      delay_skip = 62;
int      frames = 0;
bool     Exit = false;
int      argc;
char*    argv[10];
bool     UInput[14][2] = {false};
bool     gotUIn = false;
int      FPS = 0;
char     execLoc[1024];
char*    dropped_filedir = "";
bool*    restart;
char     chars[68];
char*    font[68][8];
bool     ShowFPS;
bool     crash;
uint8_t  overlay[SW][SH][4] = {{{0}}};
bool     enableOL = false;
bool     Title_lock = false;
CPU_INIT CPU;

int main(int c, char *v[]) {
 Title_Name = malloc(1024);
 Title_Name = "TheGameRazer - [NO ROM]";
 argc = c; for (int i=0; i<argc; i++) { argv[i] = v[i]; }  //printf(">>ARG[%d]: %s\n",i,argv[i]);
 
 printf("Loading TGR-PRTO v0.0.32 Alpha Build...\n");
 
 SDL_Init(SDL_INIT_VIDEO);
 SDL_CreateWindowAndRenderer(SW, SH, 2, &window, &GPU_SCREEN);
 SDL_SetRenderDrawColor(GPU_SCREEN, 0, 0, 0, 255);
 SDL_RenderClear(GPU_SCREEN);
 printf("EMU Notice: Screen Opened...\n");
 
 pthread_t call_CPU;
 pthread_create(&call_CPU, NULL, CPU_EXEC, NULL);
 pthread_t call_CLOCK;
 pthread_create(&call_CLOCK, NULL, CPU_CLOCK, NULL);
// clock_t EMU_clock = time(0);
 int p = 0;
 crash = true; //when the EMU crashes it'll know if it was a crash or a exit
 getChar("TEST", 45, 5, 255,255,255,true);
 while (Exit == false) {
  if (Title_Name != "" && Title_lock == false) {
   printf("ROM Title: %s\n",Title_Name);
   SDL_SetWindowTitle(window,Title_Name);
   Title_Name = "";
  }
  /* ############# GPU ############# */
  if (p > 100) {
   SDL_RenderPresent(GPU_SCREEN);
   p = 0;
  } else {
   p++;
  }
  if (GPU.run == true) {
   if (frames < FPSLimmit) {
    if (CPU.debug == true) { printf("GPU: update\n"); }
    SDL_SetRenderDrawColor(GPU_SCREEN, 0, 0, 0, 255);
    SDL_RenderClear(GPU_SCREEN);
    for (int y=0;y<SH;y++) {
     for (int x=0;x<SW;x++) {
      SDL_SetRenderDrawColor(GPU_SCREEN,  buffer[x][y][0],  buffer[x][y][1],  buffer[x][y][2], 255);
      if (enableOL == true && overlay[x][y][3] == 1) {
       SDL_SetRenderDrawColor(GPU_SCREEN, overlay[x][y][0], overlay[x][y][1], overlay[x][y][2], 255);
      }
      SDL_RenderDrawPoint(GPU_SCREEN, x, y);
     }
    }
    SDL_RenderPresent(GPU_SCREEN);
    frames++;
   } else if (CPU.debug == true) { printf("GPU: update skip'd\n"); }
   GPU.run = false;
  }
  if (ShowFPS == true) {
//   bool* ShowFPS;
   getChar("TEST", 25, SH-26, 256,128,132,true);
   getChar("TEST", 26, SH-25, 8,8,8,true);
  }
  /* ############ INPUT ############ */
  const Uint8 *keystates = SDL_GetKeyboardState(NULL);
  gotUIn = false;
  while(SDL_PollEvent(&event)) {
   //Player 1//
   if (keystates[input0_A])      { UInput[ 0][0] = true; } else { UInput[ 0][0] = false; }
   if (keystates[input0_B])      { UInput[ 1][0] = true; } else { UInput[ 1][0] = false; }
   if (keystates[input0_C])      { UInput[ 2][0] = true; } else { UInput[ 2][0] = false; }
   if (keystates[input0_X])      { UInput[ 3][0] = true; } else { UInput[ 3][0] = false; }
   if (keystates[input0_Y])      { UInput[ 4][0] = true; } else { UInput[ 4][0] = false; }
   if (keystates[input0_Z])      { UInput[ 5][0] = true; } else { UInput[ 5][0] = false; }
   if (keystates[input0_L])      { UInput[ 6][0] = true; } else { UInput[ 6][0] = false; }
   if (keystates[input0_R])      { UInput[ 7][0] = true; } else { UInput[ 7][0] = false; }
   if (keystates[input0_Start])  { UInput[ 8][0] = true; } else { UInput[ 8][0] = false; }
   if (keystates[input0_Select]) { UInput[ 9][0] = true; } else { UInput[ 9][0] = false; }
   if (keystates[input0_Up])     { UInput[10][0] = true; } else { UInput[10][0] = false; }
   if (keystates[input0_Down])   { UInput[11][0] = true; } else { UInput[11][0] = false; }
   if (keystates[input0_Left])   { UInput[12][0] = true; } else { UInput[12][0] = false; }
   if (keystates[input0_Right])  { UInput[13][0] = true; } else { UInput[13][0] = false; }
   //Player 2//
   if (keystates[input1_A])      { UInput[ 0][1] = true; } else { UInput[ 0][1] = false; }
   if (keystates[input1_B])      { UInput[ 1][1] = true; } else { UInput[ 1][1] = false; }
   if (keystates[input1_C])      { UInput[ 2][1] = true; } else { UInput[ 2][1] = false; }
   if (keystates[input1_X])      { UInput[ 3][1] = true; } else { UInput[ 3][1] = false; }
   if (keystates[input1_Y])      { UInput[ 4][1] = true; } else { UInput[ 4][1] = false; }
   if (keystates[input1_Z])      { UInput[ 5][1] = true; } else { UInput[ 5][1] = false; }
   if (keystates[input1_L])      { UInput[ 6][1] = true; } else { UInput[ 6][1] = false; }
   if (keystates[input1_R])      { UInput[ 7][1] = true; } else { UInput[ 7][1] = false; }
   if (keystates[input1_Start])  { UInput[ 8][1] = true; } else { UInput[ 8][1] = false; }
   if (keystates[input1_Select]) { UInput[ 9][1] = true; } else { UInput[ 9][1] = false; }
   if (keystates[input1_Up])     { UInput[10][1] = true; } else { UInput[10][1] = false; }
   if (keystates[input1_Down])   { UInput[11][1] = true; } else { UInput[11][1] = false; }
   if (keystates[input1_Left])   { UInput[12][1] = true; } else { UInput[12][1] = false; }
   if (keystates[input1_Right])  { UInput[13][1] = true; } else { UInput[13][1] = false; }
   if (keystates[SDL_SCANCODE_ESCAPE]) {
    if (enableOL == true) {
     enableOL = false;
     printf("[EMU] enableOL: false\n");
    } else {
     enableOL = true;
     printf("[EMU] enableOL: true\n");
    } SDL_Delay(100);
   }
   if (keystates[SDL_SCANCODE_LCTRL] && keystates[SDL_SCANCODE_D]) {
    if (CPU.debug == true) {
     CPU.debug = false;
     printf("EMU: [CPU.debug: False]\n");
    } else {
     CPU.debug = true;
     printf("EMU: [CPU.debug: True]\n");
    } SDL_Delay(100);
   }
   if (keystates[SDL_SCANCODE_LCTRL] && keystates[SDL_SCANCODE_M]) {
    getChar("  `````````````````````````````````````````````````````````", 0, 0,0xFF,0x7F,0x64,true);
    getChar("  ._______._______________________________________________.", 0, 0,0xFF,0x7F,0x7F,true);
    SDL_Delay(100);
   }
   if (keystates[SDL_SCANCODE_LCTRL] && keystates[SDL_SCANCODE_R]) {
    Title_Name = "TheGameRazer";
    if (keystates[SDL_SCANCODE_LSHIFT]) {
     CPU.running = false;
     printf("--------[[EMU-HARD-RESTART]]--------\n");
     for (int y=0;y<SH;y++) { for (int x=0;x<SW;x++) { buffer[x][y][0] = 0; buffer[x][y][1] = 0; buffer[x][y][2] = 0; } }
     CPU.running = true;
     pthread_create(&call_CPU, NULL, CPU_EXEC, NULL);
    } else {
     printf("--------[[EMU-SOFT-RESTART]]--------\n");
     CPU.reset = true;
    } SDL_Delay(100);
   }
   if (keystates[SDL_SCANCODE_LCTRL] && keystates[SDL_SCANCODE_Q]) { Exit = true; }
   switch(event.type) {
//    case SDL_KEYDOWN:
//     printf("Oh! Key press: %d\n",event.type);
//     break;
    case SDL_QUIT:
     crash = false;
     Exit = true;
     break;
    
    case (SDL_DROPFILE):
     dropped_filedir = event.drop.file;
     printf("EMU Notice: Switching ROM to: %s\n",dropped_filedir);
     //SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION,"File dropped on window",dropped_filedir,window);
     //SDL_free(dropped_filedir);
     CPU.running = false;
     printf("--------[[EMU-HARD-RESTART]]--------\n");
     for (int y=0;y<SH;y++) { for (int x=0;x<SW;x++) { buffer[x][y][0] = 0; buffer[x][y][1] = 0; buffer[x][y][2] = 0; } }
     CPU.running = true;
     pthread_create(&call_CPU, NULL, CPU_EXEC, NULL);
     getChar("Loaded ROM:",   8*4 , SH-(8*4), 255,255,128,true);
     getChar(dropped_filedir, 8*16, SH-(8*4), 255,255,128,true);
     break;
   }
  }
  gotUIn = true;
  if (FPS > 0) {
   SDL_Delay(CPU.IPC/12000000);
  }
  if (CPU.running == false && dropped_filedir != "") { Exit = true; }
 }
// SDL_Delay(1000 / (12000000 / CPU.IPC));
 if (argc < 2 && crash == true) { SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION,"TheGameRazer [[HALT DETECTED]]","\nEMU: We have detected the main loop has stopped due to a halt...\n\nThis halt is due to the program has ended or due to a Emulation Error.\nIf there is a Emulation Error check in the terminal for what the problem is in the ROM.\n/!\\Reminder: check the ROM before reporting EMU probblems/!\\",window); }
 SDL_DestroyRenderer(GPU_SCREEN);
 SDL_DestroyWindow(window);
 SDL_Quit();
 printf("EMU Notice: Screen Closed...\n");
 if (crash == true) {
  printf("\nEMU: We have detected the main loop has stopped due to a halt...\n\nThis halt is due to the program has ended or due to a Emulation Error.\nIf there is a Emulation Error check above for what the problem is in the ROM.\n/!\\Reminder: check the ROM before reporting EMU probblems/!\\\n");
 }
 //printf("The screen isn't active but is can still be viewed, to reset the emulator you need to goto your\n terminal and press [CTRL] + [C] and relaunch.");
 //printf("TIP: pressing [CTRL] + [M] will list RAM from RenderRAMPOS + 0x0000 to RenderRAMPOS + 0x02FF\n same with VideoRAM by pressing [CTRL] + [V]...\n and pressing [CTRL] + [KeyPad+] or [CTRL] + [KeyPad-] will adjust RenderRAMPOS by 0x10");
 printf("\nTotal ran instructions: %ld\n\n",CPU.TI);
 return 0;
}

//right val nibble = byte & 0xF;
//left  val nibble = byte >> 4;

void getChar(char* Letter, int x, int y, int R, int G, int B, bool A) {
 for (int i=0;i<strlen(Letter);i++) {
  int j = 0;
  for (j=0;j<70;j++) {
   if (j == 70) { j = 0; break; }
   if (Letter[i] == chars[j]) { break; }
  }
  for (int ix=0;ix<8;ix++) {
   for (int iy=0;iy<8;iy++) {
    if (font[j][iy][ix] == '1' && (i*8)+x+ix >= 0 && (i*8)+x+ix < SW && y+iy >= 0 && y+iy < SH) {
     overlay[(i*8)+x+ix][y+iy][0] = R;
     overlay[(i*8)+x+ix][y+iy][1] = G;
     overlay[(i*8)+x+ix][y+iy][2] = B;
     overlay[(i*8)+x+ix][y+iy][3] = (int)A;
    }
   }
  }
 }
}

char chars[68] = " `ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()-+_=[]{}\\|;:'\".<>/?~'";
char *font[68][8] = {
 {"00000000","00000000","00000000","00000000","00000000","00000000","00000000","00000000"},//   00
 {"11111111","11111111","11111111","11111111","11111111","11111111","11111111","11111111"},// `█01
 {"00111100","01000010","01000010","01111110","01000010","01000010","01000010","00000000"},// A 02
 {"01111100","01000010","01000010","01111100","01000010","01000010","01111100","00000000"},// B 03
 {"00111100","01000010","01000000","01000000","01000000","01000010","00111100","00000000"},// C 04
 {"01111100","01000010","01000010","01000010","01000010","01000010","01111100","00000000"},// D 05
 {"01111110","01000000","01000000","01111100","01000000","01000000","01111110","00000000"},// E 06
 {"01111110","01000000","01000000","01111100","01000000","01000000","01000000","00000000"},// F 07
 {"00111100","01000010","01000000","01001110","01000010","01000010","00111100","00000000"},// G 08
 {"01000010","01000010","01000010","01111110","01000010","01000010","01000010","00000000"},// H 09
 {"01111110","00011000","00011000","00011000","00011000","00011000","01111110","00000000"},// I 10
 {"01111110","00000100","00000100","00000100","00000100","01000100","00111000","00000000"},// J 11
 {"01000100","01001000","01010000","01100000","01010000","01001000","01000100","00000000"},// K 12
 {"01000000","01000000","01000000","01000000","01000000","01000000","01111110","00000000"},// L 13
 {"01000010","01100110","01100110","01011010","01011010","01000010","01000010","00000000"},// M 14
 {"01000010","01100010","01010010","01001010","01000110","01000010","01000010","00000000"},// N 15
 {"00111100","01000010","01000010","01000010","01000010","01000010","00111100","00000000"},// O 16
 {"01111100","01000010","01000010","01111100","01000000","01000000","01000000","00000000"},// P 17
 {"00111100","01000010","01000010","01000010","01001010","01000110","00111110","00000000"},// Q 18
 {"01111100","01000010","01000010","01111100","01010000","01001000","01000100","00000000"},// R 19
 {"00111100","01000010","00100000","00011000","00000100","01000010","00111100","00000000"},// S 20
 {"01111110","00011000","00011000","00011000","00011000","00011000","00011000","00000000"},// T 21
 {"00000000","01000010","01000010","01000010","01000010","01000010","00111100","00000000"},// U 22
 {"00000000","01000010","01000010","01000010","00100100","00100100","00011000","00000000"},// V 23
 {"00000000","01010100","01010100","01010100","01010100","01010100","00101000","00000000"},// W 24
 {"00000000","01000010","00100100","00011000","00011000","00100100","01000010","00000000"},// X 25
 {"01000010","01000010","00100100","00011000","00011000","00011000","00011000","00000000"},// Y 26
 {"01111110","00000100","00001000","00010000","00100000","01000000","01111110","00000000"},// Z 27
 {"00111100","01000010","01100010","01011010","01000110","01000010","00111100","00000000"},// 0 28
 {"00001000","00011000","00001000","00001000","00001000","00001000","00001000","00000000"},// 1 29
 {"00111100","01000010","00000100","00001000","00010000","00100000","01111110","00000000"},// 2 30
 {"00111100","01000010","00000010","00001100","00000010","01000010","00111100","00000000"},// 3 31
 {"00000100","00001100","00010100","00100100","01111110","00000100","00000100","00000000"},// 4 32
 {"01111110","01000000","01000000","01111100","00000010","01000010","00111100","00000000"},// 5 33
 {"00111100","01000010","01000000","01111100","01000010","01000010","00111100","00000000"},// 6 34
 {"01111110","00000010","00000100","00000100","00001000","00001000","00010000","00000000"},// 7 35
 {"00111100","01000010","01000010","00111100","01000010","01000010","00111100","00000000"},// 8 36
 {"00111100","01000010","01000010","00111110","00000010","00000010","00111100","00000000"},// 9 37
 {"00001000","00001000","00001000","00001000","00001000","00000000","00001000","00000000"},// ! 38
 {"00111100","01000010","01110010","01101010","01110010","01011100","00111110","00000000"},// @ 39
 {"00000000","00100100","01111110","00100100","00100100","01111110","00100100","00000000"},// # 40
 {"00011000","00111100","01011010","00111000","00011100","01011010","00111100","00011000"},// $ 41
 {"01100001","10010010","10010100","01101000","00010110","00101001","01001001","10000110"},// % 42
 {"00011000","00100100","01000010","00000000","00000000","00000000","00000000","00000000"},// ^ 43
 {"00011000","00100100","00100100","00111010","01000100","01000100","00111010","00000000"},// & 44
 {"00101010","00011100","00111110","00011100","00101010","00000000","00000000","00000000"},// * 45
 {"00001100","00010000","00010000","00010000","00010000","00010000","00001100","00000000"},// ( 46
 {"00110000","00001000","00001000","00001000","00001000","00001000","00110000","00000000"},// ) 47
 {"00000000","00000000","00000000","01111110","01111110","00000000","00000000","00000000"},// - 48
 {"00000000","00011000","00011000","01111110","01111110","00011000","00011000","00000000"},// + 49
 {"00000000","00000000","00000000","00000000","00000000","00000000","00000000","11111111"},// _ 50
 {"00000000","00000000","01111110","00000000","00000000","01111110","00000000","00000000"},// = 51
 {"00011100","00010000","00010000","00010000","00010000","00010000","00011100","00000000"},// [ 52
 {"00111000","00001000","00001000","00001000","00001000","00001000","00111000","00000000"},// ] 53
 {"00011100","00010000","00010000","00100000","00010000","00010000","00011100","00000000"},// { 54
 {"00111000","00001000","00001000","00000100","00001000","00001000","00111000","00000000"},// } 55
 {"10000000","01000000","00100000","00010000","00001000","00000100","00000010","00000001"},// \ 56 /
 {"00001000","00001000","00001000","00001000","00001000","00001000","00001000","00001000"},// | 57
 {"00000000","00000000","00001000","00000000","00000000","00001000","00010000","00000000"},// ; 58
 {"00000000","00000000","00001000","00000000","00000000","00001000","00000000","00000000"},// : 59
 {"00001000","00001000","00000000","00000000","00000000","00000000","00000000","00000000"},// ' 60
 {"00100100","00100100","00000000","00000000","00000000","00000000","00000000","00000000"},// " 61
 {"00000000","00000000","00000000","00000000","00000000","00000000","00001000","00010000"},// . 62
 {"00000000","00000000","00000000","00000000","00000000","00000000","00001000","00000000"},// < 63
 {"00000000","00000110","00011000","01100000","00011000","00000110","00000000","00000000"},// > 64
 {"00000000","01100000","00011000","00000110","00011000","01100000","00000000","00000000"},// / 65
 {"00000001","00000010","00000100","00001000","00010000","00100000","01000000","10000000"},// ? 66
 {"00010000","00001000","00000000","00000000","00000000","00000000","00000000","00000000"},// ~ 67
 {"00000000","00000000","00000000","00110010","01001100","00000000","00000000","00000000"},// ' 68
};