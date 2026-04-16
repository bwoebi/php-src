--TEST--
Error paths in scope function don't leak VM stack frames
--FILE--
<?php
function make_escaped() {
    return fn() { return 1; };
}

function test_recursion() {
    $fn = fn() {
        global $ref;
        $ref();
    };
    global $ref;
    $ref = $fn;
    try {
        $fn();
    } catch (Error) {}
}

// Call in tight loops to detect VM stack leaks
for ($i = 0; $i < 10000; $i++) {
    $escaped = make_escaped();
    try {
        $escaped();
    } catch (Error) {}
}
echo "lifetime: ok\n";

for ($i = 0; $i < 10000; $i++) {
    test_recursion();
}
echo "recursion: ok\n";
?>
--EXPECT--
lifetime: ok
recursion: ok
