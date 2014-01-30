/*
** © 2014 by Philipp Dunkel <pip@pipobscure.com>
** Licensed under MIT License.
*/

#ifndef fse_watch_h
#define fse_watch_h

#include "fse_native.h"
#include "nan.h"


namespace fse {
  class Watcher {
    public:
      static void Init (v8::Handle<v8::Object> exports);
    private:
      static NAN_METHOD(New);
      static NAN_METHOD(Stop);
  };
}

NODE_MODULE(fse, fse::Watcher::Init)

#endif // fse_watch_h
