/*
// -*- mode: c++; c-basic-offset:4 -*-

// This file is part of loaddap.


// Copyright (c) 2005 OPeNDAP, Inc.
// Author: James Gallagher <jgallagher@opendap.org>
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// 
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// You can contact OPeNDAP, Inc. at PO Box 112, Saunderstown, RI. 02874-0112.
*/

/* (c) COPYRIGHT URI/MIT 1996
   Please read the full copyright statement in the file COPYRIGHT.

   Authors:
      grf		Glenn Fleirl (glenn@lake.mit.edu)
      jhrg,jimg		James Gallagher (jgallagher@gso.uri.edu)
*/

/* 
   System V queue software. Also works with SunOS 4.
   These functions return -1 to indicate an error and 0 or some positive
   value (e.g., the queue id) to indicate success.
*/

#include "config.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "debug.h"

#include "queue.h"

typedef struct {
    long mtype;
    char mtext[MAX_QUEUE_LEN];
} message;

/* make queue */

long
questart(key_t *msg_qu_key)
{
    long queue_id;

    if ((queue_id = msgget(*msg_qu_key, 0777 | IPC_CREAT | IPC_EXCL)) == -1) {
	if (errno == EEXIST)
	    return 0;		/* is this correct? jhrg */
	perror("DODS: Cannot create message queue\n");
	return -1;
    }

    return queue_id;
}

void
questop(long queue_id)
{
    if (msgctl(queue_id, IPC_RMID, 0) == -1)
	perror("DODS: Could not stop queue");
}

/* Connect to queue */

long
queconn(key_t *msg_qu_key)
{
    int iret;
    int queue_size;
    long queue_id;
    struct msqid_ds buf;

    if ((queue_id = msgget(*msg_qu_key, 0666)) == -1) {
	perror("DODS: Cannot locate message queue");
	return -1;
    }

    iret = msgctl(queue_id, IPC_STAT, &buf);
    if (iret) {
	perror("DODS: Queue size error");
	return -1;
    }

    queue_size = buf.msg_qbytes;
    if (queue_size > MAX_QUEUE_LEN)
	queue_size = MAX_QUEUE_LEN;

    DBG(fprintf(stderr, "queue size %d\n", queue_size));

    return queue_id;
}

/* `s' must not be larger than MAX_QUEUE_LEN! */

int
quewrite(long queue_id, char *s, int msgtype)
{
    message msg1;

    msg1.mtype = msgtype;
    if (strlen(s) > MAX_QUEUE_LEN) {
	perror("DODS: Unable to send the message because it is too long!");
	return -1;
    }
    strcpy(msg1.mtext, s);

    if (msgsnd(queue_id, &msg1, strlen(s) + 1, 0) == -1) {
	perror("DODS: Not able to send the message");
	return -1;
    }

    return 0;
}

int 
queread(long queue_id, char *s, int msgtype)
{
    message msg1;

    if (msgrcv(queue_id, &msg1, sizeof(msg1.mtext), msgtype, 0) == -1) {
	perror("DODS: Not able to receive the message");
	return -1;
    }

    strcpy(s, msg1.mtext);
    return 0;
}
