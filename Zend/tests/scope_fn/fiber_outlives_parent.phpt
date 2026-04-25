--TEST--
Cross-stack scope fn used as fiber body: parent exit drives forced unwind
--FILE--
<?php
$fiber = null;
function outer() {
    global $fiber;
    $x = 42;
    $fn = fn() {
        Fiber::suspend("paused");
    };
    $fiber = new Fiber($fn);
    var_dump($fiber->start()); // string(6) "paused"
}

try {
    outer();
    echo "no error?\n";
} catch (Error $e) {
    echo "caught at outer return: ", $e->getMessage(), "\n";
}

/* Fiber was force-unwound through scope_ed at outer's exit; its saved EX is
 * now scope_ed's prev (on the fiber's own vm_stack), with the deferred
 * exception waiting for re-injection on resume. */
try {
    $fiber->resume();
    echo "no resume error?\n";
} catch (Throwable $e) {
    echo "caught at resume: ", $e->getMessage(), "\n";
}

$fiber = null;
echo "done\n";
?>
--EXPECT--
string(6) "paused"
caught at outer return: Scope function closure must not outlive the declaring scope
caught at resume: Cannot resume a fiber that is not suspended
done
