<?php

class SettingController extends Controller{    
    public static function index(){
        $lan = json_encode(Database::select("select * from lan")); 
        self::view('setting/index',array('page' => 3, 'lan' => $lan));
    }

    public static function editLan($name, $type, $address, $network, $netmask, $broadcast){        
        if(self::verifyLan($name, $type, $address, $network, $netmask, $broadcast)){
            $cmd = "update lan set ";
            $cmd .= "type='$type' , ";
            $cmd .= "address='$address' , ";
            $cmd .= "network='$network' , ";
            $cmd .= "netmask='$netmask' , ";
            $cmd .= "broadcast='$broadcast' , ";
            $cmd .= "setflag=1 where name='$name'";
            Database::exec($cmd);            
            Database::exec('update operation set resetLan=1');       
            return true;
        }
        return false;        
    }
    private static function verifyLan($name, $type, $address, $network, $netmask, $broadcast){
        $lan = Database::select("select * from lan where name ='$name'");
        if(count($lan) != 1)
            return false;
        if(strcmp($type, "static") != 0 && strcmp($type, "dhcp") != 0)
            return false;
        if(self::verifyNumber($address) || self::verifyNumber($network) || self::verifyNumber($netmask) || self::verifyNumber($broadcast))
            return false;
        return true;
    }
    private static function verifyNumber($number){
        $array = explode(".", $number);
        if(count($array) != 4)
            return true;
        
        return false;
    }

    public static function getFlag($name){
        $lan = Database::select("select * from lan where name ='$name'");
        if(count($lan) != 1){
            return false;
        }
        
        if($lan[0]->setflag == 0)
            return true;
        return false;       
    }
}