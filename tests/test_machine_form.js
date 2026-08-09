// test_machine_form.js - the one piece of INI handling that lives in
// JavaScript.
//
// machine-form.js reads a configuration through the C parser (DescribeMachineINI)
// and generates INI text that the C validator checks before it is saved. In
// between sits foreignSections(): the code that carries over every section the
// form does not own, so pressing Save does not delete the settings the form
// cannot show.
//
// [runtime] alone holds the telnet port, the throttle, the charset, the print
// and tape directories, the drum and CDC images and the memory size. If this
// function is wrong, all of it disappears the first time somebody uses the
// form, and nothing says so. Hence a test of its own.
//
//   node tests/test_machine_form.js        (exit 0 = all passed)

'use strict';

const fs = require('fs');
const path = require('path');
const vm = require('vm');

// machine-form.js only reaches for the DOM inside functions we do not call
// here, so bare stubs are enough to let it load.
const sandbox = { console, JSON, document: { getElementById: () => null }, Promise };
sandbox.window = sandbox;
vm.createContext(sandbox);

const modulePath = path.join(__dirname, '..', 'template-glass', 'js', 'machine-form.js');
vm.runInContext(fs.readFileSync(modulePath, 'utf8'), sandbox, { filename: modulePath });

const mf = sandbox.machineForm;
if (!mf || !mf._foreignSections) {
  console.error('FAIL: machine-form.js did not expose _foreignSections');
  process.exit(1);
}

let passCount = 0, failCount = 0;
function section(n) { console.log('\n=== ' + n + ' ==='); }
function assert(c, d) { if (c) { passCount++; console.log('  PASS: ' + d); } else { failCount++; console.log('  FAIL: ' + d); } }
function assertEqual(a, e, d) {
  if (a === e) { passCount++; console.log('  PASS: ' + d); }
  else { failCount++; console.log('  FAIL: ' + d + ' (expected: ' + JSON.stringify(e) + ', got: ' + JSON.stringify(a) + ')'); }
}

const join = (arr) => arr.join('\n---\n');

section('Sections the form owns are dropped');
assertEqual(join(mf._foreignSections('[machine]\ncpu = 100\n')), '', '[machine] is not carried');
assertEqual(join(mf._foreignSections('[terminals]\nenabled = 5\n')), '', '[terminals] is not carried');
assertEqual(join(mf._foreignSections('[boot]\ndevice = smd.0.0\n')), '', '[boot] is not carried');
assertEqual(join(mf._foreignSections('[controller.smd.0]\nenabled = yes\n')), '',
            '[controller.*] is not carried');
assertEqual(join(mf._foreignSections('[peripheral.lineprinter]\nenabled = yes\n')), '',
            '[peripheral.*] is not carried');

section('Sections the form does not own ARE carried');
const withRuntime =
  '[machine]\ncpu = 100\n\n' +
  '[runtime]\ncharset = norwegian\nmemory = 4\ndrum = DRUM.IMG\n\n' +
  '[boot]\ndevice = smd.0.0\n';
const carried = mf._foreignSections(withRuntime);
assertEqual(carried.length, 1, 'exactly one foreign section found');
assert(carried[0].indexOf('[runtime]') >= 0, 'and it is [runtime]');
assert(carried[0].indexOf('charset = norwegian') >= 0, 'with its charset');
assert(carried[0].indexOf('memory = 4') >= 0, 'with its memory size');
assert(carried[0].indexOf('drum = DRUM.IMG') >= 0, 'with its drum image');
assert(carried[0].indexOf('cpu = 100') < 0, 'and nothing from [machine] leaked in');
assert(carried[0].indexOf('device = smd.0.0') < 0, 'and nothing from [boot] either');

section('A section the form has never heard of');
const future = '[machine]\ncpu = 100\n\n[something.new]\nkey = value\n';
const c2 = mf._foreignSections(future);
assertEqual(c2.length, 1, 'an unknown section is carried, not dropped');
assert(c2[0].indexOf('key = value') >= 0, 'with its contents');

section('Several foreign sections');
const many =
  '[runtime]\ncharset = off\n\n' +
  '[machine]\ncpu = 110\n\n' +
  '[future.thing]\na = 1\n';
const c3 = mf._foreignSections(many);
assertEqual(c3.length, 2, 'both are kept');
assert(join(c3).indexOf('charset = off') >= 0, 'the first survives');
assert(join(c3).indexOf('a = 1') >= 0, 'and so does the last');

section('Leading comments before any section');
// Lines before the first [section] belong to no section. They must NOT be
// carried: the generated file writes its own header, and keeping the old one
// would stack a new header on top of it at every save.
const leading = '# my machine\n# second line\n\n[runtime]\ncharset = off\n';
const c4 = mf._foreignSections(leading);
assertEqual(c4.length, 1, 'only the real section is carried');
assert(c4[0].indexOf('# my machine') < 0, 'the leading comment is not carried');

section('Odd but legal input');
assertEqual(mf._foreignSections('').length, 0, 'empty text yields nothing');
assertEqual(mf._foreignSections(null).length, 0, 'null yields nothing');
assertEqual(mf._foreignSections('no sections here\n').length, 0,
            'text with no section header yields nothing');
const spaced = '   [runtime]   \ncharset = off\n';
assertEqual(mf._foreignSections(spaced).length, 1, 'an indented section header is still a section');
const upper = '[RUNTIME]\ncharset = off\n';
assertEqual(mf._foreignSections(upper).length, 1, 'and case does not change ownership');
const upperOwned = '[MACHINE]\ncpu = 100\n';
assertEqual(mf._foreignSections(upperOwned).length, 0, 'including for the sections we own');

console.log('\n===============================');
console.log('Results: ' + passCount + ' passed, ' + failCount + ' failed');
if (failCount === 0) { console.log('All tests passed.'); process.exit(0); }
process.exit(1);
