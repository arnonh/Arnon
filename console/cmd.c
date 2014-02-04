/*
 * status.c
 *
 *  Created on: Nov 21, 2013
 *      Author: freyjed
 */

#include "ch.h"
#include "hal.h"
#include "chprintf.h"
//#include "usbcfg.h"
#include "shell.h"
#include "ansi.h"
#include "cmd.h"

#include "ff.h"



//void cmd_mem(BaseSequentialStream *chp, int argc, char *argv[]) {
//  (void)argv;
//  (void)argc;
//  size_t n;
//  n=chCoreStatus();
//  chprintf(chp, "Free: ");
//  if (n<500) {
//	  chprintf(chp, "%s", "\e[1;30;41m");
//  }
//  chprintf(chp,"%9u",n);
//  if (n<500) {
//	  chprintf(chp, "%s", "\e[0m\r\n");
//  }
//  chprintf(chp,"\r\n");
//}

void cmd_threads(BaseSequentialStream *chp, int argc, char *argv[]) {
//  static const char *states[] = {THD_STATE_NAMES};
//  Thread *tp;
//
//  (void)argv;
//  if (argc > 0)
//  {
//     chprintf(chp, "Usage: threads\r\n");
//     return;
//  }
//  chprintf(chp, "%10s %10s %10s %6s %6s %11s %7s\r\n",
//           "name", "add", "stack", "prio", "refs", "state", "time");
//  tp = chRegFirstThread();
//  do
//  {
//     chprintf(chp, "%10s %.10lx %.10lx %6lu %6lu %11s %7lu\r\n",
//              (uint32_t)tp->p_name, (uint32_t)tp, (uint32_t)tp->p_ctx.r13,
//              (uint32_t)tp->p_prio, (uint32_t)(tp->p_refs - 1),
//              states[tp->p_state], (uint32_t)tp->p_time);
//     tp = chRegNextThread(tp);
//  } while (tp != NULL);
  static const char *states[] = {THD_STATE_NAMES};
  Thread *tp;

  (void)argv;
  (void)argc;

  chprintf(chp, "%s", "\e[1;30;41m");
  chprintf(chp, "%15s%10s%10s%15s","name","priority","state","time");
  chprintf(chp, "%s", "\e[0m\r\n");
  tp = chRegFirstThread();
  do {
    chprintf(chp, "%15s%10lu%10s%15lu\r\n",
             (uint32_t)tp->p_name,
             (uint32_t)tp->p_prio,
             states[tp->p_state],
             (uint32_t)tp->p_time);
    tp = chRegNextThread(tp);
  } while (tp != NULL);
}

void cmd_status(BaseSequentialStream *chp, int argc, char *argv[]) {
	Thread *tp = chThdCreateFromHeap(NULL, THD_WA_SIZE(128), NORMALPRIO+1, statusRefresh, chp);
	(void)argv;
	(void)argc;
	uint8_t c;
	while (TRUE) {
		if (chSequentialStreamRead(chp, &c, 1) == 0) {
			chThdTerminate(tp);
			chThdWait(tp);
			return;
		}
		if (c == 4 || c == 3) {
			chprintf(chp,"Killing Threads.");
			chThdTerminate(tp);
			chThdWait(tp);
			ansiClear(chp);
			return;
		}
	}
}

msg_t statusRefresh(BaseSequentialStream *chp) {
	systime_t time;
	chRegSetThreadName("statusRefresh");
	while (!chThdShouldTerminate()) {
		time = chTimeNow() + MS2ST(1000);
		//
		ansiClear(chp);
		cmd_mem(chp,0,NULL);
		cmd_threads(chp,0,NULL);
		//
		chThdSleepUntil(time);
	}
	return (msg_t)NULL;
}






