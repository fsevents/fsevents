/*
** © 2014 by Philipp Dunkel <pip@pipobscure.com>
** Licensed under MIT License.
*/

void FSEvents::emitEvent(const char *path, UInt32 flags, UInt64 id) {
  Napi::HandleScope handle_scope(env);
  Napi::Object object = handle();
  Napi::Value key = Napi::String::New(env, "handler");
  Napi::FunctionReference handler((object).Get(key.To<Napi::Function>()));
  Napi::Value argv[] = {
    Napi::String::New(env, path),
    Napi::Number::New(env, flags),
    Napi::Number::New(env, id)
  };
  handler.Call(3, argv, &async_resource);
}

Napi::Value FSEvents::New(const Napi::CallbackInfo& info) {
  std::string path = info[0].As<Napi::String>();

  FSEvents *fse = new FSEvents(*path);
  fse->Wrap(info.This());
  (info.This()).Set(Napi::String::New(env, "handler"), info[1]);

  return info.This();
}

Napi::Value FSEvents::Stop(const Napi::CallbackInfo& info) {
  FSEvents* fse = this;

  fse->threadStop();
  fse->asyncStop();

  return info.This();
}

Napi::Value FSEvents::Start(const Napi::CallbackInfo& info) {
  FSEvents* fse = this;
  fse->asyncStart();
  fse->threadStart();

  return info.This();
}
