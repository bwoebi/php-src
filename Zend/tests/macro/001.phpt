--TEST--
Simple macro
--FILE--
<?php

function sql!($args) {
	$buf = "";
	$strbuf = [];
	foreach ($args as [$val, $tok]) {
		if ($tok == T_VARIABLE) {
			if ($buf !== "") {
				$strbuf[] = "'" . addcslashes($buf, "\\'") . "'";
				$buf = "";
			}
			$strbuf[] = "(is_array($val) ? implode(', ', array_map(fn(\$v) => \"'\" . addcslashes(\$v, \"\\\\'\") . \"'\", $val)) : (\"'\" . addcslashes($val, \"\\\\'\") . \"'\"))";
		} else {
			$buf .= $val;
		}
	}

	if ($buf !== "") {
		$strbuf[] = "'" . addcslashes($buf, "\\'") . "'";
	}
	$code = "return " . implode(" . ", $strbuf) . ";";
	var_dump($code);
	return $code;
}

$val = 1;
$vals = ['foo\'', 3];

var_dump(sql!(SELECT foo FROM table WHERE col in ($val, $vals)));

?>
--EXPECT--
string(340) "return 'SELECT foo FROM table WHERE col in (' . (is_array($val) ? implode(', ', array_map(fn($v) => "'" . addcslashes($v, "\\'") . "'", $val)) : ("'" . addcslashes($val, "\\'") . "'")) . ', ' . (is_array($vals) ? implode(', ', array_map(fn($v) => "'" . addcslashes($v, "\\'") . "'", $vals)) : ("'" . addcslashes($vals, "\\'") . "'")) . ')';"
string(54) "SELECT foo FROM table WHERE col in ('1', 'foo\'', '3')"
