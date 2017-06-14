var test = require('tape')
  , byt  = require('../');

test('treats numbers as bytes', function(t) {
  t.equal( byt(1024)  , 1024, 'should return 1024');
  t.equal( byt('1024'), 1024, 'should return 1024');
  t.end();
});

test('converts unit strings', function(t) {
  t.equal( byt('1b') , 1, 'should convert b'); 
  t.equal( byt('1k') , 1024, 'should convert k'); 
  t.equal( byt('2kb'), 2048, 'should convert kb'); 
  t.equal( byt('1m') , 1048576, 'should convert m'); 
  t.equal( byt('2mb'), 2097152, 'should convert mb'); 
  t.equal( byt('1g') , 1073741824, 'should convert g'); 
  t.equal( byt('2gb'), 2147483648, 'should convert gb'); 
  t.end();
});

test('converts uppercase units', function(t) {
  t.equal( byt('1K') , 1024, 'should convert k'); 
  t.equal( byt('1MB') , 1048576, 'should convert m'); 
  t.end();
});

test('converts unit strings with float values', function(t) {
  t.equal( byt('1.5k'), 1536     , 'should convert k'); 
  t.equal( byt('2.5m'), 2621440  , 'should convert m'); 
  t.equal( byt('0.5g'), 536870912, 'should convert g'); 
  t.end();
});

test('returns undefined value for invalid inputs', function(t, u) {
  t.equal( byt('bacon'), u, 'should return undef');
  t.end();
});
