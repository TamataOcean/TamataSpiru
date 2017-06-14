var matcher = /(b|k|m|g)b?$/i
var units = { 'b': 1 << 0, 'k': 1 << 10, 'm': 1 << 20, 'g': 1 << 30 };

module.exports = function byt(val) {
  var bytes;
  if (typeof val === 'number') return val;
  if (typeof val === 'string') {
    var num  = parseFloat(val);
    if (isNaN(num) === false) {
      var unit = (val.match(matcher) || [])[1];
      bytes = (unit) ? units[unit.toLowerCase()] * num : num;
    }
  }
  return bytes;
}
