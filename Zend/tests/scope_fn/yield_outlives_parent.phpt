--TEST--
Scope-fn generator that outlives its parent: force-destructed at parent exit
--FILE--
<?php
$gen = null;
function outer() {
    global $gen;
    $x = 1;
    $fn = fn() {
        yield $x;
        yield $x + 1;
        yield $x + 2;
    };
    $gen = $fn();
    var_dump($gen->current()); // int(1)
    /* outer returns with $gen still suspended after first yield. Parent-exit
     * cleanup must force-destruct the generator before freeing parent's
     * frame, then throw the escape Error. */
}
try {
    outer();
    echo "no error?\n";
} catch (Error $e) {
    echo "caught: ", $e->getMessage(), "\n";
}

/* Force the parent's vm_stack region (where scope_ed lived) to be
 * reused by subsequent function calls. If the generator's saved state
 * wasn't properly torn down, this would clobber the scope_ed memory
 * the generator still references — and the next access below would
 * crash. */
function churn(int $depth): string {
    $local = str_repeat("x", 200);
    if ($depth > 0) return churn($depth - 1) . $local;
    return $local;
}
$noise = "";
for ($i = 0; $i < 50; $i++) $noise .= churn(8);
unset($noise);

/* Generator is dead. Subsequent ops report "already finished". */
var_dump($gen->valid());
try {
    $gen->next();
    var_dump($gen->current());
} catch (Throwable $e) {
    echo "next: ", $e->getMessage(), "\n";
}
$gen = null;
echo "done\n";
?>
--EXPECT--
int(1)
caught: Scope function closure must not outlive the declaring scope
bool(false)
NULL
done
