#!/usr/bin/env node
/*
 * test-eth-wasm.js - the ND-500 ethernet exports, loaded under node.
 *
 * No browser and no disk image. Emscripten output runs perfectly well in node,
 * so the module can be loaded and the seam exercised directly. What this
 * proves is the part that is easy to get wrong and impossible to see from the
 * page: that the exports EXIST, are callable through Module._name, and refuse
 * politely before there is a machine instead of faulting.
 *
 * What it deliberately does NOT prove: frames actually reaching NDIX. That
 * needs a booted guest with a disk image, and belongs in an end-to-end run.
 *
 *   node test-eth-wasm.js [path/to/nd100wasm.js]
 */
'use strict';

const path = require('path');
const fs = require('fs');

let passed = 0, failed = 0;
function check(what, ok) {
  if (ok) { passed++; console.log('  [PASS] ' + what); }
  else    { failed++; console.log('  [FAIL] ' + what); }
}
function checkEq(what, expect, got) {
  if (expect === got) { passed++; console.log('  [PASS] ' + what); }
  else { failed++; console.log('  [FAIL] ' + what + ': expected ' + expect + ', got ' + got); }
}

const modPath = process.argv[2] ||
  path.resolve(__dirname, '../../build_wasm/bin/nd100wasm.js');

if (!fs.existsSync(modPath)) {
  console.log('no wasm module at ' + modPath);
  console.log('build it first:  . ~/repos/emsdk/emsdk_env.sh && make wasm');
  process.exit(1);
}

// The module calls main() and can print a good deal; keep it out of the way.
global.Module = {
  noInitialRun: true,
  print: () => {},
  printErr: () => {},
  onRuntimeInitialized: run
};

// NOT require(). This is a non-modularized emscripten build (MODULARIZE=0), so
// it opens with `var Module = typeof Module != 'undefined' ? Module : {}`.
// Inside a CommonJS module that `var` is module-scoped and hoists to undefined,
// so it shadows the global and quietly throws away the object above - the
// module then loads, finds no onRuntimeInitialized, and the process exits 0
// having tested nothing. Which is exactly what happened the first time.
//
// Run it in the GLOBAL scope instead, where `var Module` IS globalThis.Module
// and the pre-set object survives.
// Its node branch uses require/__dirname, which are module-scoped in CommonJS
// and therefore absent from the global scope we are about to run it in.
const vm = require('vm');
global.require = require;
global.__dirname = path.dirname(modPath);
global.__filename = modPath;
process.chdir(path.dirname(modPath));   // it locates its .wasm relative to cwd
vm.runInThisContext(fs.readFileSync(modPath, 'utf8'), { filename: modPath });

function run() {
  const M = global.Module;

  console.log('\nthe exports exist and are callable');
  const names = [
    '_Nd500_Eth_Attach', '_Nd500_Eth_Detach', '_Nd500_Eth_InjectRxFrame',
    '_Nd500_Eth_PollTxFrame', '_Nd500_Eth_GetLastTxSegment',
    '_Nd500_Eth_GetLastTxLength', '_Nd500_Eth_GetLastTxBuffer',
    '_Nd500_Eth_GetTxDropped', '_Nd500_Eth_SetLink'
  ];
  for (const n of names) check(n, typeof M[n] === 'function');

  console.log('\nis there an ND-500 in this build at all?');
  const have = M._Nd500_Available ? M._Nd500_Available() : 0;
  check('Nd500_Available says yes', have === 1);
  if (!have) {
    console.log('  (built without an nd500x checkout - the stubs are what is linked,');
    console.log('   so the behaviour checks below would be testing the stubs)');
    console.log('\n' + passed + ' passed, ' + failed + ' failed');
    process.exit(failed === 0 ? 0 : 1);
  }

  console.log('\nbefore there is a machine, it refuses rather than faulting');
  checkEq('Attach returns -1 with no machine created', -1, M._Nd500_Eth_Attach(0));
  checkEq('nothing is waiting to transmit', 0, M._Nd500_Eth_PollTxFrame());
  checkEq('nothing has been dropped', 0, M._Nd500_Eth_GetTxDropped());

  // A frame handed in before the guest is booted must be refused, not written
  // into a machine that does not exist. This is the call the gateway makes on
  // every inbound frame, so it runs constantly and must be safe at any moment.
  const ptr = M._malloc(58);
  M.HEAPU8.fill(0, ptr, ptr + 58);
  checkEq('InjectRxFrame is refused before boot', -1, M._Nd500_Eth_InjectRxFrame(0, ptr, 58));
  checkEq('a zero-length frame is refused', -1, M._Nd500_Eth_InjectRxFrame(0, ptr, 0));
  checkEq('an oversized frame is refused', -1, M._Nd500_Eth_InjectRxFrame(0, ptr, 99999));
  M._free(ptr);

  console.log('\nSetLink and Detach are safe to call at any time');
  M._Nd500_Eth_SetLink(0, 1);
  M._Nd500_Eth_SetLink(0, 0);
  M._Nd500_Eth_Detach();
  check('neither faulted', true);

  console.log('\n' + passed + ' passed, ' + failed + ' failed');
  process.exit(failed === 0 ? 0 : 1);
}
