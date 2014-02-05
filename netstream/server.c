


    #include "server.h"



/**
 * Stack area for the SOCKET thread.
 */
WORKING_AREA(wa_socket_server, SOCKET_THREAD_STACK_SIZE);


    /*
    * TCP server thread.
    */
    msg_t server_thread(void *arg) {
       //uint16_t port = *((uint16_t *) arg);
      ShellCommand *commands = arg;
       struct netconn *conn, *newconn;
       err_t err;

       chRegSetThreadName("server");

       /* Create a new TCP connection handle */
       conn = netconn_new(NETCONN_TCP);
       LWIP_ERROR("TCP server: invalid conn", (conn != NULL), return RDY_RESET;);

       /* Bind to a port. */
       netconn_bind(conn, NULL, SOCKET_THREAD_PORT);

       /* Listen for connections. */
       netconn_listen(conn);

       while (TRUE) {
          err = netconn_accept(conn, &newconn);
          if (err != ERR_OK)
             continue;

          /* Dynamic allocation to allow multiple shell instances. */
          NetStream * nsp = chHeapAlloc(NULL, sizeof(NetStream));
          ShellConfig * shell_cfgp = chHeapAlloc(NULL, sizeof(ShellConfig));

          if(nsp && shell_cfgp) {
             nsObjectInit(nsp);
             nsStart(nsp, newconn);

             shell_cfgp->sc_channel = (BaseSequentialStream *) nsp;
             shell_cfgp->sc_commands = commands;

             shellCreate(shell_cfgp, SHELL_WA_SIZE, NORMALPRIO - 1);
          }
       }
       return RDY_OK;
    }

