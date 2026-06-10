<?php
header("Content-type: text/javascript");
// Expires in the past: clients always revalidate and get the current bundle
header("Expires: " . gmdate("D, d M Y H:i:s", time() - 60 * 60 * 24 * 30) . " GMT");

if (isset($_SERVER["HTTP_ACCEPT_ENCODING"]) && strstr($_SERVER["HTTP_ACCEPT_ENCODING"], "gzip")) {
	header("Content-Encoding: gzip");
	header("Vary: Accept-Encoding");
	ob_start("ob_gzhandler");
}

$files = array(
	'./files/jquery-3.6.0.min.js',
	'./files/bootstrap.min.js',
	'./files/jquery.weekline.min.js',
	'./files/feather.min.js',
	'./files/zifra.js',
);

foreach ($files as $file) {
	include $file;
	// Some minified files end without a newline, or even inside a line
	// comment (e.g. a trailing "//# sourceMappingURL=..."). The newline and
	// lone semicolon keep each file from breaking the one after it.
	echo "\n;\n";
}
