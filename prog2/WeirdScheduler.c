#include <lwp.h>
#include <stdlib.h>
#include <stdio.h>
#include "lwp.h"

static thread qhead=NULL;

static void s_admit(thread new) {
  qhead = new;
}

static void s_remove(thread victim) {
  qhead = NULL;
}

static thread s_next() {
  thread res;
  res = qhead;
  qhead = NULL;
  return res;
}

static struct scheduler publish = {NULL,NULL,s_admit,s_remove,s_next};
scheduler Weird = &publish;

