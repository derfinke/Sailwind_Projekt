/**
 * \file tcp_server.c
 * @date Sep 17, 2023
 * @brief tcp handling
 */

#include "tcp.h"
#include <string.h>
#include "lwip.h"
#include "cJSON.h"
#include "REST.h"
#include "stdint.h"

#define REST_API_PORT 2375

static const uint32_t tcp_server_ip[4] = { 192, 168, 0, 123 };
enum tcp_server_states {
  ES_NONE = 0,
  ES_ACCEPTED,
  ES_RECEIVED,
  ES_CLOSING
};

struct tcp_server_struct {
  uint8_t state;
  uint8_t retries;
  struct tcp_pcb *pcb;
  struct pbuf *p;
};

/**
 * @brief  This function is the implementation of tcp_accept LwIP callback
 * @param  arg: not used
 * @param  newpcb: pointer on tcp_pcb struct for the newly created tcp connection
 * @param  err: not used
 * @retval err_t: error status
 */
static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err);

/**
 * @brief  This function handles incoming tcp packets and passes them to the REST handler
 * @param  arg: not used
 * @param  newpcb: pointer on tcp_pcb connection
 * @param  err: not used
 * @retval none
 */
static void tcp_server_handle(struct tcp_pcb *tpcb,
                              struct tcp_server_struct *es);

/**
 * @brief  This function is the implementation for tcp_recv LwIP callback
 * @param  arg: pointer on a argument for the tcp_pcb connection
 * @param  tpcb: pointer on the tcp_pcb connection
 * @param  pbuf: pointer on the received pbuf
 * @param  err: error information regarding the reveived pbuf
 * @retval err_t: error code
 */
static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p,
                             err_t err);

/**
 * @brief  This function implements the tcp_err callback function (called
 *         when a fatal tcp_connection error occurs.
 * @param  arg: pointer on argument parameter
 * @param  err: not used
 * @retval None
 */
static void tcp_server_error(void *arg, err_t err);

/**
 * @brief  This function implements the tcp_poll LwIP callback function
 * @param  arg: pointer on argument passed to callback
 * @param  tpcb: pointer on the tcp_pcb for the current tcp connection
 * @retval err_t: error code
 */
static err_t tcp_server_poll(void *arg, struct tcp_pcb *tpcb);

/**
 * @brief  This function implements the tcp_sent LwIP callback (called when ACK
 *         is received from remote host for sent data)
 * @param  None
 * @retval None
 */
static err_t tcp_server_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);

/**
 * @brief  This function is used to send data for tcp connection
 * @param  tpcb: pointer on the tcp_pcb connection
 * @param  es: pointer on echo_state structure
 * @retval None
 */
static void tcp_server_send(struct tcp_pcb *tpcb, struct tcp_server_struct *es);

/**
 * @brief  This functions closes the tcp connection
 * @param  tcp_pcb: pointer on the tcp connection
 * @param  es: pointer on echo_state structure
 * @retval None
 */
static void tcp_server_connection_close(struct tcp_pcb *tpcb,
                                        struct tcp_server_struct *es);

void tcp_server_init(void) {
  /* 1. create new tcp pcb */
  struct tcp_pcb *tpcb;

  tpcb = tcp_new();

  err_t err;

  /* 2. bind _pcb to port 7 ( protocol) */
  ip_addr_t myIPADDR;
  IP_ADDR4(&myIPADDR, tcp_server_ip[0], tcp_server_ip[1], tcp_server_ip[2],
           tcp_server_ip[3]);
  err = tcp_bind(tpcb, &myIPADDR, 2375);

  if (err == ERR_OK) {
    /* 3. start tcp listening for _pcb */
    tpcb = tcp_listen(tpcb);

    /* 4. initialize LwIP tcp_accept callback function */
    tcp_accept(tpcb, tcp_server_accept);
  } else {
    /* deallocate the pcb */
    memp_free(MEMP_TCP_PCB, tpcb);
  }
}

static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err) {
  err_t ret_err;
  struct tcp_server_struct *tcp_server;

  LWIP_UNUSED_ARG(arg);
  LWIP_UNUSED_ARG(err);

  /* set priority for the newly accepted tcp connection newpcb */
  tcp_setprio(newpcb, TCP_PRIO_MIN);

  /* allocate structure es to maintain tcp connection information */
  tcp_server = (struct tcp_server_struct*) mem_malloc(
      sizeof(struct tcp_server_struct));
  if (tcp_server != NULL) {
    tcp_server->state = ES_ACCEPTED;
    tcp_server->pcb = newpcb;
    tcp_server->retries = 0;
    tcp_server->p = NULL;

    /* pass newly allocated server structure as argument to newpcb */
    tcp_arg(newpcb, tcp_server);

    /* initialize lwip tcp_recv callback function for newpcb  */
    tcp_recv(newpcb, tcp_server_recv);

    /* initialize lwip tcp_err callback function for newpcb  */
    tcp_err(newpcb, tcp_server_error);

    /* initialize lwip tcp_poll callback function for newpcb */
    tcp_poll(newpcb, tcp_server_poll, 0);

    ret_err = ERR_OK;
  } else {
    /*  close tcp connection */
    tcp_server_connection_close(newpcb, tcp_server);
    /* return memory error */
    ret_err = ERR_MEM;
  }
  return ret_err;
}

static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p,
                             err_t err) {
  struct tcp_server_struct *tcp_server;
  err_t ret_err;

  LWIP_ASSERT("arg != NULL", arg != NULL);

  tcp_server = (struct tcp_server_struct*) arg;

  /* if empty tcp frame from client => close connection */
  if (p == NULL) {
    /* remote host closed connection */
    tcp_server->state = ES_CLOSING;
    if (tcp_server->p == NULL) {
      /* we're done sending, close connection */
      tcp_server_connection_close(tpcb, tcp_server);
    } else {
      /* we're not done yet */
      /* acknowledge received packet */
      tcp_sent(tpcb, tcp_server_sent);

      /* send remaining data*/
      tcp_server_send(tpcb, tcp_server);
    }
    ret_err = ERR_OK;
  }
  /* else : a non empty frame was received from client but for some reason err != ERR_OK */
  else if (err != ERR_OK) {
    /* free received pbuf*/
    if (p != NULL) {
      tcp_server->p = NULL;
      pbuf_free(p);
    }
    ret_err = err;
  } else if (tcp_server->state == ES_ACCEPTED) {
    /* first data chunk in p->payload */
    tcp_server->state = ES_RECEIVED;

    /* store reference to incoming pbuf (chain) */
    tcp_server->p = p;

    /* initialize LwIP tcp_sent callback function */
    tcp_sent(tpcb, tcp_server_sent);

    /* handle the received data */
    tcp_server_handle(tpcb, tcp_server);

    ret_err = ERR_OK;
  } else if (tcp_server->state == ES_RECEIVED) {
    /* more data received from client and previous data has been already sent*/
    if (tcp_server->p == NULL) {
      tcp_server->p = p;

      /* handle the received data */
      tcp_server_handle(tpcb, tcp_server);
    } else {
      struct pbuf *ptr;

      /* chain pbufs to the end of what we recv'ed previously  */
      ptr = tcp_server->p;
      pbuf_chain(ptr, p);
    }
    ret_err = ERR_OK;
  } else if (tcp_server->state == ES_CLOSING) {
    /* odd case, remote side closing twice, trash data */
    tcp_recved(tpcb, p->tot_len);
    tcp_server->p = NULL;
    pbuf_free(p);
    ret_err = ERR_OK;
  } else {
    /* unknown es->state, trash data  */
    tcp_recved(tpcb, p->tot_len);
    tcp_server->p = NULL;
    pbuf_free(p);
    ret_err = ERR_OK;
  }
  return ret_err;
}

static void tcp_server_handle(struct tcp_pcb *tpcb,
                              struct tcp_server_struct *tcp_server) {
  char buf[300];

  REST_request_handler((char*) tcp_server->p->payload, buf);

  tcp_server->p->payload = (void*) buf;
  tcp_server->p->tot_len = (tcp_server->p->tot_len - tcp_server->p->len)
      + strlen(buf);
  tcp_server->p->len = strlen(buf);

  tcp_server_send(tpcb, tcp_server);
}

static void tcp_server_error(void *arg, err_t err) {
  struct tcp_server_struct *tcp_server;

  LWIP_UNUSED_ARG(err);

  tcp_server = (struct tcp_server_struct*) arg;
  if (tcp_server != NULL) {
    /*  free es structure */
    mem_free(tcp_server);
  }
}

static err_t tcp_server_poll(void *arg, struct tcp_pcb *tpcb) {
  err_t ret_err;
  struct tcp_server_struct *tcp_server;

  tcp_server = (struct tcp_server_struct*) arg;
  if (tcp_server != NULL) {
    if (tcp_server->p != NULL) {
      tcp_sent(tpcb, tcp_server_sent);
      /* there is a remaining pbuf (chain) , try to send data */
      tcp_server_send(tpcb, tcp_server);
    } else {
      /* no remaining pbuf (chain)  */
      if (tcp_server->state == ES_CLOSING) {
        /*  close tcp connection */
        tcp_server_connection_close(tpcb, tcp_server);
      }
    }
    ret_err = ERR_OK;
  } else {
    /* nothing to be done */
    tcp_abort(tpcb);
    ret_err = ERR_ABRT;
  }
  return ret_err;
}

static err_t tcp_server_sent(void *arg, struct tcp_pcb *tpcb, u16_t len) {
  struct tcp_server_struct *tcp_server;

  LWIP_UNUSED_ARG(len);

  tcp_server = (struct tcp_server_struct*) arg;
  tcp_server->retries = 0;

  if (tcp_server->p != NULL) {
    /* still got pbufs to send */
    tcp_sent(tpcb, tcp_server_sent);
    tcp_server_send(tpcb, tcp_server);
  } else {
    /* if no more data to send and client closed connection*/
    if (tcp_server->state == ES_CLOSING)
      tcp_server_connection_close(tpcb, tcp_server);
  }
  return ERR_OK;
}

static void tcp_server_send(struct tcp_pcb *tpcb,
                            struct tcp_server_struct *tcp_server) {
  struct pbuf *ptr;
  err_t wr_err = ERR_OK;

  while ((wr_err == ERR_OK) && (tcp_server->p != NULL)
      && (tcp_server->p->len <= tcp_sndbuf(tpcb))) {

    /* get pointer on pbuf from es structure */
    ptr = tcp_server->p;

    /* enqueue data for transmission */
    wr_err = tcp_write(tpcb, ptr->payload, ptr->len, 1);

    if (wr_err == ERR_OK) {
      u16_t plen;
      u8_t freed;

      plen = ptr->len;

      /* continue with next pbuf in chain (if any) */
      tcp_server->p = ptr->next;

      if (tcp_server->p != NULL) {
        /* increment reference count for es->p */
        pbuf_ref(tcp_server->p);
      }

      /* chop first pbuf from chain */
      do {
        /* try hard to free pbuf */
        freed = pbuf_free(ptr);
      } while (freed == 0);
      /* we can read more data now */
      tcp_recved(tpcb, plen);
    } else if (wr_err == ERR_MEM) {
      /* we are low on memory, try later / harder, defer to poll */
      tcp_server->p = ptr;
    } else {
      /* other problem ?? */
    }
  }
}

static void tcp_server_connection_close(struct tcp_pcb *tpcb,
                                        struct tcp_server_struct *tcp_server) {

  /* remove all callbacks */
  tcp_arg(tpcb, NULL);
  tcp_sent(tpcb, NULL);
  tcp_recv(tpcb, NULL);
  tcp_err(tpcb, NULL);
  tcp_poll(tpcb, NULL, 0);

  /* delete es structure */
  if (tcp_server != NULL) {
    mem_free(tcp_server);
  }

  /* close tcp connection */
  tcp_close(tpcb);
}
