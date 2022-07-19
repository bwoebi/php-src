--TEST--
Observer: Unfinished generator with finally
--EXTENSIONS--
zend_test
--INI--
zend_test.observer.enabled=1
zend_test.observer.observe_all=1
zend_test.observer.show_return_value=1
--FILE--
<?php
function generator() {
    try {
        yield 0;
    } finally {
        echo "finally\n";
    }
}

$generator = generator();
echo $generator->current(), "\n";

?>
--EXPECTF--
<!-- init '%s%eobserver_generator_%d.php' -->
<file '%s%eobserver_generator_%d.php'>
  <!-- init generator() -->
  <generator>
  </generator:0>
0
</file '%s%eobserver_generator_%d.php'>
<generator>
finally
</generator:NULL>
