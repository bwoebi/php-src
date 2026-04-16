--TEST--
Scope function closure cannot be rebound
--FILE--
<?php
function test() {
    $fn = fn() { return 1; };
    try {
        $fn2 = Closure::bind($fn, new stdClass);
    } catch (Error $e) {
        echo $e->getMessage() . "\n";
    }
}
test();
?>
--EXPECT--
Cannot rebind scope of a scope function
