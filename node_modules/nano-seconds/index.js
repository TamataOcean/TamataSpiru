'use strict';

const currentMs = Date.now();
const startSeconds = Math.floor(currentMs / 1000);
const startNanoseconds = (currentMs % 1000) * 1000 * 1000;
const oneSecond = 1000 * 1000 * 1000;
let nsCount = 0;

/**
 * Get nano seconds
 *
 * @returns {Integer} The value of nano seconds
 */
function nanoseconds() {
  if (exports.mode === 'step') {
    nsCount = (nsCount + 1) % 1e6;
    return nsCount;
  }
  return Math.floor(Math.random() * 1e6);
}

/**
 * The hrtime function for browser
 *
 * @returns {Array} [Sceonds, NanoSeconds]
 */
function browserHrtime() {
  const current = Math.floor(window.performance.now() * 1e6);
  const currentSeconds = Math.floor(current / 1e9);
  const currentNanoseconds = current % 1e9;
  return [
    currentSeconds,
    currentNanoseconds,
  ];
}


/**
 * Get the hrtime
 *
 * @returns {Array} [Seconds, NanoSeconds]
 */
function getHrtime() {
  /* eslint no-undef:0 */
  if (window && window.performance && window.performance.now) {
    return browserHrtime();
  }
  const current = Date.now();
  const currentSeconds = Math.floor(current / 1000);
  const currentNanoseconds = ((current % 1000) * 1e6) + nanoseconds();
  return [
    currentSeconds,
    currentNanoseconds,
  ];
}


/**
 * Custom function of hrtime
 *
 * @param {Array} time The start time of hrtime
 * @returns {Array} [Seconds, NanoSeconds]
 */
function customHrtime(time) {
  const arr = getHrtime();
  const currentSeconds = arr[0];
  const currentNanoseconds = arr[1];
  if (!time) {
    return [
      currentSeconds,
      currentNanoseconds,
    ];
  }
  let offsetSeconds = currentSeconds - time[0];
  let offsetNanoseconds = currentNanoseconds - time[1];
  if (offsetNanoseconds < 0) {
    offsetNanoseconds += 1e9;
    offsetSeconds -= 1;
  }
  return [
    offsetSeconds,
    offsetNanoseconds,
  ];
}

const hrtime = (process && process.hrtime) || customHrtime;
const start = hrtime();


/**
 * Get the current time with nanoseconds
 *
 * @returns {Array} [Seconds, NanoSeconds]
 */
function now() {
  const arr = hrtime(start);
  const value = arr[1] + startNanoseconds;
  if (value >= oneSecond) {
    return [
      startSeconds + arr[0] + 1,
      value % oneSecond,
    ];
  }
  return [
    startSeconds + arr[0],
    value,
  ];
}


/**
 * Pad ns with '0'
 *
 * @param {Integer} ns The nano seconds
 * @returns {String}
 */
function padNS(ns) {
  const str = `${ns}`;
  const pad = '000000000'.substring(0, 9 - str.length);
  return `${pad}${str}`;
}

/**
 * Get the nano seconds string
 *
 * @param {Array} arr [Seconds, NanoSeconds]
 * @returns {String}
 */
function toString(arr) {
  const ns = arr || now();
  return `${ns[0]}${padNS(ns[1])}`;
}

/**
 * Get the nano seconds iso string
 *
 * @param {Array} arr [Seconds, NanoSeconds]
 * @returns {String}
 */
function toISOString(arr) {
  const ns = arr || now();
  const str = new Date(ns[0] * 1000).toISOString();
  const nsStr = `${padNS(ns[1])}`.replace(/0+$/, '');
  return str.replace('000Z', `${nsStr}Z`);
}

/**
 * Get the nano form iso string
 *
 * @param {any} str
 */
function fromISOString(str) {
  const arr = str.split('.');
  const date = new Date(`${arr[0]}.000Z`);
  const value = arr[1].substring(0, arr[1].length - 1);
  const ns = Number.parseInt(`${value}000000000`.substring(0, 9), 10);
  return [
    date.getTime() / 1000,
    ns,
  ];
}

/**
 * Get the difference of the nano seconds
 *
 * @param {Array} ns [Seconds, NanoSeconds]
 * @param {Array} ns2 [Seconds, NanoSeconds], default is `now()`
 * @returns {Integer} The difference nano seconds
 */
function difference(ns, ns2) {
  const current = ns2 || now();
  const us = current[1] - ns[1];
  const s = current[0] - ns[0];
  return (s * oneSecond) + us;
}

exports.now = now;
exports.toString = toString;
exports.difference = difference;
exports.toISOString = toISOString;
exports.fromISOString = fromISOString;
exports.mode = 'random';
