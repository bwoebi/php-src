--TEST--
Fiber::suspend during forced unwind re-throws the deferred exception (panic mode)
--FILE--
<?php
$fiber = null;
function outer() {
    global $fiber;
    $fn = fn() {
        try {
            Fiber::suspend("paused");
        } catch (Error $e) {
            echo "inner caught: ", $e->getMessage(), "\n";
            try {
                /* During forced unwind, this Fiber::suspend() must re-throw
                 * the same Error rather than actually suspending. */
                Fiber::suspend("would-suspend");
                echo "unreachable\n";
            } catch (Error $e2) {
                echo "panic re-throw: ", $e2->getMessage(), "\n";
            }
        }
    };
    $fiber = new Fiber($fn);
    var_dump($fiber->start()); // string(6) "paused"
}
try { outer(); } catch (Error $e) { echo "outer escape: ", $e->getMessage(), "\n"; }
$fiber = null;
echo "done\n";
?>
--EXPECT--
string(6) "paused"
inner caught: Scope function closure must not outlive the declaring scope
panic re-throw: Scope function closure must not outlive the declaring scope
outer escape: Scope function closure must not outlive the declaring scope
done
