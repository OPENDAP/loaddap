
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

/* 
 * $Log: queue.c,v $
 * Revision 1.2  2003/10/23 18:34:02  dan
 * Changed config include to config_writedap.h from config_writeval.h
 * This is to remain consistent with the renaming used from loaddods
 * to loaddap.  To support nested sequences writeval was modified
 * to send an end-of-sequence marker to delimit sequence instances.
 *
 * Revision 1.1.1.1  2003/10/22 19:43:35  dan
 * Version of the Matlab CommandLine client which uses Matlab Structure
 * variables to maintain the shape of the underlying DODS data.
 *
 * Revision 1.6  2000/06/15 00:01:33  jimg
 * Merged with 3.1.5
 *
 * Revision 1.5.8.1  2000/06/14 20:46:01  jimg
 * Fixed an error in the instrumentation; a variable was misspelled.
 *
 * Revision 1.5  1999/04/30 17:06:58  jimg
 * Merged with no-gnu and release-2-24
 *
 * Revision 1.4.28.1  1999/04/20 15:55:57  jimg
 * Fixed copyright and log comments.
 *
 * Revision 1.4  1996/10/29 21:44:42  jimg
 * Added string.h to included files
 *
 * Revision 1.3  1996/10/23 23:57:16  jimg
 * Added MAX_QUEUE_LEN constant.
 * Fixed up error messages.
 *
 * Revision 1.2  1996/10/07 21:08:58  jimg
 * Fixed the return values; -1 for error and 0 or greater for success. This
 * is true for all the functions that return a result.
 *
 * Revision 1.1  1996/10/07 19:10:31  jimg
 * Created from code in urlqueue and loaddods.
 */

static char id[]={"$Id: queue.c,v 1.2 2003/10/23 18:34:02 dan Exp $"};

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <sys/ipc.h>
#include <sys/msg.h>

#include "debug.h"

#include "config_writedap.h"
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
