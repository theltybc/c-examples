#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <assert.h>

struct mymsgbuf {
  long mtype; /* Message type */
  int some1;
  double some2;
} msg;

int open_queue(key_t keyval);
int remove_queue(int qid);
int get_qid(void);
int send_message(int qid, struct mymsgbuf *qbuf);
int read_message(int qid, long type, struct mymsgbuf *qbuf);

#define check_error(val, msg) \
  do {                        \
    if ((val) == -1) {        \
      perror((msg));          \
      abort();                \
    }                         \
  } while (0);

int main() {

  int qid = get_qid();

  /* Load up the message with arbitrary test data */
  msg.mtype = 9; /* Message type must be a positive number! */
  msg.some1 = 99;
  msg.some2 = 999.0;

  check_error(send_message(qid, &msg), "send message");

  msg.mtype = 0;
  msg.some1 = 0;
  msg.some2 = 0;

  check_error(read_message(qid, 9, &msg), "read message");

  assert(msg.mtype == 9);
  assert(msg.some1 == 99);
  assert(msg.some2 == 999.0);

  check_error(remove_queue(qid), "remove queue");

  return EXIT_SUCCESS;
}

int get_qid(void) {
  int qid;
  key_t msgkey;

  /* Generate our IPC key value */
  msgkey = ftok(".", 'm');
  check_error(msgkey, "Generate our IPC key");

  /* Open/create the queue */
  qid = open_queue(msgkey);
  check_error(qid, "Open/create the queue");
  return qid;
}

int open_queue(key_t keyval) {
  return msgget(keyval, IPC_CREAT | IPC_EXCL | 0660); //
}

int remove_queue(int qid) {
  return msgctl(qid, IPC_RMID, NULL);
}

int send_message(int qid, struct mymsgbuf *qbuf) {
  int length = sizeof(struct mymsgbuf) - sizeof(long);
  return msgsnd(qid, qbuf, length, 0);
}

int read_message(int qid, long type, struct mymsgbuf *qbuf) {
  int length = sizeof(struct mymsgbuf) - sizeof(long);
  return msgrcv(qid, qbuf, length, type, 0);
}

int peek_message(int qid, long type) {
  if (msgrcv(qid, NULL, 0, type, IPC_NOWAIT) == -1) {
    return errno == E2BIG;
  }
  return 0;
}
