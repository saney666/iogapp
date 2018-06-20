<?php

class Authorization{

    static $level_1 = "0000";
    static $level_2 = "7777";   

    public static function verify($password){
        if (strcmp($password, self::$level_1) == 0)
            $_SESSION['login'] = 1;         
        else if (strcmp($password, self::$level_2) == 0)
            $_SESSION['login'] = 2;  
        if (isset($_SESSION['login']))
            return true;
        return false;
    }

    public static function dispose(){
        session_destroy();
    }

    public static function getLevel(){        
        if (isset($_SESSION['login'])){            
            if($_SESSION['login'] == 2)
                return 2;
            return 1;
        }
        return 0;
    }
}