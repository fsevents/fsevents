/*
** © 2014 by Philipp Dunkel <pip@pipobscure.com>
** Licensed under MIT License.
*/

void FSEvents::emitEvent(const char *path, UInt32 flags, UInt64 id) {
  Napi::Env env = Env();
  Napi::HandleScope scope(env);

  handler.Call(env.Undefined(), {
    Napi::String::New(env, path),
    Napi::Number::New(env, flags),
    Napi::Number::New(env, id)
  });
}

Napi::Value FSEvents::Stop(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  FSEvents* fse = this;

  fse->threadStop();
  fse->asyncStop();

  return info.This();
}

Napi::Value FSEvents::Start(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  FSEvents* fse = this;
  fse->asyncStart();
  fse->threadStart();

  return info.This();
}
