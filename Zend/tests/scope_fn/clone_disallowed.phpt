--TEST--
Scope function closure cannot be cloned
--XFAIL--
GC assertion failure after clone error in debug builds
--FILE--
<?php
function test() {
    $fn = fn() { return 1; };
    try {
        $fn2 = clone $fn;
    } catch (Error $e) {
        echo $e->getMessage() . "\n";
    }
}
test();
?>
--EXPECT--
Cannot clone a scope function closure
