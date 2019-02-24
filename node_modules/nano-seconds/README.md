# nano-seconds

Get the nano seconds of now

[![Build Status](https://travis-ci.org/vicanso/nano-seconds.svg?style=flat-square)](https://travis-ci.org/vicanso/nano-seconds)
[![Coverage Status](https://img.shields.io/coveralls/vicanso/nano-seconds/master.svg?style=flat)](https://coveralls.io/r/vicanso/nano-seconds?branch=master)
[![npm](http://img.shields.io/npm/v/nano-seconds.svg?style=flat-square)](https://www.npmjs.org/package/nano-seconds)
[![Github Releases](https://img.shields.io/npm/dm/nano-seconds.svg?style=flat-square)](https://github.com/vicanso/nano-seconds)

## Installation

```bash
$ npm install nano-seconds
```

## API

### now

Get the nano seconds of current time, it will return [seconds, nanos]

```js
const nano = require('nano-seconds');
// [ 1488895112, 951851969 ]
console.info(nano.now());
```

### toString

Format the nano seconds to string

- `ns` if not set the param, it will be `nano.now()`

```js
const nano = require('nano-seconds');
const ns = nano.now();
// [ 1488895353, 21164240 ]
console.info(ns);
// 1488895353021164240
console.info(nano.toString(ns));
// 1488895353025439741
console.info(nano.toString());
```

### toISOString

Format the nano seconds to ISOString

- `ns` if not set the param, it will be `nano.now()`

```js
const nano = require('nano-seconds');
const ns = nano.now();
// 2017-06-22T14:37:42.506635539Z
console.info(nano.toISOString(ns));
// 2017-06-22T14:37:42.5069231Z
console.info(nano.toISOString());
```

### fromISOString

Get the nano from ISOString

- `iosFormat` ISO Date String

```js
const nano = require('nano-seconds');
const arr = nano.fromISOString(str);
// [ 1505477440, 922020280 ]
console.info(arr);
```


### difference

Get the difference of two nano seconds, ns2 - ns1

- `ns1` The start nano seconds

- `ns2` The end nano seconds, default is `nano.now()`

```js
const ns = nano.now();
setTimeout(() => {
  const diff = nano.difference(ns);
  // 102661874
  console.info(diff);
}, 100);
```


## License

MIT
