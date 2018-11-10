/*
** © 2018 by Philipp Dunkel, Ben Noordhuis, Elan Shankar
** Licensed under MIT License.
*/

#ifndef __runner
#define __runner

#include <pthread.h>
#include <limits.h>
#include "CoreFoundation/CoreFoundation.h"
#include "CoreServices/CoreServices.h"
#define NAPI_EXPERIMENTAL
#include <node_api.h>
#include <uv.h>

#ifdef NDEBUG
#define CHECK(x) do { if (!(x)) abort(); } while (0)
#else
#define CHECK assert
#endif

typedef struct {
  char path[PATH_MAX];

  napi_env env;
  napi_ref callback;

  CFRunLoopRef loop;
  uv_thread_t thread;
  uv_async_t *async;
  uv_mutex_t mutex;

  void *head;
  void *tail;
} fse_t;

napi_value start(napi_env env, const char (*path)[PATH_MAX], napi_value callback);
void stop(fse_t *instance);

#endif
