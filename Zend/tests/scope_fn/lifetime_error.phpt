--TEST--
Scope function throws when called after defining scope exits
--FILE--
<?php
function make_scope_fn() {
    return fn() { return 1; };
}
$escaped = make_scope_fn();
try {
    $escaped();
} catch (Error $e) {
    echo $e->getMessage() . "\n";
}
?>
--EXPECT--
Cannot call scope function: defining scope has exited
