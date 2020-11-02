#ifndef __loop_h
#define __loop_h

#include <stdlib.h>
#include <limits.h>

#include "CoreServices/CoreServices.h"

#include <stdio.h>
#define DEBUG(txt, num) printf("DEBUG: %s -> %i\n", txt, num)

typedef struct
{
  pthread_t thread;
  CFRunLoopRef loop;
  pthread_mutex_t lock;
  pthread_cond_t init;
} fse_loop_t;

typedef struct
{
  unsigned long long id;
  char path[PATH_MAX];
  unsigned int flags;
} fse_event_t;
typedef void (*fse_event_handler_t)(void *context, size_t numevents, fse_event_t *events);
typedef void (*fse_thread_hook_t)(void *context);

struct fse_watcher_s
{
  char path[PATH_MAX];
  FSEventStreamRef stream;
  fse_event_handler_t handler;
  fse_thread_hook_t hookend;
  void *context;
  fse_loop_t *global;
};

typedef struct fse_watcher_s *fse_watcher_t;

void fse_init(fse_loop_t *fsevents);
void fse_destroy(fse_loop_t *fsevents);

fse_watcher_t fse_alloc();
void fse_free(fse_watcher_t watcherp);
void fse_watch(const char *path, fse_event_handler_t handler, void *context, fse_thread_hook_t hookstart, fse_thread_hook_t hookend, fse_watcher_t watcher_p);
void fse_unwatch(fse_watcher_t watcher);
void *fse_context_of(fse_watcher_t watcher);
#endif
