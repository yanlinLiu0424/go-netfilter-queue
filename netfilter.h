/*
   Copyright 2014 Krishna Raman <kraman@gmail.com>

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

#ifndef _NETFILTER_H
#define _NETFILTER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <math.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <linux/types.h>
#include <linux/socket.h>
#include <linux/netfilter.h>
#include <libnetfilter_queue/libnetfilter_queue.h>

extern void go_callback(
    uint32_t id,
    unsigned char* data,
    int len,
    u_int32_t mark,
    u_int32_t idx,
    struct nfq_q_handle* qh,
    u_int32_t netidx
);

static int nf_callback(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg, struct nfq_data *nfa, void *cb_func){
    uint32_t id = -1;
    struct nfqnl_msg_packet_hdr *ph = NULL;
    unsigned char *buffer = NULL;
    int ret = 0;
    int mark = 0;
    u_int32_t idx;
    u_int32_t netidx=0;

    ph = nfq_get_msg_packet_hdr(nfa);
    id = ntohl(ph->packet_id);

    ret = nfq_get_payload(nfa, &buffer);
    idx = (uint32_t)((uintptr_t)cb_func);
    netidx = nfq_get_outdev(nfa);
    if (netidx==0){
         netidx = nfq_get_indev(nfa);
    }

    mark = nfq_get_nfmark(nfa);

    go_callback(id, buffer, ret, mark, idx, qh, netidx);
    return 0;
}

static inline struct nfq_q_handle* CreateQueue(struct nfq_handle *h, u_int16_t queue, u_int32_t idx)
{
    return nfq_create_queue(h, queue, &nf_callback, (void*)((uintptr_t)idx));
}

static inline int Run(struct nfq_handle *h, int fd)
{
    char buf[4096] __attribute__ ((aligned));
    int rv;

    int opt = 1;
    setsockopt(fd, SOL_NETLINK, NETLINK_NO_ENOBUFS, &opt, sizeof(int));

    while ((rv = recv(fd, buf, sizeof(buf), 0)) && rv >= 0) {
        nfq_handle_packet(h, buf, rv);
    }

    return errno;
}

#endif
