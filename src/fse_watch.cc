/*
** © 2014 by Philipp Dunkel <pip@pipobscure.com>
** Licensed under MIT License.
*/

#include "fse_watch.h"
#include "fse_native.h"

using namespace fse;

void Watcher::Init (v8::Handle<v8::Object> exports) {
  v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Watcher::New);
  tpl->SetClassName(NanSymbol("FSEvents"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  NODE_SET_PROTOTYPE_METHOD(tpl, "stop", Watcher::Stop);

  exports->Set(v8::String::NewSymbol("FSEvents"), tpl->GetFunction());
}

NAN_METHOD(Watcher::New) {
  NanScope();
  char* path = NanFromV8String(args[0], Nan::UTF8, NULL, NULL, 0, v8::String::NO_OPTIONS);
  Native* obj = new Native(path, new NanCallback(args[1].As<v8::Function>()));
  obj->wrap(args.This());

  NanReturnValue(args.This());
}

NAN_METHOD(Watcher::Stop) {
  NanScope();
  Native* native = node::ObjectWrap::Unwrap<Native>(args.This());
  native->stop();
  NanReturnUndefined();
}
