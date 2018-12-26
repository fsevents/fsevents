#ifndef __loop_h
#define __loop_h

#include <stdlib.h>
#include <limits.h>

typedef struct {
  unsigned long long id;
  char path[PATH_MAX];
  unsigned int flags;
} fse_event_t;

typedef void (*fse_event_handler_t)(void *context, size_t numevents, fse_event_t *events);
typedef void (*fse_thread_hook_t)(void *context);
typedef struct fse_watcher_s* fse_watcher_t;

void fse_init();
fse_watcher_t fse_alloc();
void fse_free(fse_watcher_t watcherp);
void fse_watch(const char *path, fse_event_handler_t handler, void *context, fse_thread_hook_t hookstart, fse_thread_hook_t hookend, fse_watcher_t watcher_p);
void fse_unwatch(fse_watcher_t watcher);
void *fse_context_of(fse_watcher_t watcher);
#endif
