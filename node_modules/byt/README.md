byte
====

Convert arbitrary unit strings into byte counts.

[![Build Status](https://travis-ci.org/dstokes/byt.png)](https://travis-ci.org/dstokes/byt)  
[![NPM](https://nodei.co/npm/byt.png?downloads=true)](https://nodei.co/npm/byt/)  

example
=======
``` js
var b = require('byt');

b(1024);   // => 1024
b('1k');   // => 1024
b('2.5m'); // => 2621440
b('1GB');  // => 1073741824
```

install
=======

With [npm](http://npmjs.org) do:

```
npm install byt
```
