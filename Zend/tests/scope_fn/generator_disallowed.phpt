--TEST--
Scope function cannot be a generator
--FILE--
<?php
function test() {
    $fn = fn() { yield 1; };
}
?>
--EXPECTF--
Fatal error: Scope functions cannot be generators in %s on line %d
