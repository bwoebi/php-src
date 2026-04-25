--TEST--
Scope-fn generator created in parent, captured by a Fiber: parent-exit cleanup destructs the generator
--FILE--
<?php
$fiber = null;
$gen = null;
function outer() {
    global $fiber, $gen;
    $x = 100;
    $fn = fn() {
        yield $x;
        $x++;
        yield $x;
    };
    $gen = $fn();
    var_dump($gen->current()); // int(100)

    $fiber = new Fiber(function () {
        global $gen;
        Fiber::suspend("paused");
        /* When user resumes us, the generator has been force-destructed
         * (parent exited). It reports as already finished. */
        echo "valid: ", var_export($gen->valid(), true), "\n";
        try {
            $gen->next();
        } catch (Throwable $e) {
            echo "next: ", $e->getMessage(), "\n";
        }
    });
    var_dump($fiber->start()); // string(6) "paused"
    /* outer returns: $gen and $fiber both still alive. The closure is
     * referenced by the generator; the generator's parent-exit cleanup
     * force-destructs it. The fiber doesn't reference the closure
     * directly (only via $gen) so no fiber unwind happens. */
}

try {
    outer();
} catch (Error $e) {
    echo "caught: ", $e->getMessage(), "\n";
}

/* Pollute the vm_stack region that held outer's frame (and scope_ed): if the
 * generator or fiber kept a stale pointer into that region, the next access
 * would land in overwritten memory and crash. */
function churn(int $depth, string $tail = ""): string {
    $local = str_repeat("y", 256);
    if ($depth > 0) return churn($depth - 1, $tail) . $local;
    return $tail . $local;
}
$noise = "";
for ($i = 0; $i < 40; $i++) $noise .= churn(10);
unset($noise);

/* Generator is dead. Fiber is alive, suspended. Resume it. */
echo "post-outer: gen valid=", var_export($gen->valid(), true), "\n";
$fiber->resume();

$fiber = null;
$gen = null;
echo "done\n";
?>
--EXPECT--
int(100)
string(6) "paused"
caught: Scope function closure must not outlive the declaring scope
post-outer: gen valid=false
valid: false
done
