#include "options.h"
#include "Vcsemu.h"
#include "types.h"
#include "vmachine.h"
#include "display.h"
#include <string.h>

#include "emuapi.h"

/****************************************************************************
* Local macros / typedefs
****************************************************************************/

/****************************************************************************
* Global data
****************************************************************************/

/****************************************************************************
* Imported procedures
****************************************************************************/
extern void mainloop(void);

/****************************************************************************
* Local procedures
****************************************************************************/

/****************************************************************************
* Exported procedures
****************************************************************************/
int vcs_Init(void)
{
  /*
   * Apenas reserva as estruturas do emulador. A CPU/TIA nao pode ser
   * inicializada aqui porque, no menu online, ainda nao existe ROM carregada
   * e rom_size ainda e zero.
   */
  return init_machine();
}

static const char *select_mapper_from_size(int size)
{
  /*
   * Mapeamento automatico para os formatos Atari mais comuns suportados
   * por este nucleo x2600/espvcs.
   *   2K/4K  = sem bankswitch
   *   8K     = F8
   *   12K    = FA (CBS)
   *   16K    = F6
   */
  if (size <= 4096) {
    base_opts.bank = 0;
    return "4K/SEM BANKSWITCH";
  }
  if (size == 8192) {
    base_opts.bank = 1;
    return "F8 (8K)";
  }
  if (size == 12288) {
    base_opts.bank = 4;
    return "FA (12K)";
  }
  if (size == 16384) {
    base_opts.bank = 2;
    return "F6 (16K)";
  }

  base_opts.bank = 0;
  return "DESCONHECIDO";
}

int vcs_Start(char * filename)
{
  int size = emu_LoadFile(filename, (char *)theCart, 16384);

  if (size < 2048 || size > 16384)
    return 0;

  rom_size = size;
  if (size == 2048)
  {
    memcpy (&theCart[2048], &theCart[0], 2048);
    rom_size = 4096;
  }
  select_mapper_from_size(rom_size);

  /* Agora sim: ROM e mapper ja estao definidos antes do reset da maquina. */
  init_hardware();
  tv_on();
  return 1;

}


void vcs_Step(void)
{
  //emu_printf("s");
  mainloop();
}






