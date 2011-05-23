#include <node.h>
#include <node_events.h>
#include <v8.h>
#include <ev.h>

#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <CoreServices/CoreServices.h>

#define MAXPATH 1024

typedef struct s_evt *p_evt;
struct s_evt {
	FSEventStreamEventFlags flags;
	FSEventStreamEventId evtid;
	char path[MAXPATH + 1];
	p_evt next;
};
static v8::Persistent<v8::FunctionTemplate> constructor_template;
namespace node_fsevents {
	using namespace v8;
	using namespace node;
	class NodeFSEvents : node::ObjectWrap {
		public:
			static v8::Handle<v8::Value> Shutdown(const v8::Arguments& args) {
			  HandleScope scope;
				NodeFSEvents *native = node::ObjectWrap::Unwrap<NodeFSEvents>(args.This());
				native->Shutdown();
			  return Undefined();
			}
			static void Initialize(v8::Handle<v8::Object> target) {
				HandleScope scope;
			  Local<FunctionTemplate> t = FunctionTemplate::New(NodeFSEvents::New);
			  constructor_template = Persistent<FunctionTemplate>::New(t);
			  constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
			  constructor_template->SetClassName(String::NewSymbol("FSEvents"));
				Local<Function> constructor = constructor_template->GetFunction();
				
				constructor->Set(String::New("kFSEventStreamEventFlagNone"), Integer::New(0x00000000));
				constructor->Set(String::New("kFSEventStreamEventFlagMustScanSubDirs"), Integer::New(0x00000001));
			  constructor->Set(String::New("kFSEventStreamEventFlagUserDropped"), Integer::New(0x00000002));
				constructor->Set(String::New("kFSEventStreamEventFlagKernelDropped"), Integer::New(0x00000004));
				constructor->Set(String::New("kFSEventStreamEventFlagEventIdsWrapped"), Integer::New(0x00000008));
				constructor->Set(String::New("kFSEventStreamEventFlagHistoryDone"), Integer::New(0x00000010));
				constructor->Set(String::New("kFSEventStreamEventFlagRootChanged"), Integer::New(0x00000020));
				constructor->Set(String::New("kFSEventStreamEventFlagMount"), Integer::New(0x00000040));
				constructor->Set(String::New("kFSEventStreamEventFlagUnmount"), Integer::New(0x00000080));
				
				target->Set(String::NewSymbol("FSEvents"), constructor);
			}
			static v8::Handle<v8::Value> New(const v8::Arguments& args) {
			  if (!args.IsConstructCall()) {
			    return FromConstructorTemplate(constructor_template, args);
			  }

			  HandleScope scope;

			  if (args.Length() < 1 || !args[0]->IsString()) {
			    return ThrowException(String::New("Bad arguments"));
			  }

				String::Utf8Value pathname(args[0]->ToString());

				FSEventStreamEventId since = kFSEventStreamEventIdSinceNow;
				if ((args.Length() > 1) && (args[1]->IsInt32())) {
					since = args[1]->Int32Value();
				}
			  NodeFSEvents *nativeobj = new NodeFSEvents(*pathname, since);
			  nativeobj->Wrap(args.Holder());
				NODE_SET_METHOD(args.Holder(), "stop", NodeFSEvents::Shutdown);
			  return args.This();
			}

			NodeFSEvents(const char *path, FSEventStreamEventId since) : ObjectWrap() {
				running=1;
				first = NULL;
				last = NULL;
				strncpy(pathname, path ? path : "/", MAXPATH);
				sinceWhen = since;
				pthread_mutex_init(&mutex, NULL);
				ev_async_init(&watcher, NodeFSEvents::Poll);
				watcher.data = this;
				ev_async_start (EV_DEFAULT_UC_ &watcher);
				pthread_create(&thread, NULL, &NodeFSEvents::Run, this);
			}

			~NodeFSEvents() {
				this->Shutdown();
			}
			void Shutdown() {
				if (running) {
					CFRunLoopStop(runLoop);
					pthread_join(thread, NULL);
					pthread_mutex_destroy(&mutex);
					ev_async_stop(EV_DEFAULT_UC_ &watcher);
				}
				running = 0;
			}
			
			static void *Run(void *data) {
				NodeFSEvents *This = (NodeFSEvents *)data;
				CFStringRef dir_names[1];
				dir_names[0] = CFStringCreateWithCString(NULL, This->pathname, kCFStringEncodingUTF8);
				CFArrayRef pathsToWatch = CFArrayCreate(NULL, (const void **)&dir_names, 1, NULL);
			  FSEventStreamContext context = { 0, data, NULL, NULL, NULL };
				FSEventStreamRef stream = FSEventStreamCreate(NULL, &NodeFSEvents::Event, &context, pathsToWatch, This->sinceWhen, (CFAbsoluteTime) 0.1, kFSEventStreamCreateFlagNone | kFSEventStreamCreateFlagWatchRoot);
				This->runLoop = CFRunLoopGetCurrent();
				FSEventStreamScheduleWithRunLoop(stream, This->runLoop, kCFRunLoopDefaultMode);
				FSEventStreamStart(stream);
				CFRunLoopRun();
				FSEventStreamStop(stream);
				FSEventStreamUnscheduleFromRunLoop(stream, This->runLoop, kCFRunLoopDefaultMode);
				FSEventStreamInvalidate(stream);
				FSEventStreamRelease(stream);
				pthread_exit(NULL);
			}
			static void Event(ConstFSEventStreamRef streamRef, void *userData, size_t numEvents, void *eventPaths, const FSEventStreamEventFlags eventFlags[], const FSEventStreamEventId eventIds[]) {
				NodeFSEvents *This = static_cast<NodeFSEvents*>(userData);
				char **paths = static_cast<char**>(eventPaths);
				size_t idx;
				p_evt item;
				pthread_mutex_lock(&(This->mutex));
				for (idx=0; idx < numEvents; idx++) {
					item = (p_evt)malloc(sizeof(struct s_evt));
					item->next = NULL;
					strncpy(item->path, paths[idx],MAXPATH);
					item->flags = eventFlags[idx];
					item->evtid = eventIds[idx];
					if (!This->first) This->first = item;
					if (This->last) {
						This->last->next = item;
					}
					This->last = item;
				}
				pthread_mutex_unlock(&(This->mutex));
				ev_async_send (EV_DEFAULT_UC_ &(This->watcher));
			}
			static void Poll(EV_P_ ev_async *w, int revents) {
				NodeFSEvents *This = static_cast<NodeFSEvents*>(w->data);
				HandleScope scope;
				TryCatch try_catch;
				Local<Value> callback_v = This->handle_->Get(String::New("emit"));
				Local<Function> callback = Local<Function>::Cast(callback_v);
				p_evt item;
				pthread_mutex_lock(&(This->mutex));
				item = This->first;
				v8::Handle<v8::Value> args[4];
				args[0] = v8::String::New("change");
				while (item) {
					This->first = item->next;
					args[1] = v8::String::New(item->path ? item->path : "");
					args[2] = v8::Integer::New(item->flags);
					args[3] = v8::Integer::New(item->evtid);
					callback->Call(This->handle_, 4, args);
					free(item);
					item = This->first;
				}
				This->first = NULL;
				This->last = NULL;
				pthread_mutex_unlock(&(This->mutex));
			}
			
			int running;
			char pathname[MAXPATH + 1];
			CFRunLoopRef runLoop;
			p_evt last;
			p_evt first;
			ev_async watcher;
			pthread_t thread;
			pthread_mutex_t mutex;
			FSEventStreamEventId sinceWhen;
	};
	extern "C" void init(v8::Handle<v8::Object> target) {
		node_fsevents::NodeFSEvents::Initialize(target);
	}
}

