#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <SD.h>
#include <esp_heap_caps.h>
#include <esp_system.h>
#include <string.h>
#include <strings.h>

#define CUSTOM_SETTINGS
#define INCLUDE_GAMEPAD_MODULE
#include <DabbleESP32.h>

extern "C" {
#include "emuapi.h"
#include "Vcsemu.h"
}

// ============================================================
// HALO 2600 RETRO - SD ONLY TEST v5 - TFT HSPI INSTANCE + MAX500
// Base: HALO 2600 RETRO v1.0.0 (repositorio do usuario)
// ROMs: exclusivamente microSD /Atari2600/*.a26|*.bin
// ============================================================

// Diagnostico antecipado: impede compilar com setup que pode reiniciar o ESP32-S3.
#if !defined(USE_HSPI_PORT)
  #error "HALO SD v5: habilite USE_HSPI_PORT no User_Setup ativo do TFT_eSPI."
#endif
#if !defined(ILI9341_DRIVER)
  #error "HALO SD v5: configure ILI9341_DRIVER no TFT_eSPI."
#endif
#if !defined(TFT_CS) || (TFT_CS != 10) || !defined(TFT_DC) || (TFT_DC != 9) || !defined(TFT_RST) || (TFT_RST != 4)
  #error "HALO SD v5: pinos TFT incorretos; use CS=10 DC=9 RST=4."
#endif
#if !defined(TFT_MOSI) || (TFT_MOSI != 11) || !defined(TFT_MISO) || (TFT_MISO != 13) || !defined(TFT_SCLK) || (TFT_SCLK != 12)
  #error "HALO SD v5: SPI do TFT precisa ser MOSI=11 MISO=13 SCK=12."
#endif
#if defined(TOUCH_CS) && (TOUCH_CS == 14)
  #error "HALO SD v5: remova TOUCH_CS 14 do User_Setup; GPIO14 e exclusivamente SD_CS."
#endif

TFT_eSPI tft = TFT_eSPI(320, 240);

// Mesmo barramento SPI validado no teste do usuario.
#define SD_SCK   12
#define SD_MISO  13
#define SD_MOSI  11
#define SD_CS    14
#define TFT_CS_PIN 10

static const char *ROM_DIR = "/Atari2600";
static const int MAX_ROMS = 500;
static const size_t MAX_ROM_SIZE = 16384; // limite atual do nucleo espvcs usado
static const int VISIBLE_ITEMS = 8;
static const int PAGE_JUMP = 8;

// A instancia SPI pertence ao TFT_eSPI (USE_HSPI_PORT ATIVADO).
// SD.begin recebe tft.getSPIinstance(): NAO criar outro SPIClass(HSPI),
// NAO usar o objeto global SPI e NAO chamar SPI.begin separadamente.

struct RomEntry {
  char name[72];
  char path[192];
  uint32_t size;
};

static RomEntry *roms = nullptr;
static int romCount = 0;
static int selectedRom = 0;
static int menuTop = 0;
static bool menuNeedsRedraw = true;
static bool emulatorInitialized = false;

enum AppMode {
  MODE_MENU,
  MODE_GAME
};
static AppMode appMode = MODE_MENU;

// ------------------------------------------------------------
// UI
// ------------------------------------------------------------
static void centerText(const String &txt, int y, uint16_t color, uint8_t size) {
  tft.setTextSize(size);
  tft.setTextColor(color, TFT_BLACK);
  int16_t x = (tft.width() - (int)txt.length() * 6 * size) / 2;
  if (x < 2) x = 2;
  tft.setCursor(x, y);
  tft.print(txt);
}

static void showStatus(const String &line1, const String &line2 = "") {
  tft.fillScreen(TFT_BLACK);
  centerText("HALO 2600 RETRO", 28, TFT_CYAN, 2);
  centerText(line1, 92, TFT_WHITE, 2);
  if (line2.length()) centerText(line2, 128, TFT_YELLOW, 1);
}

static bool isSupportedSize(uint32_t size) {
  return size >= 2048 && size <= MAX_ROM_SIZE;
}

static void drawMenu() {
  tft.fillScreen(TFT_BLACK);
  tft.fillRect(0, 0, tft.width(), 32, TFT_DARKGREY);
  tft.setTextSize(2);
  tft.setTextColor(TFT_CYAN, TFT_DARKGREY);
  tft.setCursor(6, 8);
  tft.print("HALO2600 SD");

  if (romCount <= 0) {
    centerText("SEM ROMS", 88, TFT_RED, 2);
    centerText("/Atari2600", 124, TFT_YELLOW, 1);
    centerText("Use .a26 ou .bin", 142, TFT_LIGHTGREY, 1);
    menuNeedsRedraw = false;
    return;
  }

  if (selectedRom < 0) selectedRom = romCount - 1;
  if (selectedRom >= romCount) selectedRom = 0;

  if (selectedRom < menuTop) menuTop = selectedRom;
  if (selectedRom >= menuTop + VISIBLE_ITEMS) {
    menuTop = selectedRom - VISIBLE_ITEMS + 1;
  }

  const int firstY = 40;
  const int rowH = 21;

  for (int row = 0; row < VISIBLE_ITEMS; ++row) {
    int idx = menuTop + row;
    if (idx >= romCount) break;

    int y = firstY + row * rowH;
    bool selected = (idx == selectedRom);
    bool supported = isSupportedSize(roms[idx].size);

    if (selected) {
      tft.fillRect(3, y - 1, tft.width() - 6, rowH, TFT_BLUE);
      tft.setTextColor(supported ? TFT_WHITE : TFT_YELLOW, TFT_BLUE);
    } else {
      tft.setTextColor(supported ? TFT_LIGHTGREY : TFT_DARKGREY, TFT_BLACK);
    }

    tft.setTextSize(1);
    tft.setCursor(7, y + 5);
    tft.print(selected ? ">" : " ");
    tft.print(supported ? " " : "! ");

    String n = roms[idx].name;
    if (n.length() > 38) n = n.substring(0, 35) + "...";
    tft.print(n);
  }

  tft.drawFastHLine(0, 211, tft.width(), TFT_DARKGREY);
  tft.setTextSize(1);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setCursor(5, 216);
  tft.printf("%d/%d  %uB", selectedRom + 1, romCount,
             (unsigned)roms[selectedRom].size);
  tft.setCursor(5, 229);
  tft.print("UP/DN nav  L/R pag  X abrir");

  menuNeedsRedraw = false;
}

// ------------------------------------------------------------
// SD / biblioteca
// ------------------------------------------------------------
static bool isRomFilename(const char *name) {
  if (!name) return false;
  const char *dot = strrchr(name, '.');
  if (!dot) return false;
  return strcasecmp(dot, ".a26") == 0 || strcasecmp(dot, ".bin") == 0;
}

static void makeDisplayName(const char *pathOrName, char *out, size_t outSize) {
  const char *base = strrchr(pathOrName, '/');
  base = base ? base + 1 : pathOrName;

  snprintf(out, outSize, "%s", base);
  char *dot = strrchr(out, '.');
  if (dot) *dot = '\0';

  // Deixa nomes de packs um pouco mais legiveis sem alterar o arquivo real.
  for (size_t i = 0; out[i]; ++i) {
    if (out[i] == '_') out[i] = ' ';
  }
}

static int romComparator(const void *a, const void *b) {
  const RomEntry *ra = (const RomEntry *)a;
  const RomEntry *rb = (const RomEntry *)b;
  return strcasecmp(ra->name, rb->name);
}

static void printMemory(const char *tag) {
  Serial.printf("[MEM][%s] heap=%u | PSRAM livre=%u | PSRAM total=%u\n",
                tag,
                (unsigned)ESP.getFreeHeap(),
                (unsigned)ESP.getFreePsram(),
                (unsigned)ESP.getPsramSize());
}

static bool testSDPath(const char *path, const char *tag) {
  unsigned long t0 = millis();
  Serial.printf("[DIAG][%s] SD.open(\"%s\") ...\n", tag, path);
  File f = SD.open(path);
  if (!f) {
    Serial.printf("[DIAG][%s] FALHOU abrir %s | %lu ms\n",
                  tag, path, millis() - t0);
    return false;
  }
  Serial.printf("[DIAG][%s] OK abrir %s | dir=%s | %lu ms\n",
                tag, path, f.isDirectory() ? "SIM" : "NAO", millis() - t0);
  f.close();
  return true;
}

static bool mountSDSharedSPI() {
  unsigned long t0 = millis();

  Serial.println("[BOOT][SD] Preparando CS no barramento compartilhado...");
  pinMode(TFT_CS_PIN, OUTPUT);
  pinMode(SD_CS, OUTPUT);
  digitalWrite(TFT_CS_PIN, HIGH);
  digitalWrite(SD_CS, HIGH);
  delay(10);

  Serial.printf("[BOOT][SD] Pinos compartilhados: SCK=%d MISO=%d MOSI=%d | TFT_CS=%d | SD_CS=%d\n",
                SD_SCK, SD_MISO, SD_MOSI, TFT_CS_PIN, SD_CS);
  printMemory("antes-SD.begin");

  SPIClass &sharedSPI = tft.getSPIinstance();
  Serial.printf("[BOOT][SPI] TFT_eSPI SPI instance=%p | objeto SPI global=%p\n",
                (void*)&sharedSPI, (void*)&SPI);
  Serial.println("[BOOT][SD] Chamando SD.begin(SD_CS, tft.getSPIinstance(), 1 MHz)...");
  unsigned long beginStart = millis();
  bool mounted = SD.begin(SD_CS, sharedSPI, 1000000);
  Serial.printf("[BOOT][SD] SD.begin() retornou=%s | levou %lu ms\n",
                mounted ? "TRUE" : "FALSE", millis() - beginStart);

  if (!mounted) {
    Serial.println("[BOOT][SD] ERRO ao montar cartao no SPI compartilhado.");
    return false;
  }

  uint8_t cardType = SD.cardType();
  Serial.printf("[BOOT][SD] cardType=%u\n", cardType);
  if (cardType == CARD_NONE) {
    Serial.println("[BOOT][SD] Nenhum cartao detectado.");
    return false;
  }

  const char *typeName = "DESCONHECIDO";
  if (cardType == CARD_MMC) typeName = "MMC";
  else if (cardType == CARD_SD) typeName = "SDSC";
  else if (cardType == CARD_SDHC) typeName = "SDHC";

  Serial.printf("[BOOT][SD] Tipo=%s | capacidade=%llu MB | FS=%llu MB | usado=%llu MB\n",
                typeName,
                SD.cardSize() / (1024ULL * 1024ULL),
                SD.totalBytes() / (1024ULL * 1024ULL),
                SD.usedBytes() / (1024ULL * 1024ULL));

  if (!testSDPath("/", "SHARED-SPI-ROOT")) return false;
  if (!testSDPath(ROM_DIR, "SHARED-SPI-ROMDIR")) {
    Serial.printf("[BOOT][SD] ERRO: pasta %s nao encontrada. Nenhum arquivo sera criado.\n", ROM_DIR);
    return false;
  }

  Serial.printf("[BOOT][SD] Montagem compartilhada concluida | total %lu ms\n", millis() - t0);
  return true;
}

static bool scanRoms() {
  if (roms) {
    free(roms);
    roms = nullptr;
  }
  romCount = 0;

  File dir = SD.open(ROM_DIR);
  if (!dir || !dir.isDirectory()) {
    if (dir) dir.close();
    Serial.println("[SD] ERRO: /Atari2600 nao existe ou nao e um diretorio.");
    return false;
  }

  // Aloca apenas o maximo que realmente iremos indexar.
  const size_t bytes = (size_t)MAX_ROMS * sizeof(RomEntry);
  roms = (RomEntry *)heap_caps_malloc(bytes, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  if (!roms) roms = (RomEntry *)malloc(bytes);
  if (!roms) {
    Serial.printf("[MEM] Falha ao alocar indice para %d ROMs (%u bytes).\n",
                  MAX_ROMS, (unsigned)bytes);
    dir.close();
    return false;
  }
  memset(roms, 0, bytes);

  Serial.printf("[ROM] Lendo no maximo as primeiras %d ROMs de %s...\n",
                MAX_ROMS, ROM_DIR);
  unsigned long scanStart = millis();
  unsigned long lastProgress = scanStart;
  uint32_t entriesSeen = 0;

  int idx = 0;
  Serial.println("[ROM] Chamando openNextFile() pela primeira vez...");
  File file = dir.openNextFile();
  Serial.printf("[ROM] Primeiro openNextFile(): %s | %lu ms\n",
                file ? "OK" : "SEM ARQUIVOS", millis() - scanStart);

  // IMPORTANTE: paramos imediatamente ao encontrar 500 ROMs validas.
  // Nao percorremos o restante do pack, deixando a inicializacao muito mais rapida.
  while (file && idx < MAX_ROMS) {
    entriesSeen++;
    if (!file.isDirectory() && isRomFilename(file.name())) {
      const char *p = file.path();
      // Nem todo backend de File retorna path() completo. Construimos /Atari2600/arquivo
      // quando necessario e IGNORAMOS nomes longos em vez de truncar um caminho.
      String fullPath = (p && p[0] == '/') ? String(p) :
                        String(ROM_DIR) + "/" + String(file.name());
      if (fullPath.length() >= sizeof(roms[idx].path)) {
        Serial.printf("[ROM][SKIP] Caminho excede %u caracteres: %s\n",
                      (unsigned)(sizeof(roms[idx].path) - 1), fullPath.c_str());
      } else {
        snprintf(roms[idx].path, sizeof(roms[idx].path), "%s", fullPath.c_str());
        makeDisplayName(file.name(), roms[idx].name, sizeof(roms[idx].name));
        roms[idx].size = (uint32_t)file.size();
        idx++;
      }

      if (idx <= 10 || (idx % 25) == 0) {
        Serial.printf("[ROM] #%d | %s | %u bytes | entradaFS=%u | tempo=%lu ms\n",
                      idx,
                      roms[idx - 1].path,
                      (unsigned)roms[idx - 1].size,
                      (unsigned)entriesSeen,
                      millis() - scanStart);
      }
    } else if (entriesSeen <= 10) {
      Serial.printf("[ROM] Ignorado #%u: %s%s\n",
                    (unsigned)entriesSeen,
                    file.isDirectory() ? "[DIR] " : "[NAO-ROM] ",
                    file.name());
    }

    if (millis() - lastProgress >= 2000) {
      Serial.printf("[ROM] Progresso: %d/%d ROMs | %u entradas vistas | %lu ms\n",
                    idx, MAX_ROMS, (unsigned)entriesSeen, millis() - scanStart);
      lastProgress = millis();
    }

    file.close();
    if (idx >= MAX_ROMS) break;
    file = dir.openNextFile();
  }

  if (file) file.close();
  dir.close();

  romCount = idx;

  if (romCount == 0) {
    Serial.println("[SD] Nenhuma ROM .a26/.bin encontrada em /Atari2600.");
    return true;
  }

  // Ordena somente as ROMs que foram indexadas (maximo 500).
  qsort(roms, romCount, sizeof(RomEntry), romComparator);

  int compatible = 0;
  for (int i = 0; i < romCount; ++i) {
    if (isSupportedSize(roms[i].size)) compatible++;
  }

  Serial.printf("[ROM] Biblioteca pronta: %d ROMs carregadas (limite %d).\n",
                romCount, MAX_ROMS);
  Serial.printf("[ROM] Entradas do diretorio examinadas: %u | tempo scanner=%lu ms\n",
                (unsigned)entriesSeen, millis() - scanStart);
  Serial.printf("[ROM] Tamanho compativel com nucleo atual: %d/%d\n",
                compatible, romCount);
  Serial.printf("[MEM] Indice reservado: %u bytes | heap=%u | PSRAM=%u\n",
                (unsigned)bytes,
                (unsigned)ESP.getFreeHeap(),
                (unsigned)ESP.getFreePsram());
  return true;
}

// ------------------------------------------------------------
// Controles - preservados da base v1.0.0
// ------------------------------------------------------------
static uint16_t readGameKeys() {
  uint16_t keys = 0;

  const bool dLeft  = GamePad.isLeftPressed();
  const bool dRight = GamePad.isRightPressed();
  const bool dUp    = GamePad.isUpPressed();
  const bool dDown  = GamePad.isDownPressed();

  if (dLeft != dRight) {
    if (dLeft)  keys |= MASK_JOY2_RIGHT;
    if (dRight) keys |= MASK_JOY2_LEFT;
  } else if (!dLeft && !dRight) {
    const int ax = GamePad.getXaxisData();
    const int DEAD_ZONE = 2;
    if (ax > DEAD_ZONE)       keys |= MASK_JOY2_LEFT;
    else if (ax < -DEAD_ZONE) keys |= MASK_JOY2_RIGHT;
  }

  if (dUp != dDown) {
    if (dUp)   keys |= MASK_JOY2_UP;
    if (dDown) keys |= MASK_JOY2_DOWN;
  } else if (!dUp && !dDown) {
    const int ay = GamePad.getYaxisData();
    const int DEAD_ZONE = 2;
    if (ay > DEAD_ZONE)       keys |= MASK_JOY2_UP;
    else if (ay < -DEAD_ZONE) keys |= MASK_JOY2_DOWN;
  }

  if (GamePad.isCrossPressed() || GamePad.isSquarePressed()) {
    keys |= MASK_JOY2_BTN;
  }

  bool start = GamePad.isStartPressed();
  bool select = GamePad.isSelectPressed();
  if (!select && (GamePad.isCirclePressed() || start)) keys |= MASK_KEY_USER1;
  if (!start && (GamePad.isTrianglePressed() || select)) keys |= MASK_KEY_USER2;
  return keys;
}

static bool exitComboPressed() {
  static unsigned long comboStart = 0;
  bool both = GamePad.isStartPressed() && GamePad.isSelectPressed();
  if (!both) {
    comboStart = 0;
    return false;
  }
  if (comboStart == 0) comboStart = millis();
  return millis() - comboStart >= 350;
}

static void waitRelease(uint32_t timeoutMs = 800) {
  unsigned long start = millis();
  while (millis() - start < timeoutMs) {
    Dabble.processInput();
    if (!GamePad.isCrossPressed() && !GamePad.isSquarePressed() &&
        !GamePad.isStartPressed() && !GamePad.isSelectPressed()) break;
    delay(5);
  }
}

static void exitGameToMenu() {
  Serial.println("[MENU] START+SELECT -> menu SD");
  emu_SetDabbleKeys(0);
  appMode = MODE_MENU;
  waitRelease(1000);
  menuNeedsRedraw = true;
  drawMenu();
}

static void startSelectedRom() {
  if (romCount <= 0 || selectedRom < 0 || selectedRom >= romCount) return;

  const RomEntry &r = roms[selectedRom];
  if (!isSupportedSize(r.size)) {
    Serial.printf("[ROM] Incompativel com nucleo atual: %s (%u bytes)\n",
                  r.path, (unsigned)r.size);
    showStatus("ROM INCOMPATIVEL", String(r.size) + " bytes");
    delay(1400);
    menuNeedsRedraw = true;
    drawMenu();
    return;
  }

  showStatus("CARREGANDO...", r.name);
  Serial.printf("[ROM] Abrindo SD: %s (%u bytes)\n", r.path, (unsigned)r.size);

  if (!emulatorInitialized) {
    if (!emu_init() || !vcs_Init()) {
      Serial.println("[EMU] Falha ao inicializar emulador.");
      showStatus("ERRO EMULADOR", "Memoria/init");
      delay(1600);
      menuNeedsRedraw = true;
      drawMenu();
      return;
    }
    emulatorInitialized = true;
  }

  emu_SetDabbleKeys(0);
  if (!vcs_Start((char *)r.path)) {
    Serial.println("[EMU] vcs_Start falhou.");
    showStatus("ERRO NA ROM", "Mapper/tamanho/leitura");
    delay(1600);
    menuNeedsRedraw = true;
    drawMenu();
    return;
  }

  appMode = MODE_GAME;
  waitRelease();
  emu_SetDabbleKeys(0);
  tft.fillScreen(TFT_BLACK);
}

// ------------------------------------------------------------
// Menu input
// ------------------------------------------------------------
static void handleMenuInput() {
  static bool prevUp = false;
  static bool prevDown = false;
  static bool prevLeft = false;
  static bool prevRight = false;
  static bool prevChoose = false;

  bool dUp = GamePad.isUpPressed();
  bool dDown = GamePad.isDownPressed();
  bool dLeft = GamePad.isLeftPressed();
  bool dRight = GamePad.isRightPressed();

  bool up = false, down = false, left = false, right = false;

  if (dUp != dDown) {
    up = dUp; down = dDown;
  } else if (!dUp && !dDown) {
    int ay = GamePad.getYaxisData();
    up = ay > 2;
    down = ay < -2;
  }

  if (dLeft != dRight) {
    left = dLeft; right = dRight;
  } else if (!dLeft && !dRight) {
    int ax = GamePad.getXaxisData();
    left = ax < -2;
    right = ax > 2;
  }

  bool choose = GamePad.isCrossPressed() || GamePad.isSquarePressed();

  if (romCount > 0) {
    if (up && !prevUp) {
      selectedRom = (selectedRom - 1 + romCount) % romCount;
      menuNeedsRedraw = true;
    }
    if (down && !prevDown) {
      selectedRom = (selectedRom + 1) % romCount;
      menuNeedsRedraw = true;
    }
    if (left && !prevLeft) {
      selectedRom -= PAGE_JUMP;
      if (selectedRom < 0) selectedRom = 0;
      menuNeedsRedraw = true;
    }
    if (right && !prevRight) {
      selectedRom += PAGE_JUMP;
      if (selectedRom >= romCount) selectedRom = romCount - 1;
      menuNeedsRedraw = true;
    }
    if (choose && !prevChoose) startSelectedRom();
  }

  prevUp = up; prevDown = down;
  prevLeft = left; prevRight = right;
  prevChoose = choose;

  if (menuNeedsRedraw && appMode == MODE_MENU) drawMenu();
}

void setup() {
  Serial.begin(115200);
  delay(1200);

  Serial.println();
  Serial.println("============================================================");
  Serial.println("HALO 2600 RETRO - SD ONLY v5 - TFT HSPI INSTANCE + MAX500");
  Serial.println("============================================================");
  Serial.printf("[BOOT] millis=%lu\n", millis());
  Serial.printf("[BOOT] CPU=%u MHz\n", (unsigned)getCpuFrequencyMhz());
  Serial.printf("[BOOT] Motivo do ultimo reset (esp_reset_reason)=%d\n", (int)esp_reset_reason());
  Serial.printf("[BOOT] PSRAM detectada=%s | tamanho=%u bytes\n",
                psramFound() ? "SIM" : "NAO", (unsigned)ESP.getPsramSize());
  printMemory("inicio");
  Serial.printf("[BOOT] ROM_DIR=%s | MAX_ROMS=%d | MAX_ROM_SIZE=%u\n",
                ROM_DIR, MAX_ROMS, (unsigned)MAX_ROM_SIZE);
  Serial.println("[BOOT] SPI compartilhado: tft.getSPIinstance() (HSPI do TFT_eSPI)");
  Serial.println("[BOOT] USE_HSPI_PORT=ATIVADO; TOUCH_CS14=PROIBIDO.");
  Serial.printf("[BOOT] Pines TFT SPI: MOSI=%d MISO=%d SCK=%d CS=%d DC=%d RST=%d\n",
                TFT_MOSI, TFT_MISO, TFT_SCLK, TFT_CS, TFT_DC, TFT_RST);

  pinMode(TFT_CS_PIN, OUTPUT);
  pinMode(SD_CS, OUTPUT);
  digitalWrite(TFT_CS_PIN, HIGH);
  digitalWrite(SD_CS, HIGH);

  Serial.println();
  Serial.println("--------------- ETAPA 1: PREPARAR HSPI ---------------");
  Serial.println("[BOOT][SPI] Nao iniciamos outro SPIClass. TFT_eSPI criara o HSPI.");
  Serial.printf("[BOOT][SPI] SD_CS=%d e TFT_CS=%d definidos HIGH antes do tft.init()\n",
                SD_CS, TFT_CS_PIN);

  Serial.println();
  Serial.println("---------------- ETAPA 2: TFT --------------------");
  unsigned long tftStart = millis();
  Serial.println("[BOOT][TFT] Chamando tft.init() na instancia HSPI do TFT_eSPI...");
  tft.init();
  Serial.printf("[BOOT][TFT] tft.init() retornou | %lu ms\n", millis() - tftStart);
  tft.setRotation(2);
  tft.fillScreen(TFT_BLACK);
  Serial.printf("[BOOT][TFT] Resolucao logica=%dx%d\n", tft.width(), tft.height());
  Serial.printf("[BOOT][SPI] HSPI instance=%p (obtida de tft.getSPIinstance())\n",
                (void*)&tft.getSPIinstance());
  showStatus("INICIANDO SD", "SPI COMPARTILHADO");

  Serial.println();
  Serial.println("--------------- ETAPA 3: MICROSD -----------------");
  // Garante que o TFT nao esta selecionado ao iniciar o SD.
  digitalWrite(TFT_CS_PIN, HIGH);
  digitalWrite(SD_CS, HIGH);
  if (!mountSDSharedSPI()) {
    Serial.println("[FATAL] Falha ao montar SD usando SPI compartilhado.");
    Serial.println("[FATAL] Confira SD_CS=14, touchscreen removido e SD nos mesmos MOSI/MISO/SCK do TFT.");
    showStatus("ERRO SD/SPI", "Veja Monitor Serial");
    while (true) delay(1000);
  }

  Serial.println();
  Serial.println("------------- ETAPA 4: TESTE CRUZADO -------------");
  Serial.println("[DIAG] 4 ciclos de desenho TFT + abertura SD + leitura de diretorio.");
  const uint16_t testColors[4] = {TFT_RED, TFT_GREEN, TFT_BLUE, TFT_YELLOW};
  for (int round = 0; round < 4; round++) {
    unsigned long t0 = millis();
    Serial.printf("[DIAG][SPI] Rodada %d/4: desenhando TFT...\n", round + 1);
    tft.fillRect(4 + round * 22, 177, 18, 12, testColors[round]);
    Serial.printf("[DIAG][SPI] TFT desenho OK: %lu ms. Abrindo SD...\n", millis() - t0);
    if (!testSDPath((round & 1) ? ROM_DIR : "/", "TFT-SD-ALTERNADO")) {
      Serial.printf("[FATAL] SD falhou na rodada %d apos TFT.\n", round + 1);
      showStatus("CONFLITO SPI", "Veja Monitor Serial");
      while (true) delay(1000);
    }
    Serial.printf("[DIAG][SPI] Rodada %d/4 OK | %lu ms\n", round + 1, millis() - t0);
    delay(10);
  }
  Serial.println("[DIAG][SPI] OK: TFT + SD alternando na instancia HSPI do TFT_eSPI.");

  Serial.println();
  Serial.println("--------------- ETAPA 5: BIBLIOTECA --------------");
  showStatus("LENDO BIBLIOTECA", "Maximo: 500 ROMs");
  if (!scanRoms()) {
    Serial.println("[FATAL] Falha na indexacao da biblioteca.");
    showStatus("ERRO BIBLIOTECA", "Veja Monitor Serial");
    while (true) delay(1000);
  }

  Serial.println();
  Serial.println("------------- ETAPA 5B: LEITURA DE ROM -------------");
  if (romCount > 0) {
    Serial.printf("[DIAG][ROM] Testando abrir e ler primeira ROM indexada: %s\n", roms[0].path);
    File romProbe = SD.open(roms[0].path, FILE_READ);
    if (!romProbe || romProbe.isDirectory()) {
      Serial.println("[FATAL][ROM] Arquivo indexado nao abriu. Verifique truncamento/caminho.");
      showStatus("ERRO CAMINHO ROM", "Veja Monitor Serial");
      while (true) delay(1000);
    }
    size_t bytesToRead = (romProbe.size() < 16u) ? romProbe.size() : 16u;
    uint8_t probeBytes[16] = {0};
    size_t bytesRead = romProbe.read(probeBytes, bytesToRead);
    romProbe.close();
    Serial.printf("[DIAG][ROM] Leitura: %u/%u bytes | dados:",
                  (unsigned)bytesRead, (unsigned)bytesToRead);
    for (size_t i = 0; i < bytesRead; i++) Serial.printf(" %02X", probeBytes[i]);
    Serial.println();
    if (bytesRead != bytesToRead) {
      Serial.println("[FATAL][ROM] Leitura parcial da ROM no SD.");
      showStatus("ERRO LENDO ROM", "Veja Monitor Serial");
      while (true) delay(1000);
    }
    tft.fillRect(4, 174, 126, 16, TFT_GREEN);
    if (!testSDPath(ROM_DIR, "POS-LEITURA-E-TFT")) {
      Serial.println("[FATAL][SPI] SD falhou apos leitura de ROM e novo desenho TFT.");
      showStatus("CONFLITO SPI", "Veja Monitor Serial");
      while (true) delay(1000);
    }
    Serial.println("[DIAG][ROM] ROM + TFT + SD: OK");
  } else {
    Serial.println("[DIAG][ROM] Nenhuma ROM indexada; teste de leitura nao aplicavel.");
  }

  Serial.println();
  Serial.println("---------------- ETAPA 6: DABBLE -----------------");
  printMemory("antes-Dabble");
  unsigned long dabbleStart = millis();
  Dabble.begin("ESP32-S3-HALO2600-SD");
  Serial.printf("[DABBLE] begin() concluido | %lu ms\n", millis() - dabbleStart);
  Serial.println("[DABBLE] Nome BLE: ESP32-S3-HALO2600-SD");
  printMemory("depois-Dabble");

  Serial.println();
  Serial.println("---------------- BOOT CONCLUIDO ------------------");
  Serial.printf("[BOOT] ROMs indexadas=%d\n", romCount);
  Serial.println("[MENU] UP/DOWN=1 jogo | LEFT/RIGHT=pagina | CROSS/SQUARE=abrir");
  Serial.println("[JOGO] START+SELECT 350ms=voltar ao menu");
  Serial.printf("[BOOT] Tempo total ate menu: %lu ms\n", millis());
  Serial.println("============================================================");

  drawMenu();
}

void loop() {
  Dabble.processInput();

  if (appMode == MODE_MENU) {
    handleMenuInput();
    delay(5);
    return;
  }

  if (exitComboPressed()) {
    exitGameToMenu();
    return;
  }

  emu_SetDabbleKeys(readGameKeys());
  vcs_Step();
}
