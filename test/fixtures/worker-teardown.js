const fs = require("fs");
const path = require("path");
const crypto = require("crypto");
const { Worker, isMainThread, parentPort, workerData } = require("worker_threads");

function frame(value) {
  process.stdout.write(JSON.stringify({ ...value, pid: process.pid }) + "\n");
}

if (isMainThread) {
  const [directory, mode, entry, receipt, expectedNativeSHA256] = process.argv.slice(2);
  const events = [];
  const record = (event) => { events.push(event); frame(event); };
  const worker = new Worker(__filename, { workerData: { directory, mode, entry, expectedNativeSHA256 } });
  let terminated;
  worker.on("message", (message) => {
    record(message);
    if (message.phase === "watch-ready") {
      fs.writeFileSync(path.join(directory, "native-event"), "real native watcher event");
    }
    if (message.phase === "native-event") {
      record({ phase: "fault-requested", mode });
      // The callback itself raises this fault before a parent command is needed.
      if (mode === "callback-throw") return;
      if (mode === "terminate") {
        terminated = worker.terminate();
        terminated.then((code) => record({ phase: "terminate-resolved", code }));
      } else {
        worker.postMessage({ command: mode });
      }
    }
  });
  worker.on("error", (error) => record({ phase: "worker-error", name: error.name, message: error.message, stack: error.stack }));
  worker.on("exit", (code) => {
    record({ phase: "worker-exit", code });
    fs.writeFileSync(receipt, JSON.stringify({ mode, events, workerExit: code }, null, 2) + "\n");
  });
} else {
  const { directory, mode, entry, expectedNativeSHA256 } = workerData;
  const resolved = require.resolve(entry);
  const native = path.join(path.dirname(resolved), "fsevents.node");
  const nativeSHA256 = crypto.createHash("sha256").update(fs.readFileSync(native)).digest("hex");
  if (nativeSHA256 !== expectedNativeSHA256) throw new Error("native payload identity changed before watcher start");
  const fsevents = require(resolved);
  parentPort.postMessage({ phase: "native-loaded", entry: resolved, native, nativeSHA256, version: require(path.join(path.dirname(resolved), "package.json")).version, node: process.version, napi: process.versions.napi, modules: process.versions.modules, threadId: require("worker_threads").threadId });
  let sawEvent = false;
  // Retain the public closer for the complete lifetime of the native watcher.
  const stop = fsevents.watch(directory, (eventPath, flags, id) => {
    if (!sawEvent && path.resolve(eventPath) === path.join(directory, "native-event")) {
      sawEvent = true;
      parentPort.postMessage({ phase: "native-event", path: eventPath, flags, id });
      if (mode === "callback-throw") throw new Error("fsevents callback regression");
    }
  });
  parentPort.on("message", async ({ command }) => {
    if (!sawEvent) throw new Error("teardown command arrived before native event readiness");
    if (command === "stop" || command === "double-stop") {
      await stop();
      if (command === "double-stop") await stop();
      parentPort.postMessage({ phase: "stop-complete", calls: command === "stop" ? 1 : 2 });
      parentPort.close();
    } else if (command === "exit0") {
      process.exit(0);
    } else if (command === "exit17") {
      process.exit(17);
    } else if (command === "throw") {
      throw new Error("fsevents teardown regression");
    } else {
      throw new Error("unknown teardown command: " + command);
    }
  });
  parentPort.postMessage({ phase: "watch-ready" });
}
