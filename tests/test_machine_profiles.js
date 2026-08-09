// test_machine_profiles.js - unit tests for the named machine configuration
// store (template-glass/js/machine-profiles.js).
//
// NODE, not a headless browser, unlike test_glass_windows.html next door. That
// harness exists because line-printer.js and paper-tape.js need a real DOM;
// machine-profiles.js touches nothing but `window` and `localStorage`, so a
// browser would only add a dependency that CI may not have. Both are stubbed
// below - localStorage in memory, so a test run can never eat the developer's
// own saved machines.
//
//   node tests/test_machine_profiles.js        (exit 0 = all passed)

'use strict';

const fs = require('fs');
const path = require('path');
const vm = require('vm');

// ---- the stubs ------------------------------------------------------------

let store = {};
const localStorage = {
  getItem: (k) => (Object.prototype.hasOwnProperty.call(store, k) ? store[k] : null),
  setItem: (k, v) => { store[k] = String(v); },
  removeItem: (k) => { delete store[k]; },
  clear: () => { store = {}; }
};

const sandbox = { localStorage, console, JSON };
sandbox.window = sandbox;          // the module hangs its API off `window`
vm.createContext(sandbox);

const modulePath = path.join(__dirname, '..', 'template-glass', 'js', 'machine-profiles.js');
vm.runInContext(fs.readFileSync(modulePath, 'utf8'), sandbox, { filename: modulePath });

const mp = sandbox.machineProfiles;
if (!mp) { console.error('FAIL: machine-profiles.js did not define window.machineProfiles'); process.exit(1); }

// ---- the framework --------------------------------------------------------

let passCount = 0, failCount = 0;

function section(name) { console.log('\n=== ' + name + ' ==='); }

function assert(cond, description) {
  if (cond) { passCount++; console.log('  PASS: ' + description); }
  else      { failCount++; console.log('  FAIL: ' + description); }
}

function assertEqual(actual, expected, description) {
  if (actual === expected) { passCount++; console.log('  PASS: ' + description); }
  else {
    failCount++;
    console.log('  FAIL: ' + description +
                ' (expected: ' + JSON.stringify(expected) +
                ', got: ' + JSON.stringify(actual) + ')');
  }
}

function reset() { localStorage.clear(); }

// ---- the tests ------------------------------------------------------------

section('First run');
reset();
assertEqual(mp.list().length, 1, 'starts with exactly one machine');
assertEqual(mp.activeName(), 'Default', 'and it is the active one');
assert(mp.ini().indexOf('[machine]') >= 0, 'its INI is a real config');

section('Migration from the old single key');
reset();
// What a user of the previous version has: one config, no profiles.
localStorage.setItem('nd100x-machine-ini', '[machine]\ncpu = 110\n');
assertEqual(mp.list().length, 1, 'the old config becomes one profile');
assert(mp.ini().indexOf('cpu = 110') >= 0, 'and it is THEIR config, not the default');

section('Create');
reset();
assertEqual(mp.create('ND-110 test', '[machine]\ncpu = ND110CX\n'), '', 'creating a machine succeeds');
assertEqual(mp.list().length, 2, 'there are now two');
assertEqual(mp.activeName(), 'ND-110 test', 'the new one is active');
assert(mp.ini().indexOf('ND110CX') >= 0, 'showing its own INI');
assert(mp.create('ND-110 test', 'x') !== '', 'a duplicate name is refused');
assert(mp.create('   ', 'x') !== '', 'an empty name is refused');
assertEqual(mp.list().length, 2, 'and neither refusal added anything');

section('Names are trimmed');
reset();
mp.create('  spaced  ', 'x');
assertEqual(mp.activeName(), 'spaced', 'surrounding blanks are dropped');

section('Switching keeps each machine separate');
reset();
mp.write('[machine]\ncpu = 100\n');            // into Default
mp.create('Second', '[machine]\ncpu = 120\n');
assert(mp.ini().indexOf('cpu = 120') >= 0, 'Second shows its own config');
assert(mp.setActive('Default'), 'switching back works');
assert(mp.ini().indexOf('cpu = 100') >= 0, 'Default still has its own');
assert(!mp.setActive('nope'), 'switching to an unknown name is refused');
assertEqual(mp.activeName(), 'Default', 'and leaves the selection alone');

section('Rename');
reset();
mp.create('old name', 'x');
assertEqual(mp.rename('old name', 'new name'), '', 'renaming succeeds');
assertEqual(mp.activeName(), 'new name', 'the active selection follows it');
assertEqual(mp.ini(), 'x', 'the config comes with it');
assert(mp.rename('new name', 'Default') !== '', 'renaming onto another name is refused');
assert(mp.rename('missing', 'whatever') !== '', 'renaming a missing machine is refused');

section('Delete');
reset();
mp.create('doomed', 'x');
assertEqual(mp.remove('doomed'), '', 'deleting works');
assertEqual(mp.list().length, 1, 'one left');
assertEqual(mp.activeName(), 'Default', 'and the selection moved to it');
assert(mp.remove('Default') !== '', 'the LAST machine cannot be deleted');
assertEqual(mp.list().length, 1, 'so there is always one to boot');

section('Survives a corrupt store');
reset();
localStorage.setItem('nd100x-machine-profiles', '{ this is not json');
assertEqual(mp.list().length, 1, 'rebuilds instead of throwing');
assert(mp.ini().indexOf('[machine]') >= 0, 'with a usable config');

section('The old key keeps pointing at the active machine');
reset();
mp.create('Live', '[machine]\ncpu = ND120CX\n');
assert(localStorage.getItem('nd100x-machine-ini').indexOf('ND120CX') >= 0,
       'so anything still reading the single key gets the right one');
mp.create('Other', '[machine]\ncpu = 100\n');
assert(localStorage.getItem('nd100x-machine-ini').indexOf('cpu = 100') >= 0,
       'and it follows the selection');

// ---- summary --------------------------------------------------------------

console.log('\n===============================');
console.log('Results: ' + passCount + ' passed, ' + failCount + ' failed');
if (failCount === 0) { console.log('All tests passed.'); process.exit(0); }
process.exit(1);
