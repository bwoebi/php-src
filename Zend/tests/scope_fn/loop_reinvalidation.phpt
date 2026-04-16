--TEST--
Re-evaluating scope function in loop invalidates previous instance
--FILE--
<?php
function test() {
    $closures = [];
    for ($i = 0; $i < 3; $i++) {
        $fn = fn() { return $i; };
        $closures[] = $fn;
    }
    // Only the last closure should be valid
    var_dump($closures[2]());

    // Earlier closures should be invalidated
    try {
        $closures[0]();
    } catch (Error $e) {
        echo $e->getMessage() . "\n";
    }
}
test();
?>
--EXPECT--
int(3)
Cannot call scope function: defining scope has exited
