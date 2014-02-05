/*
 * server.h
 *
 *  Created on: Feb 5, 2014
 *      Author: arnon
 */

#ifndef SERVER_H_
#define SERVER_H_
    #include "netstream.h"
    #include "lwip/api.h"

    #include "ch.h"
    #include "hal.h"
    #include "shell.h"

#define SHELL_WA_SIZE   THD_WA_SIZE(2048)

#ifndef SOCKET_THREAD_STACK_SIZE
#define SOCKET_THREAD_STACK_SIZE   1024
#endif

#ifndef SOCKET_THREAD_PORT
#define SOCKET_THREAD_PORT         8080
#endif

#ifndef SOCKET_THREAD_PRIORITY
#define SOCKET_THREAD_PRIORITY     (LOWPRIO + 2)
#endif

extern WORKING_AREA(wa_socket_server, SOCKET_THREAD_STACK_SIZE);


#ifdef __cplusplus
extern "C" {
#endif
  msg_t server_thread(void *arg);
#ifdef __cplusplus
}
#endif

#endif /* SERVER_H_ */
