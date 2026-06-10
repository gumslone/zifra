<?php
header("Content-type: text/css");
// Expires in the past: clients always revalidate and get the current bundle
header("Expires: " . gmdate("D, d M Y H:i:s", time() - 60 * 60 * 24 * 30) . " GMT");

if (isset($_SERVER["HTTP_ACCEPT_ENCODING"]) && strstr($_SERVER["HTTP_ACCEPT_ENCODING"], "gzip")) {
	header("Content-Encoding: gzip");
	header("Vary: Accept-Encoding");
	ob_start("ob_gzhandler");
}

$files = array(
	'./files/bootstrap.min.css',
	'./files/gumboard.css',
);

foreach ($files as $file) {
	include $file;
	echo "\n";
}
