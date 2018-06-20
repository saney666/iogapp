<?php

class TransactionController extends Controller{     

    static $pageSize = 50;
    static $pageRange = 5;
    
    public static function index(){
        $tran = json_encode(Database::select("select * from main_tran"));
        self::view('transaction/index',array('page' => 1, 'tran' => $tran));
    }

    public static function getAllSw(){
        sleep(1);
        return json_encode(Database::select("select rowid, sw from main_tran"));
    }

    public static function addNew(){
        $data = array();
        $data[0]['rowid'] = 0;
        $data[0]['interface'] = 0;
        $data[0]['iid'] = -1;
        $data[0]['format'] = 0;
        $data[0]['retry'] = 3;
        $data[0]['timeout'] = 3;
        $data[0]['delay'] = 1000;
        $data[0]['direction'] = 1;
        $data[0]['activeLog'] = 0;
        $data[0]['sw'] = 0;
        $data[0]['abnormalPoint'] = 65000;
        $data = json_encode($data);
        self::showEdit('Add Transaction', json_decode($data));
    }

    public static function edit($rowid){
        $data = Database::select("select rowid, * from tranInfo where rowid=".$rowid);
        if(count($data) != 1){
            $data = '[]';
            self::addError("Transaction data has invalid size =>".count($data));
        }
        self::showEdit('Edit Transaction', $data);
    }
    private static function showEdit($title, $data){
        $lan = self::getConnection();
        $com = self::getCom();
        $format = json_encode(Database::select("select rowid, *, 1 as hide from tranFormat order by fid"));
        $format_custom = Database::select("select rowid, *, 0 as hide from format_custom_client where tid = ".$data[0]->rowid);      
        if(count($format_custom)){
            for($i = 0;$i < count($format_custom);$i++){
                $recv = Database::select("select rowid, * from format_custom_client_recv where ccid = ".$format_custom[$i]->rowid);           
                $format_custom[$i]->recv = $recv;
                $format_custom[$i]->hide = 0;
            }
        }  
        $format_modbustcp = json_encode(Database::select("select rowid, * from format_modbus_tcp where tid = ".$data[0]->rowid));            
        $format_modbusrtu = json_encode(Database::select("select rowid, * from format_modbus_rtu where tid = ".$data[0]->rowid));            
        self::view('transaction/edit/index',array('page' => 1, 'lans' => $lan, 'coms' => $com, 'title' => $title,
        'data' => json_encode($data), 'formatData' => $format, 'formatCustomData' => json_encode($format_custom),
        'formatModbusTCPData' => $format_modbustcp, 'formatModbusRTUData' => $format_modbusrtu
        ));
    }

    public static function getConnection(){
        return json_encode(Database::select("select rowid, * from connection"));
    }

    public static function getCom(){
        return json_encode(Database::select("select rowid, * from coms"));
    }

    public static function saveCom($com){
        $command = 'update coms set ';
        $command .= 'mode='.$com->mode;
        $command .= ', baud='.$com->baud;
        $command .= ', databits='.$com->databits;
        $command .= ', parity='.$com->parity;
        $command .= ', stopbits='.$com->stopbits;
        $command .= ', flow='.$com->flow;
        $command .= ' where rowid='.$com->rowid;       
        Database::exec($command);
        return true;
    }

    public static function saveLan($lan){        
        $command = 'update connection set ';
        $command .= "ip='".$lan->ip."',";
        $command .= "port='".$lan->port."'";
        $command .= " where rowid=".$lan->rowid;     
        Database::exec($command);
        return true;
    }

    public static function addLan($ip, $port){
        $command='insert into connection (ip, port) values (';
        $command.= "'".$ip."','".$port."')";  
        Database::exec($command);
        return true;
    }

    public static function deleteLan($lan){
        $command = "delete from connection where rowid=".$lan->rowid;
        Database::exec($command);
        return true;
    }


    public static function log($rowid, $page=1){
        $data = self::getLog($rowid);
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
        $data = json_encode($fileter);

        self::view('transaction/log',array('page' => 1, 'data' => $data,
        'max' => $pageMax, 'index' => $page, 'start' => $start, 'end' => $end, 
        'dataSize' => $dataSize));
    }

    public static function logData($rowid, $page=1){
        $data = self::getLog($rowid);
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

        $data = json_encode($fileter);

        return $data;
    }

    private static function getLog($rowid){
        return Database::select("select rowid, * from log_transaction where tid=".$rowid.' order by rowid desc');
    }

    public static function apply($main, $format){
        $command = '';
        if($main[0]->rowid > 0)
            $command = self::getUpdateCommand($main);
        else
            $command = self::getInsertCommand($main);
        $tid = Database::exec($command);
        if($main[0]->rowid <= 0)
            $main[0]->rowid = $tid;
        switch($main[0]->format){            
            case 1:
                self::deleteOldFormat($main[0]->rowid, 'format_custom_client');
                self::updateFormat_Custom($main[0]->rowid, $format);
            break;
            case 2:
                self::deleteOldFormat($main[0]->rowid, 'format_modbus_rtu');
                self::udpateFormat_ModbusRTU($main[0]->rowid, $format);
            break;
            case 3:
                self::deleteOldFormat($main[0]->rowid, 'format_modbus_tcp');
                self::udpateFormat_ModbusTCP($main[0]->rowid, $format);
            break;
        }

        $command = 'update operation set updateTran=1';
        Database::exec($command);

        return true;
    }
    private static function getUpdateCommand($main){
        $command = 'update traninfo set ';
        $command .= 'interface='.$main[0]->interface;
        $command .= ',iid='.$main[0]->iid;
        $command .= ',format='.$main[0]->format;
        $command .= ',retry='.$main[0]->retry;
        $command .= ',timeout='.$main[0]->timeout;
        $command .= ',delay='.$main[0]->delay;
        $command .= ',direction='.$main[0]->direction;
        $command .= ',activeLog='.$main[0]->activeLog;
        $command .= ',abnormalPoint='.$main[0]->abnormalPoint;
        $command .= ',sw=1';
        $command .= ' where rowid='.$main[0]->rowid;
        return $command;
    }
    private static function getInsertCommand($main){
        $command ='insert into tranInfo (interface, iid, format, retry, timeout, delay, direction, activeLog, abnormalPoint, sw) values (';
        $command .= $main[0]->interface.',';
        $command .= $main[0]->iid.',';
        $command .= $main[0]->format.',';
        $command .= $main[0]->retry.',';
        $command .= $main[0]->timeout.',';
        $command .= $main[0]->delay.',';
        $command .= $main[0]->direction.',';
        $command .= $main[0]->activeLog.',';
        $command .= $main[0]->abnormalPoint.',';
        $command .= '1)';
        return $command;
    }

    private static function deleteOldFormat($tid, $tableName){
        $command = "delete from $tableName where tid=$tid";
        Database::exec($command);
    }

    private static function updateFormat_Custom($tid, $format){       
        foreach($format as $fitem){
            $command = "insert into format_custom_client (tid, command, CommandType, SplitCode, head, tail, length) values (";
            $command .= $tid.",'";
            $command .= $fitem->command."',";
            $command .= $fitem->CommandType.",'";
            $command .= $fitem->SplitCode."','";
            $command .= $fitem->head."','";
            $command .= $fitem->tail."',";
            $command .= $fitem->length.")";
            $ccid = Database::exec($command);    
            
            $command = "delete from format_custom_client_recv where ccid=".$ccid;
            Database::exec($command);
            foreach($fitem->recv as $item){
                $command = 'insert into format_custom_client_recv (ccid, position, changeway, writeaddress, parameter) values (';
                $command .= $ccid.",";
                $command .= $item->position.",";
                $command .= $item->changeway.",";
                $command .= $item->writeaddress.",'";
                $command .= $item->parameter."')";
                Database::exec($command);
            }
        }       

        return true;
    }

    private static function udpateFormat_ModbusTCP($tid, $format){
        foreach($format as $fitem){
            $command = "insert into format_modbus_tcp (tid, transactionId, protocolId, unitId, function, readAddr, number, writeAddr) values (";
            $command .= $tid.",";
            $command .= $fitem->transactionId.",";
            $command .= $fitem->protocolId.",";
            $command .= $fitem->unitId.",";
            $command .= $fitem->function.",";
            $command .= $fitem->readAddr.",";
            $command .= $fitem->number.",";
            $command .= $fitem->writeAddr.")";
            Database::exec($command);                
        }     
    }


    private static function udpateFormat_ModbusRTU($tid, $format){
        foreach($format as $fitem){
            $command = "insert into format_modbus_rtu (tid, unitId, function, readAddress, number, writeAddress) values (";
            $command .= $tid.",";
            $command .= $fitem->unitId.",";
            $command .= $fitem->function.",";
            $command .= $fitem->readAddress.",";
            $command .= $fitem->number.",";
            $command .= $fitem->writeAddress.")";
            Database::exec($command);                
        }     
    }
     
    public static function getFlag(){
        sleep(1);
        $item =  Database::select("select * from operation"); 
        if(count($item) == 1){
            if($item[0]->updateTran == 0)
                return true;
            return false;
        }
        return false;
    }
    public static function setFlag($tid, $sw, $log){
        if($sw == 0)
            return true;
        sleep(1);
        $command = "update tranInfo set sw=$sw, activeLog=$log where rowid=$tid";
        Database::exec($command);
        return true;
    }

    public static function activeUpdate(){
      Database::exec('update operation set updateTran=1');
      return true;
    }
}