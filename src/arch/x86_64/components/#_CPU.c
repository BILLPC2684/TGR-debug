//###########################//
//###########################//
//#      THE GAME RAZER     #//
//#     CPU SYSTEM FILE     #//
//#     **DO NOT EDIT**     #//
//###########################//
//###########################//
//###########################//

#include <TGR.h>

void *CPU_CLOCK(void *null) {
 while (true) {
  while (CPU.reset == true) {}
  SDL_Delay(10); //0.1 secs
  CPU.time++;
  if (CPU.time%100 == 0) {
   FPS = frames;
   if (showInfo == true) {
    printf("P1:[A:%d,B:%d,C:%d,X:%d,Y:%d,Z:%d,L:%d,R:%d,Start:%d,Select:%d,Up:%d,Down:%d,Left:%d,Right:%d]\nP2:[A:%d,B:%d,C:%d,X:%d,Y:%d,Z:%d,L:%d,R:%d,Start:%d,Select:%d,Up:%d,Down:%d,Left:%d,Right:%d]\n",UInput[0][0],UInput[1][0],UInput[2][0],UInput[3][0],UInput[4][0],UInput[5][0],UInput[6][0],UInput[7][0],UInput[8][0],UInput[9][0],UInput[10][0],UInput[11][0],UInput[12][0],UInput[13][0],UInput[0][1],UInput[1][1],UInput[2][1],UInput[3][1],UInput[4][1],UInput[5][1],UInput[6][1],UInput[7][1],UInput[8][1],UInput[9][1],UInput[10][1],UInput[11][1],UInput[12][1],UInput[13][1]);
    printf("FPS: %d\t | IPS: %d\t | TotalRan: %d\t | CPU Run Quality: %.2lf%\n",FPS,CPU.IPC,CPU.TI,((double)CPU.IPC/12000000)*100,CPU.debug);
//    printf(">>>> %d\n",CPU.IPC/12000000);
   }
   CPU.IPC = 0;
   frames = 0;
  }
 }
}

void *CPU_EXEC(void *null) {
 bool     stopatloadrom   = false; //puts cpu in debug mode when BIOS has executed and the main rom is initialized
 bool     waitInput       = false;
 bool     SoftLoop        = false;
 bool     noUnicode       = false; //sets unicode to ascii to fix monospaced font from breaking DataDump()
 bool     noPrint         = false;
 bool     devInfo         = false;
 bool    *forceRender     = false;
 bool    *restart         = false;
 int      skip            = 0;
 uint16_t System_clock    = 0;
 char    *REG             = "ABCDEFGH********"; //* is used for if REG[index] is out of bounds to prevent a crash
 char     execLoc[1024];
 char    *FN = "";
 //char    *Title_Name;
 
 if (argc > 1 && dropped_filedir == "") { if (argv[1][0] == '-') { return 0; } }
 //printf("%d\n",argc);
 for (int i=1; i<argc; i++) {
  //printf("ARG[%d/%d]: %s\n",i,argc-1,argv[i]);
  if (!strcmp(argv[i],"--slow"         ) | !strcmp(argv[i],"-s"   )) { i++; slowdown = (int) strtol(argv[i], (char **)NULL, 10); }
  if (!strcmp(argv[i],"--debug"        ) | !strcmp(argv[i],"-d"   )) { CPU.debug     = true; }
  if (!strcmp(argv[i],"--pauseLoad"    ) | !strcmp(argv[i],"-pl"  )) { stopatloadrom = true; }
  if (!strcmp(argv[i],"--waitInput"    ) | !strcmp(argv[i],"-wi"  )) { waitInput     = true; }
  if (!strcmp(argv[i],"--speed"        ) | !strcmp(argv[i],"-sp"  )) { i++; skip = (int) strtol(argv[i], (char **)NULL, 10); }
  if (!strcmp(argv[i],"--showInfo"     ) | !strcmp(argv[i],"-i"   )) { showInfo      = true; }
  if (!strcmp(argv[i],"--noUnicode"    ) | !strcmp(argv[i],"-nu"  )) { noUnicode     = true; }
  if (!strcmp(argv[i],"--noPrint"      ) | !strcmp(argv[i],"-np"  )) { noPrint       = true; }
  if (!strcmp(argv[i],"--devInfo"      ) | !strcmp(argv[i],"-di"  )) { devInfo       = true; }
  if (!strcmp(argv[i],"--forceRender"  ) | !strcmp(argv[i],"-fr"  )) { forceRender   = true; }
 }
 if (CPU.debug == true) { printf("Debug Mode: Enabled\n"); } else { printf("Debug Mode: Disbaled\n"); }
 printf("\\Initialize Memory...\n");
 RAM  = calloc(RAMSIZ,  sizeof(*RAM));
 if (!RAM) { printf("MALLOC FAILED\n"); CPU.running = false; }
 VRAM = calloc(VRAMSIZ, sizeof(*VRAM));
 if (!VRAM){ printf("MALLOC FAILED\n"); CPU.running = false; }
 
 //initialize BP to the end of ram and SP to the same point
 //CPU.BP = RAMSIZ-1;
 //CPU.SP = CPU.BP;

 //zero reg A-H
 for (int i=0;i<8;i++) { CPU.REGs[i] = 0; }
 printf(" \\0x%x\\%d\tBytes(%d MB)\tof RAM were allocated...\n",RAMSIZ,RAMSIZ,(RAMSIZ/1024/1024)+1);
 printf(" \\0x%x\\%d\tBytes(%d MB)\tof VideoRAM was allocated...\n",VRAMSIZ,VRAMSIZ,(VRAMSIZ/1024/1024)+1);
 CPU.running = true; CPU.TI = 0;
 
 for (int i=0;i<strlen(argv[0])-16;i++) {
  execLoc[i] = argv[0][i];
 }
 char BN[1024];
 strcpy(BN,execLoc);
 strcat(BN,"bin/BIOS.tgr");
 FileStruct BIOS = load(BN,0);
 // FileStruct BIOS = load("./bin/simplebios.tgr",0);
 if (dropped_filedir == "") {
  if (argc < 2) {
   printf("\nEMU Notice: Requesting ROM path...\n\n");
   return 0;
  } else {
   FN = argv[1];
  }
 } else {
  FN = dropped_filedir;
 }
 //printf("%s\n",FN);
 FileStruct ROM = load(FN,0);
 if (ROM.found == false) { return 0; }
 if (ROM.size/1024/1024 >= 16) { printf("EMU ERROR: the ROM given was too big(%d MB/16 MB)",ROM.size/1024/1024); return 0; }
 
 char SN[strlen(FN)+1];
 crop(SN, FN, 0, strlen(FN)-4);
 //printf("%s\n",SN);
 strcat(SN, ".sav");
 //printf("%s",SN);
 FileStruct SAV = load(SN,1);
 
 printf("\n\\ROM Header info:\n \\Type: %d\n  \\System: %c%c%c\n",ROM.data[0],ROM.data[1],ROM.data[2],ROM.data[3]);
 printf("0\n");
 if (ROM.data[0] == 1) {
  printf("   \\Title: %c%c%c%c%c%c%c%c\n",                  ROM.data[4],ROM.data[5],ROM.data[6],ROM.data[7],ROM.data[8],ROM.data[9],ROM.data[10],ROM.data[11]);
  printf("0.25\n");
  printf("%s\n",Title_Name);
  //char* tmp = ;
  //for (int i=4;i<12;i++) { strcat(tmp,ROM.data[i]); }
  Title_lock = true;
  //Title_Name = "TheGameRazer - 12345678";
  snprintf(Title_Name, 1024, "TheGameRazer - %c%c%c%c%c%c%c%c",ROM.data[4],ROM.data[5],ROM.data[6],ROM.data[7],ROM.data[8],ROM.data[9],ROM.data[10],ROM.data[11]);
  Title_lock = false;
  //Title_Name = tmp;
  printf("0.75\n");
 }
 printf("1\n");
 dumpData("BIOS",BIOS,BIOSIZ,noUnicode,0,BIOSIZ);
 printf("2\n");
 dumpData("ROM" ,ROM ,ROMSIZ,noUnicode,0,ROMSIZ);
 printf("3\n");
// dumpData("SAV" ,SAV ,SAVSIZ,noUnicode,0,SAVSIZ);
 CPU.IS = BIOS.data; CPU.ISz = BIOS.size; CPU.IP = 0; CPU.SP = 0; CPU.BP = 0; CPU.reset = 0;
// free(RAM); free(VRAM);

 //MAIN LOOP
 CPU.SP = RAMSIZ;
 CPU.BP = RAMSIZ;
 clock_t Delay_clock = time(0);
 bool tmp_debug = CPU.debug;
 if (skip > 0) { CPU.debug = false; }
 CPU.IPC = 12000000; //### THIS IS HERE TO FIX A PROBBLEM WITH THE TOTALRAN COUNT ###
 while(CPU.running) {
  //printf(">>> tick!!!\n");
  if (CPU.reset == true) {
   GPU_reset();
   CPU.running = true;
   CPU.IP = 0;
   CPU.TI = 0;
   CPU.IPC = 0;
   CPU.IS = BIOS.data;
   CPU.ISz = BIOS.size;
   CPU.SP = RAMSIZ;
   CPU.BP = RAMSIZ;
   FPS = 0;
//   CPU_lock = false;
   System_clock = 0;
   for (int i=0;i<8;i++) { CPU.REGs[i] = 0; }
   bool tmp_debug = CPU.debug;
   CPU.IPC = 12000000; //### THIS IS HERE TO FIX A PROBBLEM WITH THE TOTALRAN COUNT ###
   CPU.reset = false;
   SDL_Delay(1000);
  }
  if (CPU.TI > skip && tmp_debug == 1) { CPU.debug = tmp_debug; tmp_debug = 0; }
  if (Exit == true) { break; }
//  while ((((double)CPU.IPC/12000000)*100) >= 100.0) { SDL_Delay(0); }
  uint8_t A   =  CPU.IS[CPU.IP+1] >> 4 ;       //4 \.
  uint8_t B   =  CPU.IS[CPU.IP+1] & 0xF;       //4 |-> A/B/C = 1.5 bytes
  uint8_t C   =  CPU.IS[CPU.IP+2] >> 4 ;       //4 /'
  int32_t IMM = (CPU.IS[CPU.IP+2] & 0xF) << 8; //4 \.
  IMM  = (IMM |  CPU.IS[CPU.IP+3]) << 8;       //8 |->  IMM  = 3.5 bytes
  IMM  = (IMM |  CPU.IS[CPU.IP+4]) << 8;       //8 |
  IMM |=         CPU.IS[CPU.IP+5];             //8 /'
  if (CPU.debug == true) {
   printf("\nIC: 0x%x/0x%x   \t>> [",CPU.IP,CPU.ISz-1);
   for (int i=0; i < 6; i++) { if (CPU.IS[CPU.IP+i] < 0x10) { printf("0x0%x",CPU.IS[CPU.IP+i]); } else { printf("0x%x",CPU.IS[CPU.IP+i]); } if (i < 5) { printf(", "); } }
   printf("] | [A:%c, B:%c, C:%c, IMM:0x",REG[A],REG[B],REG[C]);
   if        (IMM < 0x10) {       printf("000000%x]\n\\REGs: [",IMM);
   } else if (IMM < 0x100) {      printf("00000%x]\n\\REGs: [",IMM);
   } else if (IMM < 0x1000) {     printf("0000%x]\n\\REGs: [",IMM);
   } else if (IMM < 0x10000) {    printf("000%x]\n\\REGs: [",IMM);
   } else if (IMM < 0x100000) {   printf("00%x]\n\\REGs: [",IMM);
   } else if (IMM < 0x1000000) {  printf("0%x]\n\\REGs: [",IMM);
   } else if (IMM < 0x10000000) { printf("%x]\n\\REGs: [",IMM); }
   for (int i=0; i < 8; i++) {
    if        (CPU.REGs[i] < 0x10) {    printf("%c:0x000%x",REG[i],CPU.REGs[i]);
    } else if (CPU.REGs[i] < 0x100) {   printf("%c:0x00%x",REG[i],CPU.REGs[i]);
    } else if (CPU.REGs[i] < 0x1000) {  printf("%c:0x0%x",REG[i],CPU.REGs[i]);
    } else if (CPU.REGs[i] < 0x10000) { printf("%c:0x%x",REG[i],CPU.REGs[i]);
    } if (i < 7) { printf(", "); }
   }
   printf("] | TotalRan: %d\n\\RAMPOS: 0x%x/%d | StackPointer: 0x%x/%d | StackBase: 0x%x/%d\n\\\\StackData:[",CPU.TI,CPU.RP,CPU.RP,CPU.SP,CPU.SP,CPU.BP,CPU.BP);
   for (int i = CPU.SP+1; i <= CPU.BP; ++i){
     if((i+1)%2==0){
        printf(" 0x");
      }
      printf("%02x",RAM[i]);
      if(i%16==0 && i != 0){
        printf("\n");
      }
   }

   printf("]\n \\instruction: "); //,CPU.REGs[0],CPU.REGs[1],CPU.REGs[2],CPU.REGs[3],CPU.REGs[4],CPU.REGs[5],CPU.REGs[6],CPU.REGs[7]);
  }


//  printf("IC:0x%x/%d | 0x%x\n",CPU.IP,CPU.IP,CPU.IS[CPU.IP]);
  switch(CPU.IS[CPU.IP]) {
   case 0x00:
    if (CPU.debug == true) { printf("LOAD\n"); }     //|+|-|-|0x---XXXX|REG[A] = IMM                                                                               |
    CPU.REGs[A] = IMM % (0x10000);
    break;
   case 0x01:
    if (CPU.debug == true) { printf("ADD\n"); }      //|+|+|+|0x---XXXX|if IMM > 0 then replace B with IMM  |  C = A + B/IMM                                       |
    if (IMM == 0) {
     //printf("[REG:%c]0x%x + [REG:%c]0x%x = [REG:%c]0x%x\n",REG[A],CPU.REGs[A],REG[B],CPU.REGs[B],REG[C],CPU.REGs[A] + CPU.REGs[B]);
     CPU.REGs[C] = CPU.REGs[A] + CPU.REGs[B];
    } else {
     //printf("[REG:%c]0x%x + [IMM]0x%x = [REG:%c]0x%x\n",REG[A],CPU.REGs[A],IMM % 0x10000,REG[C],CPU.REGs[A] + (IMM % 0x10000));
     CPU.REGs[C] = CPU.REGs[A] + (IMM % 0x10000);
    }
    break;
   case 0x02:
    if (CPU.debug == true) { printf("SUB\n"); }      //|+|+|+|0x---XXXX|if IMM > 0 then replace B with IMM  |  C = A - B/IMM                                       |
    if (IMM == 0) {
     CPU.REGs[C] = CPU.REGs[A] - CPU.REGs[B];
    } else {
     CPU.REGs[C] = CPU.REGs[A] - (IMM % 0x10000);
    }
    break;
   case 0x03:
    if (CPU.debug == true) { printf("MUL\n"); }      //|+|+|+|0x---XXXX|if IMM > 0 then replace B with IMM  |  C = A * B/IMM                                       |
    if (IMM == 0) {
     CPU.REGs[C] = CPU.REGs[A] * CPU.REGs[B];
    } else {
     CPU.REGs[C] = CPU.REGs[A] * (IMM % 0x10000);
    }
    break;
   case 0x04:
    if (CPU.debug == true) { printf("DIV\n"); }      //|+|+|+|0x---XXXX|if IMM > 0 then replace B with IMM  |  C = A / B/IMM                                       |
    if (IMM == 0) {
     if (CPU.REGs[B] == 0) {
      printf("EMU Error: try'd to divide by 0...\n");
      CPU.REGs[C] = 0;
      CPU.flag[4] = 1;
     } else {
      CPU.REGs[C] = CPU.REGs[A] / CPU.REGs[B];
     }
    } else {
     CPU.REGs[C] = CPU.REGs[A] / (IMM % 0x10000);
    }
    break;
   case 0x05:
    if (CPU.debug == true) { printf("AND\n"); }     //|+|+|+|0x---XXXX|if IMM > 0 then replace B with IMM  |  C = A &AND B/IMM                                     |
    if (IMM == 0) {
     CPU.REGs[C] = CPU.REGs[A] & CPU.REGs[B];
    } else {
     CPU.REGs[C] = CPU.REGs[A] & (IMM % 0x10000);
    }
    break;
   case 0x06:
    if (CPU.debug == true) { printf("OR\n"); }      //|+|+|+|0x---XXXX|if IMM > 0 then replace B with IMM  |  C = A |OR  B/IMM                                     |
    if (IMM == 0) {
     CPU.REGs[C] = CPU.REGs[A] | CPU.REGs[B];
    } else {
     CPU.REGs[C] = CPU.REGs[A] | (IMM % 0x10000);
    }
    break;
   case 0x07:
    if (CPU.debug == true) { printf("XOR\n"); }     //|+|+|+|0x---XXXX|if IMM > 0 then replace B with IMM  |  C = A ^XOR B/IMM                                     |
    if (IMM == 0) {
     CPU.REGs[C] = CPU.REGs[A] ^ CPU.REGs[B];
    } else {
     CPU.REGs[C] = CPU.REGs[A] ^ (IMM % 0x10000);
    }
    break;
   case 0x08:
    if (CPU.debug == true) { printf("NOT\n"); }     //|+|+|-|0x-------|B = NOT A                                                                                   |
    CPU.REGs[B] = ~CPU.REGs[A];
    break;
   case 0x09:
    if (CPU.debug == true) { printf("SPLIT\n"); }   //|+|+|+|0x------X|B,C = A splitted | if IMM == 0: 8-bit split else 4-bit split                                |
    if ((IMM % 0x10) == 0) {
     CPU.REGs[B] = CPU.REGs[A] & 0xFF;
     CPU.REGs[C] = CPU.REGs[A] >> 8;
    } else {
     CPU.REGs[B] = CPU.REGs[A] & 0xF;
     CPU.REGs[C] = CPU.REGs[A] >> 4;
    }
//    printf(">>>>>>  A:0x%x >> B:0x%x, C:0x%x (SPLITED)  <<<<<<\n",CPU.REGs[A],CPU.REGs[B],CPU.REGs[C]);
    break;
   case 0x0A:
    if (CPU.debug == true) { printf("COMBINE\n"); } //|+|+|+|0x------X|C = A combined with B | IMM rules same as SPLIT                                             |
    if ((IMM % 0x10) == 0) {
     CPU.REGs[C] = (CPU.REGs[A] << 8) | (CPU.REGs[B] & 0xFF);
    } else {
     CPU.REGs[C] = (CPU.REGs[A] << 4) | (CPU.REGs[B] & 0xF);
    }
//    printf(">>>>>>  A:0x%x, B:0x%x >> C:0x%x (COMBINE)  <<<<<<\n",CPU.REGs[A],CPU.REGs[B],CPU.REGs[C]);
    break;
   case 0x0B:
    if (CPU.debug == true) { printf("JMP\n\n"); }     //|+|+|/|0x-XXXXXX|if C == 1 then IC = A..B else IC = IMM                                                     |
    CPU.IP = ((CPU.REGs[A] << 16) | CPU.REGs[B])-6;
    break;
   case 0x0C:
    if (CPU.debug == true) { printf("JMPIMM\n\n"); }
    CPU.IP = (IMM % 0x1000000)-6;
    break;
   case 0x0D:
    if (CPU.debug == true) { printf("CMP=: "); }    //|+|+|-|0x---XXXX|if A == B: IC=IC+1(expected JMP) else IC=IC+2                                               |
    if (C == 0) { if (CPU.REGs[A] == CPU.REGs[B]) { if (CPU.debug == true) {  printf("True\n");} } else { if (CPU.debug == true) { printf("False\n");} CPU.IP = CPU.IP + 6; }
    } else {      if (CPU.REGs[A] == IMM%0x10000) { if (CPU.debug == true) {  printf("True\n");} } else { if (CPU.debug == true) { printf("False\n");} CPU.IP = CPU.IP + 6; }
    } break;
   case 0x0E:
    if (CPU.debug == true) { printf("CMP<: "); }    //|+|+|-|0x-------|if A < B: IC=IC+1(expected JMP) else IC=IC+2                                                |
    if (C == 0) { if (CPU.REGs[A] <  CPU.REGs[B]) { if (CPU.debug == true) {  printf("True\n");} } else { if (CPU.debug == true) { printf("False\n");} CPU.IP = CPU.IP + 6; }
    } else {      if (CPU.REGs[A] <  IMM%0x10000) { if (CPU.debug == true) {  printf("True\n");} } else { if (CPU.debug == true) { printf("False\n");} CPU.IP = CPU.IP + 6; }
    if (CPU.REGs[A] < CPU.REGs[B]) { if (CPU.debug == true) {  printf("True\n");} } else { if (CPU.debug == true) { printf("False\n");} CPU.IP = CPU.IP + 6; }
    } break;
   case 0x0F:
    if (CPU.debug == true) { printf("CMP>: "); }    //|+|+|-|0x-------|if A > B: IC=IC+1(expected JMP) else IC=IC+2                                                |
    if (C == 0) { if (CPU.REGs[A]  > CPU.REGs[B]) { if (CPU.debug == true) {  printf("True\n");} } else { if (CPU.debug == true) { printf("False\n");} CPU.IP = CPU.IP + 6; }
    } else {      if (CPU.REGs[A]  > IMM%0x10000) { if (CPU.debug == true) {  printf("True\n");} } else { if (CPU.debug == true) { printf("False\n");} CPU.IP = CPU.IP + 6; }
    } break;
   case 0x10:
    if (CPU.debug == true) { printf("RAMPOS\n"); }  //|+|+|/|0xXXXXXXX|RAMPointer = IMM                                                                            |
    if (C == 1) {
     CPU.RP = IMM;
    } else {
     CPU.RP = ((CPU.REGs[A] << 16) | CPU.REGs[B]) % 0x7FFFFFF;
    }
    break;
   case 0x11:
    if (CPU.debug == true) { printf("RRAM\n"); }    //|+|/|/|0xXXXXXXX|if B == 1 then use VRAM instead of RAM / if C == 1 then IMM will replace with RAMPointer    |
    if (B == 1) {
     if (C == 1) {
      if (CPU.debug == true) { printf("VRAM: Reading RAM[0x%x] to REG:%c\n",IMM    % 0x3FFFFFF,REG[A]); }
      CPU.REGs[A] = VRAM[IMM % 0x3FFFFFF];
     } else {
      if (CPU.debug == true) { printf("VRAM: Reading RAM[0x%x] to REG:%c\n",CPU.RP % 0x3FFFFFF,REG[A]); }
      CPU.REGs[A] = VRAM[CPU.RP % 0x3FFFFFF];
     }
    } else {
     if (C == 1) {
      if (CPU.debug == true) { printf("RAM: Reading RAM[0x%x] to REG:%c\n",IMM    % 0x7FFFFFF,REG[A]); }
      CPU.REGs[A] = RAM[IMM % 0x7FFFFFF];
     } else {
      if (CPU.debug == true) { printf("RAM: Reading RAM[0x%x] to REG:%c\n",CPU.RP % 0x7FFFFFF,REG[A]); }
      CPU.REGs[A] = RAM[CPU.RP % 0x7FFFFFF];
     }
    }
    break;
   case 0x12:
    if (CPU.debug == true) { printf("WRAM\n"); }    //|+|/|/|0xXXXXXXX|if B == 1 then index VRAM else RAM / if C == 1 then IMM will replace with RAMPointer        |
    if (B == 1) {
     if (C == 1) {
      if (CPU.debug == true) { printf("VRAM: Writing REG:%c to RAM[0x%x]\n",REG[A],IMM    % 0x3FFFFFF); }
      VRAM[IMM    % 0x3FFFFFF] = CPU.REGs[A];
     } else {
      if (CPU.debug == true) { printf("VRAM: Writing REG:%c to RAM[0x%x]\n",REG[A],CPU.RP % 0x3FFFFFF); }
      VRAM[CPU.RP % 0x3FFFFFF] = CPU.REGs[A];
     }
    } else {
     if (C == 1) {
      if (CPU.debug == true) { printf("RAM: Writing REG:%c to RAM[0x%x]\n",REG[A],IMM    % 0x7FFFFFF); }
      RAM[IMM    % 0x7FFFFFF] = CPU.REGs[A];
     } else {
      if (CPU.debug == true) { printf("RAM: Writing REG:%c to RAM[0x%x]\n",REG[A],CPU.RP % 0x7FFFFFF); }
      RAM[CPU.RP % 0x7FFFFFF] = CPU.REGs[A];
     }
    }
    break;
   case 0x13:
    if (CPU.debug == true) { printf("RSAV\n"); }    //|+|+|+|0x-------|C = SAV.data[A..B]                                                                          |
    CPU.REGs[C] = SAV.data[(CPU.REGs[A] << 16) | CPU.REGs[B]];
    break;
   case 0x14:
    if (CPU.debug == true) { printf("IRSAV\n"); }   //|+|-|-|0x-XXXXXX|A = SAV.data[IMM]                                                                           |
    CPU.REGs[A] = (SAV.data[IMM] % 0x1000000);
    break;
   case 0x15:
    if (CPU.debug == true) { printf("WSAV\n"); }    //|+|+|+|0x------X|SAV.data[A..B] = C unless if IMM == 1 then push SAV to file                                     |
    if (IMM%2 == 1) {
     if (access(SN, F_OK) == -1) {
      printf("EMU NOTTICE: file \"%s\" was not found, generating file...", SN);
     }
     FILE *SAV_file = fopen(SN, "wb");
     rewind(SAV_file);
     fwrite(SAV.data, 1, SAVSIZ, SAV_file);
     fclose(SAV_file);
     //printf("MAXsize: %d | SAVsize: %d\n",SAV.size, sizeof(SAV.data));
     if (CPU.debug == true) { printf("EMU SAV: SAV data was saved to file \"%s\"...\n",SN); }
    } else {
     SAV.data[(CPU.REGs[A] << 16) | CPU.REGs[B]] = CPU.REGs[C];
     //printf("SAV[0x%x]: 0x%x\nMAXsize: %d | SAVsize: %d\n",(CPU.REGs[A] << 16) | CPU.REGs[B],SAV.data[(CPU.REGs[A] << 16) | CPU.REGs[B]],SAV.size, sizeof(SAV.data));
    }
    break;
   case 0x16:
    if (CPU.debug == true) { printf("IWSAV\n"); }   //|+|-|-|0x-XXXXXX|SAV.data[IMM] = A                                                                           |
    SAV.data[IMM] = (CPU.REGs[A] % 0x1000000);
    break;
   case 0x17:
    if (CPU.debug == true) { printf("RROM\n"); }    //|+|+|+|0x-------|C = ROM.data[A..B]                                                                          |
    CPU.REGs[C] = ROM.data[((CPU.REGs[A] << 16) | CPU.REGs[B]) % CPU.ISz];
//    printf(">>>>>>  Adr:0x%x/%d, data:0x%x (ReadROM)  <<<<<<\n",(((CPU.REGs[A] << 16) % 0x1000) | CPU.REGs[B])%CPU.ISz,(((CPU.REGs[A] << 16) % 0x1000) | CPU.REGs[B])%CPU.ISz, CPU.REGs[C]);
    break;
   case 0x18:
    if (CPU.debug == true) { printf("IRROM\n"); }   //|+|-|-|0x-XXXXXX|A = ROM.data[IMM]                                                                           |
    CPU.REGs[A] = ROM.data[IMM % CPU.ISz];
//    printf(">>>>>>  Adr:0x%x/%d, data:0x%x (ImmReadROM)  <<<<<<\n",IMM % CPU.ISz,IMM % CPU.ISz, CPU.REGs[A]);
    break;
   case 0x19:
    if (CPU.debug == true) { printf("HALT\n"); }    //|-|-|-|0x------X|halts index IMM located in [[HALT-INFO]]                                                    |
    switch(IMM) {
     case 0x0: //|nothing                                 |N/A        |
      break;
     case 0x1: //|stop running everything                 |CPU        |
      printf("EMU: CPU HATLTED...\n");
      CPU.running = 0;
      break;
     case 0x2: //|resets the Audio                        |AUDIO      |
      break;
     case 0x3: //|resets the Input                        |INPUT      |
      //            A B C X Y Z L R S s ^ v < >
      //input.P1 = [0,0,0,0,0,0,0,0,0,0,0,0,0,0]
      //input.P2 = [0,0,0,0,0,0,0,0,0,0,0,0,0,0]
      break;
     case 0x4: //|resets Network System                   |NETWORK    |
      break;
     case 0x5: //|stops everything and prints IP          |CPU        |
      printf("EMU: CPU HATLTED at InstructionPointer 0x%x/%d...\n",CPU.IP,CPU.IP);
      CPU.running = 0;
      break;
     case 0x7: //|nothing                                 |N/A        |
      break;
     case 0x8: //|nothing                                 |N/A        |
      break;
     case 0x9: //|nothing                                 |N/A        |
      break;
     case 0xA: //|nothing                                 |N/A        |
      break;
     case 0xB: //|nothing                                 |N/A        |
      break;
     case 0xC: //|nothing                                 |N/A        |
      break;
     case 0xD: //|nothing                                 |N/A        |
      break;
     case 0xE: //|nothing                                 |N/A        |
      break;
     case 0xF: //|resets everything(restarts the emu)     |EMU        |
      CPU.IS  = BIOS.data;
      CPU.ISz = BIOS.size;
      CPU.IP  = 0; CPU.IPC = 0; CPU.TI  = 0;
      break;
    } break;
   case 0x1a:
    if (CPU.debug == true) { printf("PRINT\n"); }   //|+|-|-|0x-------|prints A in emulator terminal(for debugging ROMs only)                                      |
    if (noPrint == false) {
     if (CPU.REGs[A] > 9999) {
      printf(">> %d\t(0x%x)\n",CPU.REGs[A],CPU.REGs[A]);
     } else {
      printf(">> %d\t\t(0x%x)\n",CPU.REGs[A],CPU.REGs[A]);
     }
    } break;
   case 0x1b:
    if (CPU.debug == true) { printf("FLAGS\n"); }   //|+|-|-|0x------X|A = Flags[IMM]                                                                              |
    CPU.REGs[A] = CPU.flag[IMM];
    break;
   case 0x1c:
    if (CPU.debug == true) { printf("DVCSEND\n"); } //|+|-|-|0x----XXX|send message to device IMM[3], IMM[1-2] are for device Instruction                          |
    //Component[(IMM >> 8) % 0x10].recv(IMM & 0xFF, CPU.REGs[A])
    switch ((IMM >> 8) % 0x10) {
     case 0x0: //GPU
      GPU_send(IMM & 0xFF, CPU.REGs[A]);
      break;
     case 0x1: //INPUT
      INPUT_send(IMM & 0xFF, CPU.REGs[A]);
      break;
     case 0x2: //SOUND
      //_send(IMM & 0xFF, A);
      break;
     case 0x3: //NETWORK
      //_send(IMM & 0xFF, A);
      break;
     case 0x4: //
      //_send(IMM & 0xFF, A);
      break;
     case 0x5: //
      //_send(IMM & 0xFF, A);
      break;
     case 0x6: //
      //_send(IMM & 0xFF, A);
      break;
     case 0x7: //
      //_send(IMM & 0xFF, A);
      break;
     case 0x8: //
      //_send(IMM & 0xFF, A);
      break;
     case 0x9: //
      //_send(IMM & 0xFF, A);
      break;
     case 0xA: //
      //_send(IMM & 0xFF, A);
      break;
     case 0xB: //
      //_send(IMM & 0xFF, A);
      break;
     case 0xC: //
      //_send(IMM & 0xFF, A);
      break;
     case 0xD: //
      //_send(IMM & 0xFF, A);
      break;
     case 0xE: //
      //_send(IMM & 0xFF, A);
      break;
     case 0xF: //
      //_send(IMM & 0xFF, A);
      break;
    }
    break;
   case 0x1d:
    if (CPU.debug == true) { printf("DVCRECV\n"); } //|+|-|-|0x----XXX|same as DVCSEND but recive then send                                                        |
    //CPU.REGs[A] = Component[(IMM >> 8) % 0x10].recv(IMM & 0xFF)
//    printf("nom:%x\n8>>:%x\n",IMM,IMM >> 8);
    switch ((IMM >> 8) % 0x10) {
     case 0x0: //GPU
      CPU.REGs[A] = GPU_recv(IMM & 0xFF);
      break;
     case 0x1: //INPUT
      CPU.REGs[A] = INPUT_recv(IMM & 0xFF);
      break;
     case 0x2: //SOUND
      //CPU.REGs[A] = _recv(IMM & 0xFF, A);
      break;
     case 0x3: //NETWORK
      //CPU.REGs[A] = _recv(IMM & 0xFF, A);
      break;
     case 0x4: //
      //CPU.REGs[A] = _recv(IMM & 0xFF, A);
      break;
     case 0x5: //
      //CPU.REGs[A] = _recv(IMM & 0xFF, A);
      break;
     case 0x6: //
      //CPU.REGs[A] = _recv(IMM & 0xFF, A);
      break;
     case 0x7: //
      //CPU.REGs[A] = _recv(IMM & 0xFF, A);
      break;
     case 0x8: //
      //CPU.REGs[A] = _recv(IMM & 0xFF, A);
      break;
     case 0x9: //
      //CPU.REGs[A] = _recv(IMM & 0xFF, A);
      break;
     case 0xA: //
      //CPU.REGs[A] = _recv(IMM & 0xFF, A);
      break;
     case 0xB: //
      //CPU.REGs[A] = _recv(IMM & 0xFF, A);
      break;
     case 0xC: //
      //CPU.REGs[A] = _recv(IMM & 0xFF, A);
      break;
     case 0xD: //
      //CPU.REGs[A] = _recv(IMM & 0xFF, A);
      break;
     case 0xE: //
      //CPU.REGs[A] = _recv(IMM & 0xFF, A);
      break;
     case 0xF: //
      //CPU.REGs[A] = _recv(IMM & 0xFF, A);
      break;
    }
    break;
   case 0x1e:
    if (CPU.debug == true) { printf("ICOUT\n"); }   //|+|+|-|0x-------|A,B = IC(32-bit for 24-bit)                                                                 |
    CPU.REGs[A] = CPU.IP & 0xFF;
    CPU.REGs[B] = CPU.IP >> 16;
    break;
   case 0x1f:
    if (CPU.debug == true) { printf("COPY\n"); }    //|+|+|-|0x-------|A = B                                                                                       |
    CPU.REGs[B] = CPU.REGs[A];
    break;
   case 0x20:
    if (CPU.debug == true) { printf("EXECUTE\n"); } //|-|-|-|0xXXXXXXX|execute location = IMM[6] and IMM[0-5]                                                      |
	//SDL_Delay(10000);
    CPU.IP = IMM-6;
	  printf("EMU NOTICE: ");
    switch(A) {
     case 0x0: //executes BIOS
      printf("Switching to BIOS execute");
      CPU.IS  = BIOS.data;
      CPU.ISz = BIOS.size;
      break;
     case 0x1: //executes ROM
      if (stopatloadrom) {
       printf("**press enter**\n");//EMU: DebugMode Enabled...\n
       //CPU.debug = true, 
       stopatloadrom = false;
       getchar();
      }
      printf("Switching to ROM execute");
      CPU.IS  = ROM.data;
      CPU.ISz = ROM.size;
      break;
     case 0x2: //executes RAM
      printf("Switching to RAM execute");
      CPU.IS  = RAM;
      CPU.ISz = sizeof(RAM);
      break;
    }
	  printf("... [PC:0x%x]\n",CPU.IP);
	  //if (CPU.debug == true) { printf(" at 0x%x/%x...\n",CPU.IP+6,CPU.ISz); } else { printf("...\n"); }
    break;
   case 0x21:
    if (CPU.debug == true) { printf("RBIOS\n"); }    //|+|+|+|0x-------|C = BIOS.data[A..B]                                                                         |
    CPU.REGs[C] = BIOS.data[(CPU.REGs[A] << 16) | CPU.REGs[B]];
    break;
   case 0x22:
    if (CPU.debug == true) { printf("IRBIOS\n"); }   //|-|-|-|0xXXXXXXX|C = BIOS.data[IMM]                                                                          |
    CPU.REGs[C] = IMM % CPU.ISz;
    break;
   case 0x23:
    if (CPU.debug == true) { printf("POP "); }
    if (CPU.SP+1 < CPU.BP){
      CPU.SP+=2;
    } else {
      printf("PANIC! stack empty\n");
      CPU.running = false;
    }
    CPU.REGs[A] = (uint16_t)RAM[CPU.SP] | (((uint16_t)RAM[CPU.SP-1])<<8);
    if (CPU.debug == true) { printf("%i\n",CPU.REGs[A]);}
    break;
   case 0x24:
    if (CPU.debug == true) { printf("PUSH "); }
    if (B == 1) {
     RAM[CPU.SP--] = (uint8_t)(IMM & 0xff);
     RAM[CPU.SP--] = (uint8_t)((IMM >> 8)&0xff);
     if (CPU.debug == true) { printf("%i\n",IMM&0x10000); }
    } else {
     RAM[CPU.SP--] = (uint8_t)(CPU.REGs[A] & 0xff);
     RAM[CPU.SP--] = (uint8_t)(CPU.REGs[A] >> 8);
     if (CPU.debug == true) { printf("%i\n",CPU.REGs[A]); }
    }
    break;
   case 0x25:
    if (CPU.debug == true) { printf("CALL "); }
    //printf("CALLING FUNCTION AT: PC:0x%x | TO:0x%x\n",CPU.IP,IMM % 0x1000000);
    RAM[CPU.SP--] = (uint8_t)(CPU.IP      );// CPU.SP--;
    RAM[CPU.SP--] = (uint8_t)(CPU.IP >>  8);// CPU.SP--;
    RAM[CPU.SP--] = (uint8_t)(CPU.IP >> 16);// CPU.SP--;
    RAM[CPU.SP--] = (uint8_t)(CPU.IP >> 32);// CPU.SP--;
    if (CPU.debug == true) { printf("%i\n",CPU.IP);}
    CPU.IP = (IMM % 0x1000000)-6;
    /*printf("StackData:[");
    for (int i = CPU.SP+1; i <= CPU.BP; ++i) {
     printf(" 0x%02x",RAM[i]);
     if(i%16==0 && i != 0){
      printf("\n");
     }
    } printf("]\n[AB:0x%x%x]\n",CPU.REGs[0],CPU.REGs[1]);*/
    break;
   case 0x26:
    if (CPU.debug == true) { printf("RET "); }
    if (CPU.SP+3 < CPU.BP){
      CPU.SP+=4;
    } else {
      printf("EMU: PANIC! stack empty\n");
      Exit = true;
    }
    CPU.IP = RAM[CPU.SP] | ((RAM[CPU.SP-1])<<8) | ((RAM[CPU.SP-2])<<16) | ((RAM[CPU.SP-3])<<32);
	/*printf("RETURNING FROM FUNCTION AT: PC:0x%x | TO:0x%x\n",CPU.IP,RAM[CPU.SP] | ((RAM[CPU.SP-1])<<8) | ((RAM[CPU.SP-2])<<16) | ((RAM[CPU.SP-3])<<32));
    if (CPU.debug == true) { printf("%i\n",CPU.IP);}
    printf("StackData:[");
    for (int i = CPU.SP+1; i <= CPU.BP; ++i){
     printf(" 0x%02x",RAM[i]);
     if(i%16==0 && i != 0) {
      printf("\n");
     }
    } printf("]\n[AB:0x%x%x]\n",CPU.REGs[0],CPU.REGs[1]);*/
    break;
   case 0x27:
    if (CPU.debug == true) { printf("SWAPTOP "); }
    if (CPU.SP+1 < CPU.BP){
      CPU.SP+=2;
    } else {
      printf("EMU: PANIC! stack empty\n");
      Exit = true;
    }
    uint16_t tmp1 = (uint16_t)RAM[CPU.SP] | (((uint16_t)RAM[CPU.SP-1])<<8);
    if (CPU.SP+1 < CPU.BP) {
      CPU.SP+=2;
    } else {
      printf("EMU: PANIC! stack empty\n");
      Exit = true;
    }
    uint16_t tmp2 = (uint16_t)RAM[CPU.SP] | (((uint16_t)RAM[CPU.SP-1])<<8);
    RAM[CPU.SP--] = (uint8_t)(tmp1 & 0xff);
//    CPU.SP--;
    RAM[CPU.SP--] = (uint8_t)(tmp1 >> 8);
//    CPU.SP--;
    RAM[CPU.SP--] = (uint8_t)(tmp2 & 0xff);
//    CPU.SP--;
    RAM[CPU.SP--] = (uint8_t)(tmp2 >> 8);
//    CPU.SP--;
    break;
   case 0x28:
    if (B == 0) {
     if (CPU.debug == true) { printf("GCLK: %d\n",System_clock); }
      CPU.REGs[A] = CPU.time;
    } else {
     if (CPU.debug == true) { printf("GCLK: resetting...\n"); }
     CPU.time = 0;// Tick_clock = time(0);
     CPU.REGs[A] = 0;
    }
//    if (CPU.debug == true) { printf("SCLK: returning SystemClock...\n"); }
//    CPU.REGs[A] = CPU.time;
    break;
   case 0x29:
    if (CPU.debug == true || devInfo == true) {
     printf("Debug Delay: %dms.\n",IMM);
     if (IMM == 0) {
      if (devInfo == true) {
       printf("\DevInfo: TotalRan: %ld\n",CPU.TI);
      }
      if (waitInput == false) {
       printf("/**press enter**\n");
       getchar();
      }
     } else {
      SDL_Delay(IMM);
     }
    }
    break;
   case 0x2A:

    break;
   case 0x2B:

    break;
   case 0x2C:

    break;
   case 0x2D:

    break;
   case 0x2E:

    break;
   case 0x2F:

    break;
   case 0xFF:
    if (CPU.debug == true) { printf("NOP\n"); }  //|+|+|/|0xXXXXXXX|RAMPointer = IMM                                                                            |
	break;
   default:
    if (CPU.debug == true) { printf("UNKNOWN\n  \\"); }
    System_Error( 0, CPU.IS[CPU.IP], CPU.IP, -1, "CPU");
    CPU.running = 0;
  }
  CPU.IP = CPU.IP + 6;
  CPU.IPC++;
  CPU.TI++;
  if (CPU.IP > CPU.ISz) { 
   if (SoftLoop == 1) {
    CPU.IP = 0;
   } else {
    System_Error(1, CPU.IS[CPU.IP], CPU.IP, -1, "CPU");
    CPU.running = 0;
   }
  }
  //SDL_Delay(1000/(12000000/CPU.IPC));
  if (waitInput == true && CPU.debug == true && CPU.IS != BIOS.data) {
   printf("**press enter**\n");
   getchar();
  } else {
   //SDL_Delay(i/1000);
   if (stopatloadrom == false && CPU.TI > skip) {
    if (slowdown > 0 && slowdown < 12000000 && CPU.IPC/(12000000-slowdown) > 0) {
     SDL_Delay(CPU.IPC/(12000000-slowdown));
    } else if (CPU.IPC/(12000000-832) > 0) {
     //printf("%d\n",CPU.IPC-12000000);
     SDL_Delay(CPU.IPC/(12000000-832));
    }
   }
  }
 }
 //Exit = true;
 return NULL;
}