<?php

class MainController extends Controller
{
    public static function index()
    {        
        $lan = json_encode(Database::select("select * from lan"));        
        $tran = json_encode(Database::select("select * from main_tran"));
        self::view('welcome', array('page' => 0, 'lan' => $lan, 'tran' => $tran));        
    }

    public static function login(){
        self::view('login', array('page' => 0));
    }
}