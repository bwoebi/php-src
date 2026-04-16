--TEST--
Repeated scope function calls don't leak VM stack frames
--FILE--
<?php
function test() {
    $sum = 0;
    $fn = fn($v) { $sum += $v; };
    // Call many times - if frames leak, this would overflow the VM stack
    for ($i = 0; $i < 10000; $i++) {
        $fn($i);
    }
    var_dump($sum); // sum of 0..9999 = 49995000
}
test();
?>
--EXPECT--
int(49995000)
