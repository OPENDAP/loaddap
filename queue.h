
#ifndef _QUEUE_H
#define _QUEUE_H

extern int queread(long queue_id, char *s, int msgtype);
extern int quewrite(long queue_id, char *s, int msgtype);
extern long queconn(key_t *msg_qu_key);
extern void questop(long queue_id);
extern long questart(key_t *msg_qu_key);

#endif /* _QUEUE_H */
