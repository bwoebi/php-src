--TEST--
Observer: Unfinished generator without finally
--EXTENSIONS--
zend_test
--INI--
zend_test.observer.enabled=1
zend_test.observer.observe_all=1
zend_test.observer.show_return_value=1
--FILE--
<?php
function generator() {
    yield 0;
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
</generator:NULL>
