--TEST--
Fiber::suspend called from within a scope-fn generator's body, then parent exits
--FILE--
<?php
$fiber = null;
function outer() {
    global $fiber;
    $fn = fn() {
        yield 1;
        Fiber::suspend("inside-gen");
        yield 2;
    };
    $g = $fn();
    var_dump($g->current()); // 1

    $fiber = new Fiber(function () use ($g) {
        try {
            $g->next();
            echo "after next, current=", var_export($g->current(), true), "\n";
        } catch (Throwable $e) {
            echo "fiber caught: ", $e->getMessage(), "\n";
        }
    });
    var_dump($fiber->start()); // string(10) "inside-gen"
}

try {
    outer();
} catch (Error $e) {
    echo "outer: ", $e->getMessage(), "\n";
}

/* Pollute the vm_stack region that held outer's frame (and scope_ed): if
 * the generator's force-destruct or the fiber's saved state retained a
 * stale pointer into outer's frame, the next access lands in overwritten
 * memory and crashes. */
function churn(int $depth, string $tail = ""): string {
    $local = str_repeat("z", 384);
    if ($depth > 0) return churn($depth - 1, $tail) . $local;
    return $tail . $local;
}
$noise = "";
for ($i = 0; $i < 40; $i++) $noise .= churn(12);
unset($noise);

/* Fiber is still suspended after parent exit. Resume it: the generator
 * is force-destructed, $g->next() returns silently, current() is NULL. */
var_dump($fiber->resume());

$fiber = null;
echo "done\n";
?>
--EXPECT--
int(1)
string(10) "inside-gen"
outer: Scope function closure must not outlive the declaring scope
after next, current=NULL
NULL
done
