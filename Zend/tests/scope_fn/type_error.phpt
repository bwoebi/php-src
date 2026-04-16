--TEST--
Type error in scope function parameter is catchable
--FILE--
<?php
function test() {
    $fn = fn(int $a) { return $a; };
    try {
        $fn("not an int");
    } catch (TypeError $e) {
        echo "Caught: " . $e->getMessage() . "\n";
    }
    // Parent function continues normally
    echo "ok\n";
}
test();
?>
--EXPECTF--
Caught: %s
ok
