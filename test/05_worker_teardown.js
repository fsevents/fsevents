const fs = require("fs");
const path = require("path");
const { spawn } = require("child_process");
const crypto = require("crypto");

const directory = path.resolve(process.argv[2]);
const entry = process.argv[3] || path.resolve(__dirname, "../fsevents.js");
const evidence = process.argv[4] || directory;
const expectedNativeSHA256 = crypto.createHash("sha256").update(fs.readFileSync(path.join(path.dirname(require.resolve(entry)), "fsevents.node"))).digest("hex");
const modes = ["stop", "double-stop", "exit0", "exit17", "throw", "terminate"];
fs.mkdirSync(directory, { recursive: true });
fs.mkdirSync(evidence, { recursive: true });
const results = [];

function groupState(pid) {
  try { process.kill(-pid, 0); return "present"; }
  catch (error) { if (error.code === "ESRCH") return "gone"; throw error; }
}

async function runCase(mode) {
  const caseDirectory = path.join(directory, mode);
  fs.mkdirSync(caseDirectory, { recursive: true });
  const receipt = path.join(evidence, mode + "-worker.json");
  const child = spawn(process.execPath, [path.join(__dirname, "fixtures/worker-teardown.js"), caseDirectory, mode, entry, receipt, expectedNativeSHA256], { detached: true, stdio: ["ignore", "pipe", "pipe"] });
  let stdout = "", stderr = "", timedOut = false, spawnError;
  child.stdout.on("data", (data) => { stdout += data; });
  child.stderr.on("data", (data) => { stderr += data; });
  child.on("error", (error) => { spawnError = { name: error.name, message: error.message }; });
  const timer = setTimeout(() => {
    timedOut = true;
    try { process.kill(-child.pid, "SIGKILL"); }
    catch (error) { if (error.code !== "ESRCH") throw error; }
  }, 5000);
  const terminal = await new Promise((resolve) => child.on("close", (code, signal) => resolve({ code, signal })));
  clearTimeout(timer);
  const frames = [];
  const parseErrors = [];
  for (const line of stdout.trim().split("\n").filter(Boolean)) {
    try { frames.push(JSON.parse(line)); }
    catch (error) { parseErrors.push({ line, error: error.message }); }
  }
  let finalGroupState = groupState(child.pid);
  if (finalGroupState === "present") {
    process.kill(-child.pid, "SIGKILL");
    for (let count = 0; count < 20 && finalGroupState === "present"; count++) {
      await new Promise((resolve) => setTimeout(resolve, 10));
      finalGroupState = groupState(child.pid);
    }
  }
  const row = { mode, pid: child.pid, pgid: child.pid, terminal, actualTerminal: true, timedOut, spawnError, stdout, stderr, frames, parseErrors, finalGroupState, workerReceipt: fs.existsSync(receipt) ? JSON.parse(fs.readFileSync(receipt, "utf8")) : null };
  // Persist all native outcomes before any expectation or setup classification.
  fs.writeFileSync(path.join(evidence, mode + "-child.json"), JSON.stringify(row, null, 2) + "\n");
  results.push(row);
  const loaded = frames.find((frame) => frame.phase === "native-loaded");
  const valid = loaded && loaded.nativeSHA256 === expectedNativeSHA256 && loaded.threadId > 0 && frames.some((frame) => frame.phase === "native-event") && frames.some((frame) => frame.phase === "fault-requested");
  if (!valid || spawnError || timedOut || parseErrors.length || finalGroupState !== "gone") {
    row.classification = "INVALID_SETUP_OR_OBSERVER";
    return false;
  }
  const expected = mode === "exit17" ? 17 : mode === "throw" || mode === "terminate" ? 1 : 0;
  const workerExit = frames.find((frame) => frame.phase === "worker-exit");
  const error = frames.find((frame) => frame.phase === "worker-error");
  const stop = frames.find((frame) => frame.phase === "stop-complete");
  const passed = terminal.code === 0 && terminal.signal === null && workerExit && workerExit.code === expected && (mode !== "throw" || error && error.message === "fsevents teardown regression") && (mode !== "stop" && mode !== "double-stop" || stop && stop.calls === (mode === "stop" ? 1 : 2));
  row.classification = passed ? "PASS" : "PRODUCT_FAILURE";
  return true;
}

(async () => {
  for (const mode of modes) {
    if (!await runCase(mode)) break;
  }
  fs.writeFileSync(path.join(evidence, "results.json"), JSON.stringify({ results, expectedCases: modes }, null, 2) + "\n");
  for (const row of results) console.log(row.mode + ": " + row.classification + " child=" + JSON.stringify(row.terminal));
  process.exitCode = results.length === modes.length && results.every((row) => row.classification === "PASS") ? 0 : 1;
})().catch((error) => {
  fs.writeFileSync(path.join(evidence, "observer-error.json"), JSON.stringify({ name: error.name, message: error.message, stack: error.stack }, null, 2) + "\n");
  console.error(error);
  process.exitCode = 1;
});
