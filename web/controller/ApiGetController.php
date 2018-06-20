<?php

class ApiGetController extends Controller
{
    public static function css()
    {        
        header('Content-type: text/css');
        require "css/w3.css"; 
    }

    public static function js(){
        header('Content-type: text/script');
        require "script/vue.min.js";
        require "script/es6-promise.min.js";
        require "script/es6-promise.auto.min.js";
        require "script/axios.min.js";
    }

    public static function getMyTime(){
        echo date("Y-m-d H:i:s");  
    }

    public static function getHeartBit(){
        echo json_encode(Database::select("select heartbit from operation"));
    }

    public static function loadingPic(){
        require 'img/loading.gif';
    }

    public static function testUdp($ip, $port, $msg){ 
        $sock = self::getSock();
        self::sendTo($sock, $msg, $ip, $port);
        $recv = self::recvFrom($sock);
        self::clearSock($sock);
        echo "send:".$msg." to $ip:$port";
        echo ";recv:$recv"; 
    }
    private static function getSock(){
        $sock = socket_create(AF_INET, SOCK_DGRAM, SOL_UDP);
        $timeout = array('sec'=>2,'usec'=>0);
        socket_set_option($sock,SOL_SOCKET,SO_RCVTIMEO,$timeout);
        return $sock;
    }
    private static function sendTo(&$sock, $msg, $ip='127.0.0.1', $port=8080){
        $message = "a".$msg."c";
        $len = strlen($message);
        socket_sendto($sock, $message, $len, 0, $ip, $port);
    }
    private static function recvFrom(&$sock){   
        $buf='';     
        $ip='';
        $port=0;
        if(socket_recvfrom( $sock, $buf, 1024, 0, $ip, $port) > 0)
            return $buf;
        return "";
    }
    private static function clearSock(&$sock){
        socket_close($sock);
    }

    public static function sayHello(){ 
       $sock = self::getSock();
       self::sendTo($sock, "hi");
       $recv = self::recvFrom($sock);
       self::clearSock($sock);
       if(strlen($recv) > 0){
        if(strncmp($recv, "hello", 5) == 0)
            return true;
       } 
       return false;       
    }
}