exports.sleep = (ms) => new Promise((resolve) => setTimeout(resolve, ms));
exports.capture = () => {
  const events = [];
  events.callback = (...args) => events.push(args);
  return events;
};

exports.run = async (test) => {
  try {
    await test();
    process.exit(0);
  } catch(err) {
    console.log('  ---');
    console.log(`  message: ${err.message}`);
    console.log(`  actual: ${JSON.stringify(err.actual)}`);
    console.log(`  expected: ${JSON.stringify(err.expected)}`);
    console.log(`  source: ${err.stack.split(/\r?\n/)[1].trim()}`);
    console.log('  ...');
    process.exit(1);
  }
};
