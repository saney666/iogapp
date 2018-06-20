<?php
//http://php.net/manual/zh/book.sqlite3.php
ini_set( 'memory_limit', '32M' );

require_once('routes.php');

function __autoload($class_name)
{
    if (file_exists('./classes/' . $class_name . '.php'))
        require_once './classes/' . $class_name . '.php';
    else if (file_exists('./model/' . $class_name . '.php'))
        require_once './model/' . $class_name . '.php';
    else if (file_exists('./controller/' . $class_name . '.php'))
        require_once './controller/' . $class_name . '.php';
}


?>