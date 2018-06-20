<?php

class DebugController extends Controller
{
    public static function index()
    {        
        if(Authorization::getLevel() == 2)
            self::view('debug/index', array('page' => 4 ));        
        else
            header("Location: /.");
    }
}