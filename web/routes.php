<?php
session_start();


Route::get("index.php",'Main');
Route::get("login",'Main', 'login');

Route::get("transaction", "Transaction");
Route::get("transaction.sw", "Transaction", 'getAllSw');
Route::get("transaction/addNew", "Transaction", 'addNew');
Route::get("transaction/edit", "Transaction", 'edit');
Route::get("transaction/getLan", "Transaction", 'getConnection');
Route::get("transaction/getCom", "Transaction", 'getCom');
Route::post("transaction/saveCom", "Transaction", 'saveCom');
Route::post("transaction/saveLan", "Transaction", 'saveLan');
Route::post("transaction/addLan", "Transaction", 'addLan');
Route::post("transaction/deleteLan", "Transaction", 'deleteLan');
Route::post("transaction/apply", "Transaction", 'apply');
Route::get("transaction/flag", "Transaction", 'getFlag');
Route::post("transaction/setflag", "Transaction", 'setFlag');
Route::post("transaction/activeUpdate", "Transaction", 'activeUpdate');

Route::get("transaction/log", "Transaction", 'log');
Route::get("transaction/log.data", "Transaction", 'logData');


Route::get("log", "Log");
Route::get("log/page", "Log", 'goPage');

Route::get("setting", "Setting");
Route::post("setting.editLan", "Setting", "editLan");
Route::get("setting.getFlag", "Setting", "getFlag");

Route::get("debug", "Debug");


//-------------API------------------------
//-----------get----------
Route::get("css", "ApiGet", "css");
Route::get("js", "ApiGet", "js");
Route::get("get/mytime","ApiGet", "getMyTime");
Route::get("get/heartbit","ApiGet", "getHeartBit");
Route::get("img.loading", "ApiGet", "loadingPic");
Route::get("testUdp", "ApiGet", "testUdp");
Route::get("hello", "ApiGet", "sayHello");
//-----------post
Route::post("post.login", "ApiPost", "login");
Route::post("post.logout", "ApiPost", "logout");
Route::post('db.select', "ApiPost", "db_Select");
Route::post('db.execute', "ApiPost", "db_execute");

Route::run();



// ChromePhp::log($_GET);

// Make a request for a user with a given ID
// axios.get('/user?ID=12345')
//   .then(function (response) {
//     console.log(response);
//   })
//   .catch(function (error) {
//     console.log(error);
//   });

// // Optionally the request above could also be done as
// axios.get('/user', {
//     params: {
//       ID: 12345
//     }
//   })
//   .then(function (response) {
//     console.log(response);
//   })
//   .catch(function (error) {
//     console.log(error);
//   });

//   axios.post('/user', {
//     firstName: 'Fred',
//     lastName: 'Flintstone'
//   })
//   .then(function (response) {
//     console.log(response);
//   })
//   .catch(function (error) {
//     console.log(error);
//   });
//axios post parameter
// $body = file_get_contents('php://input');
//     $json=json_decode($_POST);