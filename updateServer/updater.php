<?php
    $destroyFirmwarePath = "./bin/firmware/destroy/destroy.bin";
    $firmwareBasePath = "./bin/firmware/deploy/dev/unknown/firmware.bin";
    $filesystemBasePath = "./bin/filesystem/deploy/littlefs.bin";

    file_put_contents("/var/www/test/data.txt", "Start Script" . "\n", FILE_APPEND);
    /*
    foreach ($_SERVER as $key => $value) {
        $output = $key . " => " . $value . "\n";
        file_put_contents("/var/www/test/data.txt", $output . "\n", FILE_APPEND);
        }
        */
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
        readfile($path);
    }

    function updateDevice($row, $pdo)
    {
        file_put_contents("/var/www/test/data.txt", "Start Update" . "\n", FILE_APPEND);
        global $destroyFirmwarePath;
        global $firmwareBasePath;

        if($row['device_to_destroy'] == 1)
        {   
            file_put_contents("/var/www/test/data.txt", "device Destroy" . "\n", FILE_APPEND);
            header($_SERVER["SERVER_PROTOCOL"].' 806 Device Destroy');
            sendFile($destroyFirmwarePath, "destroy");
            file_put_contents("/var/www/test/data.txt", "Exit Program\n\n" . "\n", FILE_APPEND);
            exit();
        }
        else if($row['device_is_locked'] == 1)
        {
            file_put_contents("/var/www/test/data.txt", "Device locked!" . "\n", FILE_APPEND);
            header('HTTP/1.0 803 Locked');
            echo 'DeviceLocked';
            file_put_contents("/var/www/test/data.txt", "Exit Program\n\n" . "\n", FILE_APPEND);
            exit();
        }
        if($row['force_update'] == 1)
        {
            try {
                $pdo->exec("Set force_update = '0'");
            }
            catch(PDOException $e)
            {
                file_put_contents("/var/www/test/data.txt", "Error while changing Force update: " . $e . "\n", FILE_APPEND);
            }
        }
        file_put_contents("/var/www/test/data.txt", "init. send File" . "\n", FILE_APPEND);
        header($_SERVER["SERVER_PROTOCOL"].' 200 OK', true, 200);
        header('Content-Type: application/octet-stream', true);
        header('Content-Disposition: attachment; filename='.basename($firmwareBasePath));
        header('Content-Length: '.filesize($firmwareBasePath), true);
        header('x-MD5: '.md5_file($firmwareBasePath), true);
        header('x-requestedType: update', true);
        header('x-requestResponse: installUpdate', true);
        header('x-versionAvailiable: ' . $row['device_firmware_version'], true);
        file_put_contents("/var/www/test/data.txt", "readFile" . "\n", FILE_APPEND);
        
        $query = "update updates set last_updated = :unixTime WHERE mac_address = :macAddr";

        $utimestamp = time();
        $macAddress = addslashes($_SERVER['HTTP_X_STA_MAC']);
        $values = array(
            ':macAddr' => $macAddress,
            ':unixTime' => $utimestamp
        );
        
        try
        {
            $res = $pdo->prepare($query);
            $res->execute($values);
            file_put_contents("/var/www/test/data.txt", "Modified lastUpdated" . "\n", FILE_APPEND);
        }
        catch (PDOException $e)
        {
           /* If there is an error an exception is thrown */
           echo 'Error';
           file_put_contents("/var/www/test/data.txt", "Querry Error (UpdateCheckDate change): " . $e->getMessage() . "\n", FILE_APPEND);
        }

        readfile($firmwareBasePath);
    }
    
    function checkForUpdate($row, $pdo, $_version)
    {
        $query = "update updates set last_update_check = :unixTime WHERE mac_address = :macAddr";

        $utimestamp = time();
        $macAddress = addslashes($_SERVER['HTTP_X_STA_MAC']);
        $values = array(
            ':macAddr' => $macAddress,
            ':unixTime' => $utimestamp
        );
        
        try
        {
            $res = $pdo->prepare($query);
            $res->execute($values);
            file_put_contents("/var/www/test/data.txt", "Modified lastUpdateCheckDate" . "\n", FILE_APPEND);
        }
        catch (PDOException $e)
        {
           /* If there is an error an exception is thrown */
           echo 'Error';
           file_put_contents("/var/www/test/data.txt", "Querry Error (UpdateCheckDate change): " . $e->getMessage() . "\n", FILE_APPEND);
        }

        file_put_contents("/var/www/test/data.txt", "Data Dump:  " .  "DeviceToDestory(SQL): " . $row['device_to_destroy'] . "\n", FILE_APPEND);
        file_put_contents("/var/www/test/data.txt", "Data Dump:  " .  "Software Version Avail: " . $row['device_firmware_version'] . "\n", FILE_APPEND);
        file_put_contents("/var/www/test/data.txt", "Data Dump:  Software Version (installed): " . $_version . "\n", FILE_APPEND);

        if($row['device_to_destroy'] == 1)
        {
            file_put_contents("/var/www/test/data.txt", "deviceToDestroy poked" . "\n", FILE_APPEND);
            header('x-requestedType: updateCheck', true);
            header('x-requestResponse: makeUpdate', true);
            header('x-versionAvailiable: 1000.1000.1000');
            file_put_contents("/var/www/test/data.txt", "Exit Program\n\n" . "\n", FILE_APPEND);
            exit();
        }
        
        $softwareVersion = addslashes($_version);

        file_put_contents("/var/www/test/data.txt", "Check for new Software" . "\n", FILE_APPEND);
        if($softwareVersion == $row['device_firmware_version'])
        {
            file_put_contents("/var/www/test/data.txt", "Software == database" . "\n", FILE_APPEND);
            header('x-requestedType: updateCheck', true);
            header('x-requestResponse: noUpdates', true);
            header('x-versionAvailiable: ' . $row['device_firmware_version'], true, 200);
            echo 'noUpdates';
            file_put_contents("/var/www/test/data.txt", "Exit Program\n\n" . "\n", FILE_APPEND);
            exit();
        }
        else if($softwareVersion < $row['device_firmware_version'] && $row['force_update'] == 1)
        {
            file_put_contents("/var/www/test/data.txt", "Software outdated" . "\n", FILE_APPEND);
            header('x-requestedType: updateCheck', true);
            header('x-requestResponse: makeUpdate', true);
            header('x-versionAvailiable: ' . $row['device_firmware_version'], true);
            echo 'Update!';
            file_put_contents("/var/www/test/data.txt", "Exit Program\n\n" . "\n", FILE_APPEND);
            exit();
        }
        else if($softwareVersion < $row['device_firmware_version'] )
        {
            file_put_contents("/var/www/test/data.txt", "Software outdated" . "\n", FILE_APPEND);
            header('x-requestedType: updateCheck', true);
            header('x-requestResponse: newVersionAvail', true);
            header('x-versionAvailiable: ' . $row['device_firmware_version'], true);
            echo 'Update!';
            file_put_contents("/var/www/test/data.txt", "Exit Program\n\n" . "\n", FILE_APPEND);
            exit();
        }
        else
        {
            file_put_contents("/var/www/test/data.txt", "software to new!" . "\n", FILE_APPEND);
            header('x-requestedType: updateCheck', true);
            header('x-requestResponse: makeUpdate', true);
            header('x-versionAvailiable: ' . $row['device_firmware_version'], true);
            echo 'Update!';
            file_put_contents("/var/www/test/data.txt", "Exit Program\n\n" . "\n", FILE_APPEND);
            exit();
        }
        header($_SERVER["SERVER_PROTOCOL"].' 500 Error while checking', true, 500);
        echo 'ERROR WHILE CHECKING FOR UPDATE - PLEASE TRY AGAIN LATER';
        file_put_contents("/var/www/test/data.txt", "Exit Program\n\n" . "\n", FILE_APPEND);
        exit();
    }


    function checkForUpdateFS($row, $pdo, $_version)
    {      

        file_put_contents("/var/www/test/data.txt", "Data Dump:  " .  "DeviceToDestory(SQL): " . $row['device_to_destroy'] . "\n", FILE_APPEND);
        file_put_contents("/var/www/test/data.txt", "Data Dump:  " .  "Software Version Avail: " . $row['device_firmware_version'] . "\n", FILE_APPEND);
        file_put_contents("/var/www/test/data.txt", "Data Dump:  Software Version (installed): " . $_version . "\n", FILE_APPEND);

        if($row['device_to_destroy'] == 1)
        {
            file_put_contents("/var/www/test/data.txt", "deviceToDestroy poked" . "\n", FILE_APPEND);
            header('x-requestedType: updateCheck', true);
            header('x-requestResponse: makeUpdate', true);
            header('x-versionAvailiable: 1000.1000.1000');
            file_put_contents("/var/www/test/data.txt", "Exit Program\n\n" . "\n", FILE_APPEND);
            exit();
        }
        
        $softwareVersion = addslashes($_version);

        file_put_contents("/var/www/test/data.txt", "Check for new FS" . "\n", FILE_APPEND);
        if($softwareVersion == $row['device_fs_version'])
        {
            file_put_contents("/var/www/test/data.txt", "FS == database" . "\n", FILE_APPEND);
            header('x-requestedType: filesystemCheck', true);
            header('x-requestResponse: noUpdates', true);
            header('x-versionAvailiable: ' . $row['device_fs_version'], true, 200);
            echo 'noUpdates';
            file_put_contents("/var/www/test/data.txt", "Exit Program\n\n" . "\n", FILE_APPEND);
            exit();
        }
        else if($softwareVersion < $row['device_fs_version'] )
        {
            file_put_contents("/var/www/test/data.txt", "FS outdated" . "\n", FILE_APPEND);
            header('x-requestedType: filesystemCheck', true);
            header('x-requestResponse: newVersionAvail', true);
            header('x-versionAvailiable: ' . $row['device_fs_version'], true);
            echo 'Update!';
            file_put_contents("/var/www/test/data.txt", "Exit Program\n\n" . "\n", FILE_APPEND);
            exit();
        }
        else
        {
            file_put_contents("/var/www/test/data.txt", "unknown FS Version!" . "\n", FILE_APPEND);
            header('x-requestedType: filesystemCheck', true);
            header('x-requestResponse: makeUpdate', true);
            header('x-versionAvailiable: ' . $row['device_fs_version'], true);
            echo 'FS Update!';
            file_put_contents("/var/www/test/data.txt", "Exit Program\n\n" . "\n", FILE_APPEND);
            exit();
        }
        header($_SERVER["SERVER_PROTOCOL"].' 500 Error while checking', true, 500);
        echo 'ERROR WHILE CHECKING FOR UPDATE - PLEASE TRY AGAIN LATER';
        file_put_contents("/var/www/test/data.txt", "Exit Program\n\n" . "\n", FILE_APPEND);
        exit();
    }
    
    function updateDeviceFS($row, $pdo)
    {
        file_put_contents("/var/www/test/data.txt", "Start FS Update" . "\n", FILE_APPEND);
        global $filesystemBasePath;
        global $destroyFirmwarePath;

        if($row['device_to_destroy'] == 1)
        {   
            file_put_contents("/var/www/test/data.txt", "device Destroy" . "\n", FILE_APPEND);
            header($_SERVER["SERVER_PROTOCOL"].' 806 Device Destroy');
            sendFile($destroyFirmwarePath, "destroy");
            file_put_contents("/var/www/test/data.txt", "Exit Program\n\n" . "\n", FILE_APPEND);
            exit();
        }
        else if($row['device_is_locked'] == 1)
        {
            file_put_contents("/var/www/test/data.txt", "Device locked!" . "\n", FILE_APPEND);
            header('HTTP/1.0 803 Locked');
            echo 'DeviceLocked';
            file_put_contents("/var/www/test/data.txt", "Exit Program\n\n" . "\n", FILE_APPEND);
            exit();
        }
        file_put_contents("/var/www/test/data.txt", "init. send File" . "\n", FILE_APPEND);
        header($_SERVER["SERVER_PROTOCOL"].' 200 OK', true, 200);
        header('Content-Type: application/octet-stream', true);
        header('Content-Disposition: attachment; filename='.basename($filesystemBasePath));
        header('Content-Length: '.filesize($filesystemBasePath), true);
        header('x-MD5: '.md5_file($filesystemBasePath), true);
        header('x-requestedType: filesystemUpdate', true);
        header('x-requestResponse: installFSUpdate', true);
        header('x-versionAvailiable: ' . $row['device_fs_version'], true);
        file_put_contents("/var/www/test/data.txt", "readFile" . "\n", FILE_APPEND);

        readfile($filesystemBasePath);
    }

    //start Script
    if (!isset($_SERVER['PHP_AUTH_USER']) || !isset($_SERVER['PHP_AUTH_PW'])) {
        header('WWW-Authenticate: Basic realm="This site is not for Public. Please verify your identity"');
        header('HTTP/1.0 401 Unauthorized');
        echo 'Please Authorize! no cred. sended!';
        file_put_contents("/var/www/test/data.txt", "Error - PHP Auth x is not set" . "\n", FILE_APPEND);
        file_put_contents("/var/www/test/data.txt", "Exit Program\n\n" . "\n", FILE_APPEND);
        exit();
    }
    else if(!check_header('HTTP_USER_AGENT', 'Nodework-Updater-Class')) {
        header($_SERVER["SERVER_PROTOCOL"].' 403 Forbidden', true, 403);
        echo 'This is an update Site. Only granted devices are able to enter this site!. </br>';
        file_put_contents("/var/www/test/data.txt", "Error - wrong user agent" . "\n", FILE_APPEND);
        file_put_contents("/var/www/test/data.txt", "Exit Program\n\n" . "\n", FILE_APPEND);
        exit();
    }
    else if(!check_header('HTTP_X_REQUESTTYPE') || !check_header('HTTP_X_SOFTWARE_VERSION') || !check_header('HTTP_X_USEDCHIP') || !check_header('HTTP_X_SOFTWARE_VERSION'))
    {
        header('x-response: device Protocol not supported!', true);
        header($_SERVER["SERVER_PROTOCOL"].' 400 Bad Request - the device is not matching the requirements to using this service', true, 400);
        echo 'Bad Request';
        file_put_contents("/var/www/test/data.txt", "Error - x-* header are missing" . "\n", FILE_APPEND);
        file_put_contents("/var/www/test/data.txt", "Exit Program\n\n" . "\n", FILE_APPEND);
        exit();
    }

    try 
    {
        $passwd = "1234";
        $updateToken = "1234";
        $macAddr = "1234";
        if($_SERVER['PHP_AUTH_PW'] != NULL)
        {
           $passwd = addslashes($_SERVER['PHP_AUTH_PW']); 
        }
        if($_SERVER['PHP_AUTH_USER'] != NULL)
        {
            $updateToken = addslashes(($_SERVER['PHP_AUTH_USER'])); 
        }
        if($_SERVER['HTTP_X_STA_MAC'] != NULL)
        {
            $macAddr = addslashes($_SERVER['HTTP_X_STA_MAC']);
        }
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
        file_put_contents("/var/www/test/data.txt", "Database connected" . "\n", FILE_APPEND);
    }
    catch(PDOException $e) 
    {
        echo 'connection to Database failed!';
        echo 'Error: ' . $e;
        file_put_contents("/var/www/test/data.txt", "Can't connect to Database!" . "\n", FILE_APPEND);
        file_put_contents("/var/www/test/data.txt", "Exit Program\n\n" . "\n", FILE_APPEND);
        exit();
    }
    
    if(!$connected)
    {
        echo 'ERROR';
        die();
    }


    $query = 'SELECT * FROM updates WHERE device_update_token = :updateToken AND mac_address = :deviceMac';

    $values = array(
        ':updateToken' => $updateToken,
        ':deviceMac' => $macAddr
    );
    try
    {
        $res = $pdo->prepare($query);
        $res->execute($values);
        file_put_contents("/var/www/test/data.txt", "SQL user check executed" . "\n", FILE_APPEND);
    }
    catch (PDOException $e)
    {
       /* If there is an error an exception is thrown */
       file_put_contents("/var/www/test/data.txt", "Querry Error: " . $e->getMessage() . "\n", FILE_APPEND);
       echo 'Query error: ' . $e->getMessage();
    }
    if($res->rowCount() > 1)
    {
        header($_SERVER["SERVER_PROTOCOL"].' 500 Internal Database Error!', true, 500);
        /*
        foreach ($_SERVER as $key => $value) {
            $output = $key . " => " . $value . "\n";
            file_put_contents("/var/www/test/data.txt", $output . "\n", FILE_APPEND);
        }
        */
        file_put_contents("/var/www/test/data.txt", "Exit Program - found more than one Row! - Database need manual check!\n\n" . "\n", FILE_APPEND);
        exit();
    }
    else if($res->rowCount() == 0)
    {
        header('WWW-Authenticate: Basic realm="This site is not for Public. Please verify your identity"');
        header('HTTP/1.0 401 Unauthorized');
        echo 'Please Authorize!';
        file_put_contents("/var/www/test/data.txt", "No Rows" . "\n", FILE_APPEND);
        file_put_contents("/var/www/test/data.txt", "Exit Program\n\n" . "\n", FILE_APPEND);
        exit();
    }
    else
    {
        $row = $res->fetch(PDO::FETCH_ASSOC);
        $hashVal = $row['device_update_password'];
        $isCorrect = sodium_crypto_pwhash_str_verify($hashVal , $passwd);
        if($isCorrect)
        {
            file_put_contents("/var/www/test/data.txt", "Pass correct" . "\n", FILE_APPEND);

            if(check_header('HTTP_X_REQUESTTYPE', 'updateCheck'))
            {
                file_put_contents("/var/www/test/data.txt", "init check for Updates" . "\n", FILE_APPEND);
                $version = $_SERVER['HTTP_X_SOFTWARE_VERSION'];
                checkForUpdate($row, $pdo, $version);
                file_put_contents("/var/www/test/data.txt", "Exit Program\n\n" . "\n", FILE_APPEND);
                exit();
            }
            else if (check_header('HTTP_X_REQUESTTYPE', 'update'))
            {
                file_put_contents("/var/www/test/data.txt", "init updateDevice" . "\n", FILE_APPEND);
                updateDevice($row,  $pdo);
                file_put_contents("/var/www/test/data.txt", "Exit Program\n\n" . "\n", FILE_APPEND);
                exit();
            }
            else if (check_header('HTTP_X_REQUESTTYPE', 'filesystemCheck'))
            {
                $version = $_SERVER['HTTP_X_FS_VERSION'];
                file_put_contents("/var/www/test/data.txt", "init check for new FS" . "\n", FILE_APPEND);
                checkForUpdateFS($row, $pdo, $version);
                file_put_contents("/var/www/test/data.txt", "Exit Program\n\n" . "\n", FILE_APPEND);
                exit();
            }
            else if (check_header('HTTP_X_REQUESTTYPE', 'filesystemUpdate'))
            {
                file_put_contents("/var/www/test/data.txt", "init updateDeviceFS" . "\n", FILE_APPEND);
                updateDeviceFS($row,  $pdo);
                file_put_contents("/var/www/test/data.txt", "Exit Program\n\n" . "\n", FILE_APPEND);
                exit();
            }
            else
            {
                file_put_contents("/var/www/test/data.txt", "unexcepted Error" . "\n", FILE_APPEND);
                header($_SERVER["SERVER_PROTOCOL"].' 400 Bad Request!', true, 400);
                file_put_contents("/var/www/test/data.txt", "Exit Program\n\n" . "\n", FILE_APPEND);
                exit();
            }
        }
        else
        {
            header('WWW-Authenticate: Basic realm="This site is not for Public. Please verify your identity"');
            header('HTTP/1.0 401 Unauthorized');
            echo 'Please Authorize! - wrong pass';
            file_put_contents("/var/www/test/data.txt", "Wrong Pass" . "\n", FILE_APPEND);
            file_put_contents("/var/www/test/data.txt", "Exit Program\n\n" . "\n", FILE_APPEND);
            exit();
        }
    }
?>