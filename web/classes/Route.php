<?php
//header('Content-Type: application/json; charset=utf-8');
class Route
{
    static $routes_get = array();
    static $routes_post = array();

    static $fun_Controller = array();
    static $fun_Name = array();

    public static function get($uri, $controller, $function = "index")
    {
        self::$routes_get[] = $uri;
        self::$fun_Controller[$uri] = $controller;
        self::$fun_Name[$uri] = $function;
    }

    public static function post($uri, $controller, $function = "index")
    {
        self::$routes_post[] = $uri;
        self::$fun_Controller[$uri] = $controller;
        self::$fun_Name[$uri] = $function;
    }

    private static function checkUri($target, $param = array())
    {
        $getParam = $_GET['uri'];
        $match = false;
        foreach ($target as $key => $value) {
            if (preg_match("#^$value$#", $getParam)) {
                $controller = self::$fun_Controller[$getParam] . "Controller";
                $fun = self::$fun_Name[$getParam];
                switch (count($param)) {
                    case 1:
                        echo $controller::$fun($param[0]);
                        break;
                    case 2:
                        echo $controller::$fun($param[0], $param[1]);
                        break;
                    case 3:
                        echo $controller::$fun($param[0], $param[1], $param[2]);
                        break;
                    case 4:
                        echo $controller::$fun($param[0], $param[1], $param[2], $param[3]);
                        break;
                    case 5:
                        echo  $controller::$fun($param[0], $param[1], $param[2], $param[3], $param[4]);
                        break;
                    case 6:
                        echo  $controller::$fun($param[0], $param[1], $param[2], $param[3], $param[4], $param[5]);
                        break;
                    case 7:
                        echo  $controller::$fun($param[0], $param[1], $param[2], $param[3], $param[4], $param[5], $param[6]);
                        break;
                    default:
                        echo  $controller::$fun();
                        break;
                }
                $match = true;
                break;
            }
        }
        if (!$match) {
            header("Location: /.");
        }

    }

    public static function run()
    {
        $target = array();
        $param = array();
        $body = "[]";
        if ($_SERVER['REQUEST_METHOD'] == 'GET') {
            if (count($_GET) > 1)
                $body = json_encode($_GET);            
            $target = self::$routes_get;
        } else if ($_SERVER['REQUEST_METHOD'] == 'POST') {
            $body = file_get_contents('php://input');
            $target = self::$routes_post;
        }
        $param = self::getParam(json_decode($body));
        
        self::checkUri($target, $param);
    }
    private static function getParam($json)
    {
        $result = array();
        if (count($json)) {           
            foreach ($json as $key => $value) {
                if(strcmp($key, 'uri') == 0)
                    continue;
                $result[] = $value;
            }
        }
        return $result;
    }

}


