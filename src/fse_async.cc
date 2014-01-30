/*
** © 2014 by Philipp Dunkel <pip@pipobscure.com>
** Licensed under MIT License.
*/

#include "fse_async.h"
#include "CoreFoundation/CoreFoundation.h"

using namespace fse;

Async::Async(void *data, notify_callback handler): data(data), handler(handler) {
  async.data = this;
  uv_loop_t *loop = uv_default_loop();
  uv_async_init(loop, &async, &Async::propagate);
}

void Async::trigger() {
  uv_async_send(&async);
}

void destroyed(uv_handle_t* handle) {
  uv_async_t *async = (uv_async_t *)handle;
  Async *client = (Async *)async->data;
  delete client;
}

void Async::destroy() {
  uv_close((uv_handle_t *) &async, &destroyed);
}

void Async::propagate(uv_async_t *handle, int status) {
  uv_async_t *async = (uv_async_t *)handle;
  Async * client = (Async*)async->data;
  client->notify();
}

void Async::notify() {
  handler(data);
}
