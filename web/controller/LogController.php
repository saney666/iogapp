<?php

class LogController extends Controller{    

    static $pageSize = 15;
    static $pageRange = 5;

    public static function index(){        
        self::goPage();
    }

    public static function goPage($page = 1, $filter=''){
        $data = Database::select("select rowid, * from log_system where target like '%$filter%' order by time desc"); 
        $dataSize = count($data);
        $pageMax = ($dataSize - $dataSize%self::$pageSize)/self::$pageSize + ($dataSize%self::$pageSize?1:0);
   
        if($page > $pageMax)
            $page = $pageMax;

        $fileter = array();   
        $count = 0;
        $index = 1;
        for($i = 0; $i < $dataSize; $i++){
            if($index == $page){
                $fileter[] = $data[$i];
            }
            if(++$count >= self::$pageSize){
                $count = 0;
                $index++;
            }
        }

        $start = $page - self::$pageRange;
        if($start < 1)
            $start = 1;
        
        $end = $page + self::$pageRange;
        if($end > $pageMax)
            $end = $pageMax;

        $targets =json_encode(Database::select("select distinct target from log_system")); 

        self::view('log/index',array('page' => 2, 'data' => json_encode($fileter),
        'max' => $pageMax, 'index' => $page, 'start' => $start, 'end' => $end, 
        'targets' => $targets, 'dataSize' => $dataSize, 'filter' => $filter
        ));
    }
}

