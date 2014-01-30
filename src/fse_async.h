/*
** © 2014 by Philipp Dunkel <pip@pipobscure.com>
** Licensed under MIT License.
*/

#ifndef fse_async_h
#define fse_async_h

#include "CoreFoundation/CoreFoundation.h"
#include "fse_types.h"
#include "nan.h"

namespace fse {
  class Async {
    public:
      Async(void *data, notify_callback handler);
      void trigger();
      void destroy();
      
    protected:
      static void propagate(uv_async_t *handle, int status);
      void notify();

      uv_async_t async;
      void *data;
      notify_callback handler;
  };
}

#endif // fse_async_h
