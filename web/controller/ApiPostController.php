<?php

class ApiPostController extends Controller
{
    public static function login($password){
        echo Authorization::verify($password);
    }

    public static function logout(){
        Authorization::dispose();
    }

    public static function db_Select($command){
        if (Authorization::getLevel() == 2){
            echo json_encode(Database::select($command));
        }  
    }

    public static function db_execute($command){
        if (Authorization::getLevel() == 2){
            echo json_encode(Database::exec($command));
        }
    }
}