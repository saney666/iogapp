<?php

class Database
{
    static $RETRY = 100;
    private static function open()
    {
        $path = "/home/root";
        exec('uname -a', $info, $return);
        if (strpos($info[0], 'uc8100') !== false)
            $path = "/home/moxa";
        else if (strpos($info[0], '#404') !== false)
            $path = "/home/root";
        else if (strpos($info[0], '#119') !== false)
            $path = "/home";
        else if (strpos($info[0], '#57') !== false)
            $path = "/home/moxa";
        else if (strpos($info[0], '#1014') !== false)
            $path = "/home/pi";     
        return new SQLite3("$path/db.sqlite3");
    }

    public static function select($query)
    {       
        $db = self::open();
        $count = 0;
        while(1){
            $table = $db->query($query);
            if($table){
                $data = array();
                while ($row = $table->fetchArray(SQLITE3_ASSOC))
                    array_push($data, $row);
                $db->close();
                $result = json_encode($data);
                
                $result = str_replace('\t', '', $result);
                $result = str_replace('\n', '', $result);
                $result = str_replace('\r', '', $result);
                
                return json_decode($result);
            }
            if($count++ >= self::$RETRY){
                $db->close();
                return false;
            }
            usleep(100000);
        }
    }

    public static function exec($command)
    {        
        $count = 0;
        $db = self::open();
        while(!$db->exec($command)){
            if($count++ >= self::$RETRY){
                $db->close();
                return false;
            }                
            usleep(100000);
        }
        
        $lastRowid = $db->lastInsertRowid();           
        $db->close();
        return $lastRowid;
    }
}
