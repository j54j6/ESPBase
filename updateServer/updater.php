<?php
    $destroyFirmwarePath = "./bin/firmware/destroy/destroy.bin";
    $firmwareBasePath = "./bin";

    header('Content-type: text/plain; charset=utf8', true);

    function check_header($name, $value = false) {
        if(!isset($_SERVER[$name])) {
            return false;
        }
        if($value && $_SERVER[$name] != $value) {
            return false;
        }
        return true;
    }

    function sendFile($path, $function) {
        header($_SERVER["SERVER_PROTOCOL"].' 200 OK', true, 200);
        header('Content-Type: application/octet-stream', true);
        header('Content-Disposition: attachment; filename='.basename($path));
        header('Content-Length: '.filesize($path), true);
        header('x-MD5: '.md5_file($path), true);
        header('x-function: '.$function, true);
        readfile($path);
    }

    function updateDevice($row)
    {
        global $destroyFirmwarePath;

        if($row['device_to_destroy'] == 1)
        {   
            header($_SERVER["SERVER_PROTOCOL"].' 806 Device Destroy');
            sendFile($destroyFirmwarePath, "destroy");
            exit();
        }
        else if($row['device_is_locked'] == 1)
        {
            header('HTTP/1.0 803 Locked');
            echo 'DeviceLocked';
            exit();
        }



    }
    
    function checkForUpdate($row)
    {
        
    }


    if (!isset($_SERVER['PHP_AUTH_USER']) || !isset($_SERVER['PHP_AUTH_PW'])) {
        header('WWW-Authenticate: Basic realm="This site is not for Public. Please verify your identity"');
        header('HTTP/1.0 401 Unauthorized');
        echo 'Please Authorize!';
        exit();
    }
    else if(!check_header('HTTP_USER_AGENT', 'Nodework-Updater-Class')) {
        header($_SERVER["SERVER_PROTOCOL"].' 403 Forbidden', true, 403);
        echo 'This is an update Site. Only granted devices are able to enter this site!. </br>';
        exit();
    }
    else if(!isset('x-requestType'))
    {
        header($_SERVER["SERVER_PROTOCOL"].' 400 Bad Request - the device is not matching the requirements to using this service', true, 400);
        header('x-response: device Protocol not supported!', true);
    }

    try 
    {
        $passwd = addslashes($_SERVER['PHP_AUTH_PW']);
        $updateToken = addslashes(($_SERVER['PHP_AUTH_USER']));
        $macAddr = addslashes($_SERVER['HTTP_X_ESP8266_STA_MAC']);
    }
    catch(Exception $error)
    {
        echo($error);
    }
    

    
    /* Login status: false = not authenticated, true = authenticated. */
    $login = FALSE;


    $host = 'localhost';
    $dbUser = "nodework";
    $dbPasswd = '1234';
    $schema = 'nodework;charset=utf8mb4';

    $pdoObject = NULL;
    $dsn = 'mysql:host=' . $host . ';dbname=' . $schema;
	
    $connected = false;
    try {
        $pdo = new PDO($dsn, $dbUser, $dbPasswd);

        /* Enable exceptions on errors */
        $pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
    
        if($pdo != NULL)
        {
            $connected = true;
        }
    }
    catch(PDOException $e) 
    {
        echo 'connection to Database failed!';
        echo 'Error: ' . $e;
    }
    
    $query = 'SELECT * FROM updates WHERE device_update_token = :updateToken AND mac_address = :deviceMac';

    $values = array(
        ':updateToken' => $updateToken,
        ':deviceMac' => $macAddr
    );
    echo 'password: ' . $passwd;
    echo 'token: ' . $updateToken;
    echo 'MAC: ' . $macAddr;
    try
    {
        $res = $pdo->prepare($query);
        $res->execute($values);
    }
    catch (PDOException $e)
    {
       /* If there is an error an exception is thrown */
       echo 'Query error: ' . $e->getMessage();
    }
    if($res->rowCount() > 1)
    {
        header($_SERVER["SERVER_PROTOCOL"].' 500 Internal Database Error!', true, 500);
        echo 'Unfortnetally we have some Problems with our Server - please report this to an Administrator or wait a couple of Minutes!';
        exit();
    }
    else if($res->rowCount() == 0)
    {
        header('WWW-Authenticate: Basic realm="This site is not for Public. Please verify your identity"');
        header('HTTP/1.0 401 Unauthorized');
        echo 'Please Authorize!';
        exit();
    }
    else
    {
        $row = $res->fetch(PDO::FETCH_ASSOC);
        $hashVal = $row['device_update_password'];
        $isCorrect = sodium_crypto_pwhash_str_verify($hashVal , $passwd);
        if($isCorrect)
        {
            header($_SERVER["SERVER_PROTOCOL"].' 200 OK', true, 200);

            if(check_header('x-requestType', 'updateCheck'))
            {
                checkForUpdate($row);
            }
            else if (check_header('x-requestType', 'update'))
            {
                updateDevice($row);
            }
            else
            {
                header($_SERVER["SERVER_PROTOCOL"].' 400 Bad Request!', true, 400);
                exit();
            }
        }
        else
        {
            header('WWW-Authenticate: Basic realm="This site is not for Public. Please verify your identity"');
            header('HTTP/1.0 401 Unauthorized');
            echo 'Please Authorize! - wrong pass';
        }
    }
?>