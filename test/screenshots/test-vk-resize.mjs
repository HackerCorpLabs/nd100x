#!/usr/bin/env node
// Test: Console terminal resizes correctly when VK is toggled on/off
import puppeteer from 'puppeteer';

const URL = process.argv[2] || 'http://localhost:8199/index.html';

(async () => {
  const browser = await puppeteer.launch({
    headless: 'new',
    args: ['--no-sandbox', '--disable-setuid-sandbox']
  });
  const page = await browser.newPage();
  await page.setViewport({ width: 1280, height: 900 });
  await page.goto(URL, { waitUntil: 'networkidle0', timeout: 15000 });

  await page.waitForSelector('.terminal-window', { visible: true, timeout: 5000 });
  await new Promise(r => setTimeout(r, 1500)); // let terminal render

  let allPassed = true;
  function check(name, ok, detail) {
    const tag = ok ? 'PASS' : 'FAIL';
    console.log(`  [${tag}] ${name}${detail ? ' -- ' + detail : ''}`);
    if (!ok) allPassed = false;
  }

  // --- Test 1: Non-maximized console VK toggle ---
  console.log('\n=== Test 1: Non-maximized console VK toggle ===');

  const before = await page.evaluate(() => {
    const win = document.getElementById('terminal-window');
    const canvas = win.querySelector('canvas.retroterm-canvas');
    const wrapper = win.querySelector('.terminal-canvas-wrapper');
    return {
      winH: win.offsetHeight,
      canvasH: canvas ? canvas.getBoundingClientRect().height : 0,
      wrapperH: wrapper ? wrapper.clientHeight : 0,
      wrapperW: wrapper ? wrapper.clientWidth : 0,
    };
  });
  console.log('  Before VK:', JSON.stringify(before));

  // Toggle VK on
  await page.evaluate(() => { window.toggleVirtualKeyboard(); });
  await new Promise(r => setTimeout(r, 500)); // wait for double-RAF + reflow

  const afterOn = await page.evaluate(() => {
    const win = document.getElementById('terminal-window');
    const canvas = win.querySelector('canvas.retroterm-canvas');
    const wrapper = win.querySelector('.terminal-canvas-wrapper');
    const vk = win.querySelector('.terminal-vk-container');
    return {
      winH: win.offsetHeight,
      canvasH: canvas ? canvas.getBoundingClientRect().height : 0,
      wrapperH: wrapper ? wrapper.clientHeight : 0,
      wrapperW: wrapper ? wrapper.clientWidth : 0,
      vkH: vk ? vk.getBoundingClientRect().height : 0,
      vkVisible: vk ? vk.style.display !== 'none' : false,
    };
  });
  console.log('  After VK on:', JSON.stringify(afterOn));

  check('VK is visible', afterOn.vkVisible);
  check('Window grew', afterOn.winH > before.winH,
    `was ${before.winH}, now ${afterOn.winH} (delta ${afterOn.winH - before.winH})`);
  check('Canvas height preserved (within 5px)',
    Math.abs(afterOn.canvasH - before.canvasH) < 5,
    `was ${before.canvasH.toFixed(1)}, now ${afterOn.canvasH.toFixed(1)}`);
  check('VK has non-zero height', afterOn.vkH > 20,
    `vkH = ${afterOn.vkH.toFixed(1)}`);

  // Toggle VK off
  await page.evaluate(() => { window.toggleVirtualKeyboard(); });
  await new Promise(r => setTimeout(r, 500));

  const afterOff = await page.evaluate(() => {
    const win = document.getElementById('terminal-window');
    const canvas = win.querySelector('canvas.retroterm-canvas');
    const vk = win.querySelector('.terminal-vk-container');
    return {
      winH: win.offsetHeight,
      canvasH: canvas ? canvas.getBoundingClientRect().height : 0,
      vkVisible: vk ? vk.style.display !== 'none' : false,
    };
  });
  console.log('  After VK off:', JSON.stringify(afterOff));

  check('VK is hidden', !afterOff.vkVisible);
  check('Window shrunk back (within 5px)',
    Math.abs(afterOff.winH - before.winH) < 5,
    `was ${before.winH}, now ${afterOff.winH}`);
  check('Canvas height restored (within 5px)',
    Math.abs(afterOff.canvasH - before.canvasH) < 5,
    `was ${before.canvasH.toFixed(1)}, now ${afterOff.canvasH.toFixed(1)}`);

  // --- Test 2: Maximized console VK toggle ---
  console.log('\n=== Test 2: Maximized console VK toggle ===');

  // Maximize the window
  await page.evaluate(() => {
    const win = document.getElementById('terminal-window');
    win.classList.add('maximized');
  });
  await new Promise(r => setTimeout(r, 500));

  const maxBefore = await page.evaluate(() => {
    const win = document.getElementById('terminal-window');
    const canvas = win.querySelector('canvas.retroterm-canvas');
    return {
      winH: win.offsetHeight,
      canvasH: canvas ? canvas.getBoundingClientRect().height : 0,
    };
  });
  console.log('  Maximized before VK:', JSON.stringify(maxBefore));

  // Toggle VK on
  await page.evaluate(() => { window.toggleVirtualKeyboard(); });
  await new Promise(r => setTimeout(r, 500));

  const maxAfterOn = await page.evaluate(() => {
    const win = document.getElementById('terminal-window');
    const canvas = win.querySelector('canvas.retroterm-canvas');
    const vk = win.querySelector('.terminal-vk-container');
    return {
      winH: win.offsetHeight,
      canvasH: canvas ? canvas.getBoundingClientRect().height : 0,
      vkH: vk ? vk.getBoundingClientRect().height : 0,
      vkVisible: vk ? vk.style.display !== 'none' : false,
    };
  });
  console.log('  Maximized after VK on:', JSON.stringify(maxAfterOn));

  check('VK is visible', maxAfterOn.vkVisible);
  check('Window height unchanged (maximized)', maxAfterOn.winH === maxBefore.winH,
    `was ${maxBefore.winH}, now ${maxAfterOn.winH}`);
  check('Canvas shrank to make room for VK',
    maxAfterOn.canvasH < maxBefore.canvasH - 10,
    `was ${maxBefore.canvasH.toFixed(1)}, now ${maxAfterOn.canvasH.toFixed(1)}`);
  check('VK has non-zero height', maxAfterOn.vkH > 20,
    `vkH = ${maxAfterOn.vkH.toFixed(1)}`);

  // Toggle VK off
  await page.evaluate(() => { window.toggleVirtualKeyboard(); });
  await new Promise(r => setTimeout(r, 500));

  const maxAfterOff = await page.evaluate(() => {
    const win = document.getElementById('terminal-window');
    const canvas = win.querySelector('canvas.retroterm-canvas');
    return {
      winH: win.offsetHeight,
      canvasH: canvas ? canvas.getBoundingClientRect().height : 0,
    };
  });
  console.log('  Maximized after VK off:', JSON.stringify(maxAfterOff));

  check('Canvas height restored (within 5px)',
    Math.abs(maxAfterOff.canvasH - maxBefore.canvasH) < 5,
    `was ${maxBefore.canvasH.toFixed(1)}, now ${maxAfterOff.canvasH.toFixed(1)}`);

  // Screenshot
  const screenshotPath = '/home/ronny/repos/nd100x/test/screenshots/output/vk-resize-test.png';
  await page.screenshot({ path: screenshotPath, fullPage: false });
  console.log('\nScreenshot saved to:', screenshotPath);

  console.log('\n' + (allPassed ? '=== ALL TESTS PASSED ===' : '=== SOME TESTS FAILED ==='));

  await browser.close();
  process.exit(allPassed ? 0 : 1);
})();
