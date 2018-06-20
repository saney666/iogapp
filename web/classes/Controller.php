<?php
class Controller
{
    public static function addError($err){
        $_SESSION['err'] = $err;
    }

    public static function view($viewName, $param = array())
    {
        $page = 0;        
        $error = '';
        if(isset($_SESSION['err'])){            
            $error =  $_SESSION['err'];       
            unset($_SESSION['err']);     
        }
        $authorization = Authorization::getLevel();
        foreach($param as $key => $value)
            $$key = $value;
        if ($authorization || $viewName == 'welcome' || $viewName == 'login')
            require_once("./views/$viewName.html");
        else
            header("Location: /.");
    }
}