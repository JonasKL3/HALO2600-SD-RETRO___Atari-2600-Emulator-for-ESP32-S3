#ifndef EMUAPI_H
#define EMUAPI_H

#define INVX 1
#define TITLE "Atari 2600 Emulator"
#define ROMSDIR "2600"

#define VID_FRAME_SKIP 0x0
#define PALETTE_SIZE 256
#define SINGLELINE_RENDERING 1
#define TFT_VBUFFER_YCROP 0

#define ACTION_NONE 0
#define ACTION_MAXKBDVAL 4
#define ACTION_EXITKBD 128
#define ACTION_RUNTFT 129

#define MASK_JOY2_RIGHT 0x0001
#define MASK_JOY2_LEFT  0x0002
#define MASK_JOY2_UP    0x0004
#define MASK_JOY2_DOWN  0x0008
#define MASK_JOY2_BTN   0x0010
#define MASK_KEY_USER1  0x0020
#define MASK_KEY_USER2  0x0040
#define MASK_KEY_USER3  0x0080
#define MASK_JOY1_RIGHT 0x0100
#define MASK_JOY1_LEFT  0x0200
#define MASK_JOY1_UP    0x0400
#define MASK_JOY1_DOWN  0x0800
#define MASK_JOY1_BTN   0x1000
#define MASK_KEY_USER4  0x2000

#ifdef __cplusplus
extern "C" {
#endif

int emu_init(void);
void emu_SetDabbleKeys(unsigned short keys);
void emu_printf(char *text);
void emu_printi(int val);
void *emu_Malloc(int size);
void emu_Free(void *pt);

int emu_FileOpen(char *filename);
int emu_FileRead(char *buf, int size);
unsigned char emu_FileGetc(void);
int emu_FileSeek(int seek);
void emu_FileClose(void);
int emu_FileSize(char *filename);
int emu_LoadFile(char *filename, char *buf, int size);
int emu_LoadFileSeek(char *filename, char *buf, int size, int seek);

void emu_InitJoysticks(void);
int emu_SwapJoysticks(int statusOnly);
unsigned short emu_DebounceLocalKeys(void);
int emu_ReadKeys(void);
int emu_GetPad(void);
int emu_ReadAnalogJoyX(int min, int max);
int emu_ReadAnalogJoyY(int min, int max);
int emu_ReadI2CKeyboard(void);
int emu_setKeymap(int index);

void emu_sndInit(void);
void emu_sndPlaySound(int chan, int volume, int freq);
void emu_sndPlayBuzz(int size, int val);

void emu_SetPaletteEntry(unsigned char r, unsigned char g, unsigned char b, int index);
void emu_DrawScreen(unsigned char *VBuf, int width, int height, int stride);
void emu_DrawLine(unsigned char *VBuf, int width, int height, int line);
void emu_DrawVsync(void);
int emu_FrameSkip(void);
void *emu_LineBuffer(int line);

#ifdef __cplusplus
}
#endif

#endif
