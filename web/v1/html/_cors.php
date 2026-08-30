<?php
// Shared response headers for the page fragments, which the device's web
// interface loads cross-origin via jQuery .load().
header("Access-Control-Allow-Origin: *");
header("Access-Control-Allow-Methods: POST, GET, OPTIONS");
header('P3P: CP="CAO PSA OUR"'); // Makes IE support cookies
header("Access-Control-Allow-Headers: Origin, X-Requested-With, Content-Type, Accept");
