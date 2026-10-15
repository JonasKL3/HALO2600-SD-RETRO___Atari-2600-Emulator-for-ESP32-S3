#include <Arduino.h>
#include <SD.h>
#include <TFT_eSPI.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <esp_heap_caps.h>

#include "emuapi.h"

extern TFT_eSPI tft;

static uint16_t palette16[PALETTE_SIZE];
static uint16_t *frame565 = nullptr;
static uint16_t dabbleKeys = 0;
static uint16_t previousKeys = 0;
static int frameSkip = 0;
static fs::File lastFile;

#define SCREEN_W 320
#define SCREEN_H 240
#define GAME_W 160
#define GAME_H 192
#define GAME_Y 24

static uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
  return (uint16_t)(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
}

static const char *resolvePath(char *filename) {
  if (!filename || !filename[0]) return nullptr;
  if (filename[0] == '/') return filename;
  static char path[224];
  snprintf(path, sizeof(path), "/%s", filename);
  return path;
}

extern "C" {

int emu_init(void) {
  dabbleKeys = 0;
  previousKeys = 0;
  frameSkip = 0;
  memset(palette16, 0, sizeof(palette16));

  if (!frame565) {
    frame565 = (uint16_t *)heap_caps_malloc(320 * 192 * sizeof(uint16_t),
                                            MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!frame565) {
      Serial.println("[PSRAM] Falha framebuffer na PSRAM; tentando heap interno.");
      frame565 = (uint16_t *)malloc(320 * 192 * sizeof(uint16_t));
    } else {
      Serial.printf("[PSRAM] Framebuffer: %u bytes.\n", 320u * 192u * 2u);
    }
  }

  if (!frame565) {
    Serial.println("[MEM] ERRO: framebuffer nao alocado.");
    return 0;
  }

  Serial.println("EMU API: SD + TFT_eSPI + Dabble");
  return 1;
}

void emu_SetDabbleKeys(unsigned short keys) { dabbleKeys = keys; }
void emu_printf(char *text) { if (text) Serial.println(text); }
void emu_printi(int val) { Serial.println(val); }

void *emu_Malloc(int size) {
  void *p = nullptr;
  if (psramFound()) p = heap_caps_malloc((size_t)size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  if (!p) p = malloc((size_t)size);
  return p;
}
void emu_Free(void *pt) { free(pt); }

int emu_FileOpen(char *filename) {
  const char *path = resolvePath(filename);
  if (!path) return 0;
  lastFile = SD.open(path, FILE_READ);
  return lastFile ? 1 : 0;
}

int emu_FileRead(char *buf, int size) {
  if (!lastFile) return 0;
  return (int)lastFile.read((uint8_t *)buf, (size_t)size);
}

unsigned char emu_FileGetc(void) {
  if (!lastFile) return 0;
  int c = lastFile.read();
  return c < 0 ? 0 : (unsigned char)c;
}

int emu_FileSeek(int seek) {
  if (!lastFile) return -1;
  return lastFile.seek((uint32_t)seek, SeekSet) ? seek : -1;
}

void emu_FileClose(void) {
  if (lastFile) lastFile.close();
}

int emu_FileSize(char *filename) {
  const char *path = resolvePath(filename);
  if (!path) return 0;
  fs::File f = SD.open(path, FILE_READ);
  if (!f) return 0;
  int size = (int)f.size();
  f.close();
  return size;
}

int emu_LoadFile(char *filename, char *buf, int size) {
  const char *path = resolvePath(filename);
  if (!path) return 0;

  fs::File f = SD.open(path, FILE_READ);
  if (!f) {
    Serial.printf("emu_LoadFile: ROM SD nao encontrada: %s\n", path);
    return 0;
  }

  size_t total = f.size();
  if (total > (size_t)size) {
    Serial.printf("emu_LoadFile: ROM maior que buffer: %u > %d\n", (unsigned)total, size);
    f.close();
    return 0;
  }

  size_t got = f.read((uint8_t *)buf, total);
  f.close();

  if (got != total) {
    Serial.printf("emu_LoadFile: leitura incompleta: %u/%u\n",
                  (unsigned)got, (unsigned)total);
    return 0;
  }

  Serial.printf("ROM SD carregada: %u bytes | %s\n", (unsigned)got, path);
  return (int)got;
}

int emu_LoadFileSeek(char *filename, char *buf, int size, int seek) {
  const char *path = resolvePath(filename);
  if (!path) return 0;
  fs::File f = SD.open(path, FILE_READ);
  if (!f) return 0;
  if (!f.seek((uint32_t)seek, SeekSet)) {
    f.close();
    return 0;
  }
  int got = (int)f.read((uint8_t *)buf, (size_t)size);
  f.close();
  return got;
}

void emu_InitJoysticks(void) {}
int emu_SwapJoysticks(int statusOnly) { (void)statusOnly; return 0; }

unsigned short emu_DebounceLocalKeys(void) {
  uint16_t click = dabbleKeys & (uint16_t)~previousKeys;
  previousKeys = dabbleKeys;
  return click;
}

int emu_ReadKeys(void) { return (int)dabbleKeys; }
int emu_GetPad(void) { return 0; }
int emu_ReadAnalogJoyX(int min, int max) { return (min + max) / 2; }
int emu_ReadAnalogJoyY(int min, int max) { return (min + max) / 2; }
int emu_ReadI2CKeyboard(void) { return 0; }
int emu_setKeymap(int index) { return index; }

void emu_sndInit(void) {}
void emu_sndPlaySound(int chan, int volume, int freq) { (void)chan; (void)volume; (void)freq; }
void emu_sndPlayBuzz(int size, int val) { (void)size; (void)val; }

void emu_SetPaletteEntry(unsigned char r, unsigned char g, unsigned char b, int index) {
  if (index >= 0 && index < PALETTE_SIZE) palette16[index] = rgb565(r, g, b);
}

void emu_DrawScreen(unsigned char *VBuf, int width, int height, int stride) {
  if (!VBuf || !frame565) return;
  if (width != GAME_W || height < 1) return;
  if (height > GAME_H) height = GAME_H;

  for (int y = 0; y < GAME_H; ++y) {
    int sy = y < height ? y : height - 1;
    const uint8_t *src = VBuf + sy * stride;
    uint16_t *dst = frame565 + y * SCREEN_W;
    for (int x = 0; x < GAME_W; ++x) {
      uint16_t c = palette16[src[x]];
      dst[x * 2] = c;
      dst[x * 2 + 1] = c;
    }
  }

  tft.pushImage(0, GAME_Y, SCREEN_W, GAME_H, frame565);
}

void emu_DrawLine(unsigned char *VBuf, int width, int height, int line) {
  (void)VBuf; (void)width; (void)height; (void)line;
}
void emu_DrawVsync(void) { frameSkip = (frameSkip + 1) & VID_FRAME_SKIP; }
int emu_FrameSkip(void) { return frameSkip; }
void *emu_LineBuffer(int line) { (void)line; return nullptr; }

}
