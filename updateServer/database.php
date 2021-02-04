<?php
    class db_connect {
        private $_pdoObject = NULL;
        private $isConnected = false;
        private $usernameColName = "username";
        private $passwordColName = "password";

        function __construct() {

        }

        function __construct2($usernameColName, $passwordColName) {
            $this->usernameColName = $usernameColName;
            $this->passwordColName = $passwordColName;
        }
        
        function connectToDB($host, $dbUser, $dbPass, $dbScheme)
        {
            if($this->isConnected)
            {
                return true;
            }

            try
            {
                $dest = 'mysql:host=' . $host . ';dbname=' . $dbScheme;
            
                $_pdoObject = new PDO($dest, $dbUser, $dbPass);

                /* Enable exceptions on errors */
                $_pdoObject->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
            }
            catch(PDOException $e) 
            {
                return false;
            }
            

            if($this->_pdoObject == NULL)
            {
                return false;
            }
            return true;
        } 

        function disconnectDB()
        {
            $this->_pdoObject = NULL;
        }

        function checkParam($table = NULL, $user, $passwd)
        {
            $queryWithData = NULL;

            if($table == NULL || $user == NULL)
            {
                return false;
            }

            try
            {
                $statement = 'SELECT * FROM :tablename WHERE ' . $this->usernameColName . ' = :username';
                $value = array (
                    ':username' => $user
                );

                $queryWithData = $this->_pdoObject->prepare($statement);
                $queryWithData->execute($value);
            }
            catch(PDOException $e)
            {
                return false;
            }
            
            if($queryWithData->rowCount() == 0 || $queryWithData->rowCount() > 1)
            {
                return false;
            }
            else
            {
                //get results from query (one ROW)
                $result = $queryWithData->fetch(PDO::FETCH_ASSOC);
                //prepare the name to the format 'name'
                $preparedDBIdentifier = "'" . $this->passwordColName . "'";
                //get the hashedPassword from DB - SODIUM CRYPT
                $hashedDBPassword = $result[$preparedDBIdentifier];
                //check if $passwd match the hash
                $isCorrect = sodium_crypto_pwhash_str_verify($hashedDBPassword , $passwd);

                //sodium need rehash feature - need implemented later
                
            }


        }
    }
?>