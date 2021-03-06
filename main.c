/*
    ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include <stdio.h>
#include <string.h>

#include "ch.h"
#include "hal.h"
#include "test.h"

#include "chprintf.h"
#include "shell.h"
#include "CDC/myUSB.h"


#include "lwipthread.h"
#include <lwip/ip_addr.h>

#include "web/web.h"
//#include "socket/socket.h"

#include "ff.h"

#include "console/ansi.h"
#include "console/cmd.h"

#include "my/myPWM.h"
#include "my/myADC.h"
#include "my/myMisc.h"

#include "httpserver_raw/httpd.h"

#include "netstream/server.h"

#include "netbios/netbios.h"

///* Virtual serial port over USB.*/
//SerialUSBDriver SDU1;
//

/*===========================================================================*/
/* Card insertion monitor.                                                   */
/*===========================================================================*/

#define POLLING_INTERVAL                10
#define POLLING_DELAY                   10

/**
 * @brief   Card monitor timer.
 */
static VirtualTimer tmr;

/**
 * @brief   Debounce counter.
 */
static unsigned cnt;

/**
 * @brief   Card event sources.
 */
static EventSource inserted_event, removed_event;

/**
 * @brief   Insertion monitor timer callback function.
 *
 * @param[in] p         pointer to the @p BaseBlockDevice object
 *
 * @notapi
 */
static void tmrfunc(void *p) {
  BaseBlockDevice *bbdp = p;

  chSysLockFromIsr();
  if (cnt > 0) {
    if (blkIsInserted(bbdp)) {
      if (--cnt == 0) {
        chEvtBroadcastI(&inserted_event);
      }
    }
    else
      cnt = POLLING_INTERVAL;
  }
  else {
    if (!blkIsInserted(bbdp)) {
      cnt = POLLING_INTERVAL;
      chEvtBroadcastI(&removed_event);
    }
  }
  chVTSetI(&tmr, MS2ST(POLLING_DELAY), tmrfunc, bbdp);
  chSysUnlockFromIsr();
}

/**
 * @brief   Polling monitor start.
 *
 * @param[in] p         pointer to an object implementing @p BaseBlockDevice
 *
 * @notapi
 */
static void tmr_init(void *p) {

  chEvtInit(&inserted_event);
  chEvtInit(&removed_event);
  chSysLock();
  cnt = POLLING_INTERVAL;
  chVTSetI(&tmr, MS2ST(POLLING_DELAY), tmrfunc, p);
  chSysUnlock();
}

/*===========================================================================*/
/* FatFs related.                                                            */
/*===========================================================================*/

/**
 * @brief FS object.
 */
static FATFS SDC_FS;

/* FS mounted and ready.*/
static bool_t fs_ready = FALSE;

/* Generic large buffer.*/
static uint8_t fbuff[1024];

static FRESULT scan_files(BaseSequentialStream *chp, char *path) {
  FRESULT res;
  FILINFO fno;
  DIR dir;
  int i;
  char *fn;

#if _USE_LFN
  fno.lfname = 0;
  fno.lfsize = 0;
#endif
  res = f_opendir(&dir, path);
  if (res == FR_OK) {
    i = strlen(path);
    for (;;) {
      res = f_readdir(&dir, &fno);
      if (res != FR_OK || fno.fname[0] == 0)
        break;
      if (fno.fname[0] == '.')
        continue;
      fn = fno.fname;
      if (fno.fattrib & AM_DIR) {
        path[i++] = '/';
        strcpy(&path[i], fn);
        res = scan_files(chp, path);
        if (res != FR_OK)
          break;
        path[--i] = 0;
      }
      else {
        chprintf(chp, "%s/%s\r\n", path, fn);
      }
    }
  }
  return res;
}


/*===========================================================================*/
/* Command line related.                                                     */
/*===========================================================================*/

#define SHELL_WA_SIZE   THD_WA_SIZE(2048)
#define TEST_WA_SIZE    THD_WA_SIZE(256)

//static void cmd_mem(BaseSequentialStream *chp, int argc, char *argv[]) {
//  size_t n, size;
//
//  (void)argv;
//  if (argc > 0) {
//    chprintf(chp, "Usage: mem\r\n");
//    return;
//  }
//  n = chHeapStatus(NULL, &size);
//  chprintf(chp, "core free memory : %u bytes\r\n", chCoreStatus());
//  chprintf(chp, "heap fragments   : %u\r\n", n);
//  chprintf(chp, "heap free total  : %u bytes\r\n", size);
//}
//
//static void cmd_threads(BaseSequentialStream *chp, int argc, char *argv[]) {
//  static const char *states[] = {THD_STATE_NAMES};
//  Thread *tp;
//
//  (void)argv;
//  if (argc > 0) {
//    chprintf(chp, "Usage: threads\r\n");
//    return;
//  }
//  chprintf(chp, "    addr    stack prio refs     state time\r\n");
//  tp = chRegFirstThread();
//  do {
//    chprintf(chp, "%.8lx %.8lx %4lu %4lu %9s %lu\r\n",
//            (uint32_t)tp, (uint32_t)tp->p_ctx.r13,
//            (uint32_t)tp->p_prio, (uint32_t)(tp->p_refs - 1),
//            states[tp->p_state], (uint32_t)tp->p_time);
//    tp = chRegNextThread(tp);
//  } while (tp != NULL);
//}


//static const ShellCommand commands[] = {
//  {"mem", cmd_mem},
//  {"threads", cmd_threads},
//  {"test", cmd_test},
//  {"tree", cmd_tree},
//  {NULL, NULL}
//};
void cmd_test(BaseSequentialStream *chp, int argc, char *argv[]) {
  Thread *tp;

  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: test\r\n");
    return;
  }
  tp = chThdCreateFromHeap(NULL, TEST_WA_SIZE, chThdGetPriority(),
                           TestThread, chp);
  if (tp == NULL) {
    chprintf(chp, "out of memory\r\n");
    return;
  }
  chThdWait(tp);
}

void cmd_tree(BaseSequentialStream *chp, int argc, char *argv[]) {
  FRESULT err;
  uint32_t clusters;
  FATFS *fsp;

  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: tree\r\n");
    return;
  }
  if (!fs_ready) {
    chprintf(chp, "File System not mounted\r\n");
    return;
  }
  err = f_getfree("/", &clusters, &fsp);
  if (err != FR_OK) {
    chprintf(chp, "FS: f_getfree() failed\r\n");
    return;
  }
  chprintf(chp,
           "FS: %lu free clusters, %lu sectors per cluster, %lu bytes free\r\n",
           clusters, (uint32_t)SDC_FS.csize,
           clusters * (uint32_t)SDC_FS.csize * (uint32_t)MMCSD_BLOCK_SIZE);
  fbuff[0] = 0;
  scan_files(chp, (char *)fbuff);
}
static const ShellCommand commands[] = {
    {"mem", cmd_mem},
    {"threads",cmd_threads},
    {"status",cmd_status},
    {"ansiTest",cmd_ansiColorTest},
    {"echo",cmd_echo},
    {"getKey",cmd_getKey},
    {"extended",cmd_ExtendedAscii},
    {"box",cmd_box},
    {"draw",cmd_draw},
    {"test", cmd_test},
    {"tree", cmd_tree},
    {"toggle", cmd_toggle},
    {"t", cmd_toggle},
    {"blinkspeed", cmd_blinkspeed},
    {"bs", cmd_blinkspeed},
    {"cycle", cmd_cycle},
    {"c", cmd_cycle},
    {"ramp", cmd_ramp},
    {"r", cmd_ramp},
    {"measure", cmd_measure},
    {"m", cmd_measure},
    {"measureAnalog", cmd_measureA},
    {"ma", cmd_measureA},
    {"vref", cmd_Vref},
    {"v", cmd_Vref},
    {"temperature", cmd_Temperature},
    {"te", cmd_Temperature},
    {"measureDirect", cmd_measureDirect},
    {"md", cmd_measureDirect},
    {"measureContinuous", cmd_measureCont},
    {"mc", cmd_measureCont},
    {"readContinuousData", cmd_measureRead},
    {"rd", cmd_measureRead},
    {"stopContinuous", cmd_measureStop},
    {"sc", cmd_measureStop},
    {NULL,NULL}
};

static const ShellConfig shell_cfg1 = {
  (BaseSequentialStream *)&SDU1,
  commands
};


/*
 * Green LED blinker thread, times are in milliseconds.
 */
static WORKING_AREA(waThread1, 128);
static msg_t Thread1(void *arg) {

  (void)arg;
  chRegSetThreadName("httpserver_raw");
  httpd_init();
}


/*
 * Card insertion event.
 */
static void InsertHandler(eventid_t id) {
  FRESULT err;

  (void)id;
  /*
   * On insertion SDC initialization and FS mount.
   */
  if (sdcConnect(&SDCD1))
    return;

  err = f_mount(0, &SDC_FS);
  if (err != FR_OK) {
    sdcDisconnect(&SDCD1);
    return;
  }
  fs_ready = TRUE;
}

/*
 * Card removal event.
 */
static void RemoveHandler(eventid_t id) {

  (void)id;
  sdcDisconnect(&SDCD1);
  fs_ready = FALSE;
}


/*
 * Application entry point.
 */
int main(void) {

  unsigned int i = 0;

  static Thread *shelltp = NULL;
  static const evhandler_t evhndl[] = {
    InsertHandler,
    RemoveHandler
  };
  struct EventListener el0, el1;

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();
  /*
   * Initializes a serial-over-USB CDC driver.
   */
  sduObjectInit(&SDU1);


  /*
* Activate custom stuff
*/
  mypwmInit();
  myADCinit();

  /*
* Creates the blinker thread.
*/
  startBlinker();



  myUSBinit();


  //sduStart(&SDU1, &serusbcfg);


  /*
   * Activates the USB driver and then the USB bus pull-up on D+.
   * Note, a delay is inserted in order to not have to disconnect the cable
   * after a reset.
   */
//  usbDisconnectBus(serusbcfg.usbp);
//  chThdSleepMilliseconds(1500);
//  usbStart(serusbcfg.usbp, &usbcfg);
//  usbConnectBus(serusbcfg.usbp);

  /*
   * Shell manager initialization.
   */
//  shellInit();



  sdcStart(&SDCD1, NULL);


  /*
   * Activates the card insertion monitor.
   */
  tmr_init(&SDCD1);


  /*
   * Activates the serial driver 6 using the driver default configuration.
   */
  sdStart(&SD6, NULL);
  palSetPadMode(GPIOC, 6, PAL_MODE_ALTERNATE(8));
  palSetPadMode(GPIOC, 7, PAL_MODE_ALTERNATE(8));
  /*
   * Creates the blinker thread.
   */
  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);

//TODO DHCP

  struct lwipthread_opts   ip_opts;
static       uint8_t      macAddress[6]    =     {0xC2, 0xAF, 0x51, 0x03, 0xCF, 0x46};
struct ip_addr ip, gateway, netmask;
IP4_ADDR(&ip,      192, 168, 1, 20);
IP4_ADDR(&gateway, 192, 168, 1, 1);
IP4_ADDR(&netmask, 255, 255, 255, 0);
ip_opts.address    = ip.addr;
ip_opts.netmask    = netmask.addr;
ip_opts.gateway    = gateway.addr;
ip_opts.macaddress = macAddress;

  /*
   * Creates the LWIP threads (it changes priority internally).
   */
  chThdCreateStatic(wa_lwip_thread, LWIP_THREAD_STACK_SIZE, NORMALPRIO + 1,
                    lwip_thread, &ip_opts);

//  /*
//   * Creates the HTTP thread (it changes priority internally).
//   */
//  chThdCreateStatic(wa_http_server, sizeof(wa_http_server), NORMALPRIO + 1,
//                    http_server, NULL);

  /*
   * Creates the HTTP thread (it changes priority internally).
   */
  chThdCreateStatic(wa_socket_server, sizeof(wa_socket_server), NORMALPRIO + 1,
                    server_thread, &commands);


  netbios_init();


  /*
   * Normal main() thread activity, in this demo it does nothing except
   * sleeping in a loop and listen for events.
   */
  chEvtRegister(&inserted_event, &el0, 0);
  chEvtRegister(&removed_event, &el1, 1);
  while (TRUE) {
    i++;
    if (!shelltp && (SDU1.config->usbp->state == USB_ACTIVE))
    {
      shelltp = shellCreate(&shell_cfg1, SHELL_WA_SIZE, NORMALPRIO);
      chprintf((BaseSequentialStream *)&SDU1, "Serial USB: %d\r\n", i);
    }else if (chThdTerminated(shelltp)) {
      chThdRelease(shelltp);    /* Recovers memory of the previous shell.   */
      shelltp = NULL;           /* Triggers spawning of a new shell.        */
    }
//    if (palReadPad(GPIOA, GPIOA_BUTTON) != 0) {
//    }
    chEvtDispatch(evhndl, chEvtWaitOneTimeout(ALL_EVENTS, MS2ST(500)));
  }
}
