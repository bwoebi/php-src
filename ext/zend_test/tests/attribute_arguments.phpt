--TEST--
Verify that parameter attributes for native functions correctly support arguments.
--EXTENSIONS--
zend_test
--FILE--
<?php

$reflection = new ReflectionFunction("zend_test_parameter_with_attribute");
$attribute = $reflection->getParameters()[0]->getAttributes()[0];
var_dump($attribute->getArguments());
var_dump($attribute->newInstance());

$reflection = new ReflectionMethod("ZendTestClassWithMethodWithParameterAttribute", "no_override");
$attribute = $reflection->getParameters()[0]->getAttributes()[0];
var_dump($attribute->getArguments());
var_dump($attribute->newInstance());

$reflection = new ReflectionMethod("ZendTestClassWithMethodWithParameterAttribute", "override");
$attribute = $reflection->getParameters()[0]->getAttributes()[0];
var_dump($attribute->getArguments());
var_dump($attribute->newInstance());

$reflection = new ReflectionMethod("ZendTestChildClassWithMethodWithParameterAttribute", "no_override");
$attribute = $reflection->getParameters()[0]->getAttributes()[0];
var_dump($attribute->getArguments());
var_dump($attribute->newInstance());

$reflection = new ReflectionMethod("ZendTestChildClassWithMethodWithParameterAttribute", "override");
$attribute = $reflection->getParameters()[0]->getAttributes()[0];
var_dump($attribute->getArguments());
var_dump($attribute->newInstance());


?>
--EXPECTF--
array(1) {
  [0]=>
  string(6) "value1"
}
object(ZendTestParameterAttribute)#%d (1) {
  ["parameter"]=>
  string(6) "value1"
}
array(1) {
  [0]=>
  string(6) "value2"
}
object(ZendTestParameterAttribute)#%d (1) {
  ["parameter"]=>
  string(6) "value2"
}
array(1) {
  [0]=>
  string(6) "value3"
}
object(ZendTestParameterAttribute)#%d (1) {
  ["parameter"]=>
  string(6) "value3"
}
array(1) {
  [0]=>
  string(6) "value2"
}
object(ZendTestParameterAttribute)#%d (1) {
  ["parameter"]=>
  string(6) "value2"
}
array(1) {
  [0]=>
  string(6) "value4"
}
object(ZendTestParameterAttribute)#%d (1) {
  ["parameter"]=>
  string(6) "value4"
}
