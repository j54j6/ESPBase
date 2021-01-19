#ifndef J54J6_MODULESTATE_H
#define J54J6_MODULESTATE_H

#include <Arduino.h>
#include "led.h"
#include "logger.h"
#include "filemanager.h"

#define opticalOnWarn

/*
    ClassReportTemplate

    Object contains all Data of one Report 
    ClassReportTemplate is part of ordered List with Pointer
*/
class ClassReportTemplate {
    friend class ClassReportModuleHandler;
    friend class ClassModuleMaster;
    private:
        String className = "n.S";
        ClassReportTemplate* _nextNode = NULL;
        bool error = false;
        String message = "";
        short errorCode = 0;
        bool reportIsLocked = false; //if report is locked nothing can changed - the handler can read the Report and remove the Node - lock the Report if all Data are set
        /*
        priority: 0 for ignoring like bool error = false
            min: 1 - Class can work without problems
                 2 - Class can work but with less functionality
                 3 - Class can't work but try to fix it automatically - Handler wait for further Error - after 3 times -> auto set to prio 4 -> class restart
                 4 - Class can't work, theoretically able to fix problem but failed - only restart Class by Handler
                 5 - Class can't work, can't fix it automatically - only restart of class by Handler
                 6 - Class can't work, can't fix it automatically - device can be Damaged - ErrorHandler stop complete Device  
        */
        short priority = 0;

    protected:
        //set
        void setNextClassReport(ClassReportTemplate* _nextClassReport)
        {
            if(_nextNode == NULL)
            {
               this->_nextNode = _nextClassReport; 
            }
        }

        //Get
        String getClassname()
        {
            return this->className;
        }

        bool getError()
        {
            return this->error;            
        }

        String getErrorMessage()
        {
            return this->message;
        }

        short getErrorCode()
        {
            return errorCode;
        }

        ClassReportTemplate* getNextClassReport()
        {
            if(_nextNode != NULL)
            {
                return this->_nextNode;
            }
            else
            {
                return NULL;
            }
        }

    public:
        ClassReportTemplate(){};
        ClassReportTemplate(String className, String message, short errorCode, short priority, bool isError)
        {
            this->className = className;
            this->message = message;
            this->errorCode = errorCode;
            this->error = isError;
            this->priority = priority;
        }

        void lockReport()
        {
            reportIsLocked = true;
        }

        void setError(bool newValue)
        {
            if(!reportIsLocked)
            {
                this->error = newValue;
            }
        }

        void setErrorMessage(const char* message)
        {
            if(!reportIsLocked)
            {
                this->message = message;
            }
        }

        void setErrorCode(short newErrorCode)
        {
            if(!reportIsLocked)
            {
                this->errorCode = newErrorCode;
            }
        }

        void setErrorPriority(short newPrio)
        {
            if(!reportIsLocked)
            {
                this->priority = newPrio;
            }
        }
};

/*
    ClassReportModuleHandler

    Part of every ClassModuleSlave - every class had won ClassReportModuleHandler which handles all
    Reports
*/
class ClassReportModuleHandler  {
    friend class ClassModuleMaster;
    private:
        ClassReportTemplate* _firstReport = NULL;
        ClassReportTemplate* _actualReportPointer = NULL;
    
        //helper function
        ClassReportTemplate* getLastNode()
        {
            if(_firstReport == NULL)
            {
                return NULL;
            }
            ClassReportTemplate* _actualReportPointer = _firstReport;
            while(_actualReportPointer->getNextClassReport() != NULL)
            {
                _actualReportPointer = _actualReportPointer->getNextClassReport();
            }
            return _actualReportPointer;
        }
    protected:
        ClassReportTemplate* getActualReport()
        {
            return _actualReportPointer;
        }

        void removeActualReport()
        {
            ClassReportTemplate* _lastPoint = _actualReportPointer;
            if(this->_actualReportPointer == NULL)
            {
                return;
            }
            else if(_actualReportPointer == _firstReport)
            {
                _firstReport = NULL;
                _actualReportPointer = NULL;
                delete _lastPoint; // needed to aloocate the storage - Object sacved at stack and not in heap
                return;
            }

            ClassReportTemplate* _cacheTemplate = _firstReport;
            while(_cacheTemplate->getNextClassReport() != _actualReportPointer)
            {
                _cacheTemplate = _cacheTemplate->getNextClassReport();
            }
            if(_cacheTemplate->getNextClassReport() == _actualReportPointer)
            {
                if(_actualReportPointer->getNextClassReport() != NULL)
                {
                    _cacheTemplate->setNextClassReport(_actualReportPointer->getNextClassReport());
                    _actualReportPointer = _actualReportPointer->getNextClassReport();
                }
                else
                {
                    _cacheTemplate->setNextClassReport(NULL);
                    _actualReportPointer = _cacheTemplate;
                }
            }
            delete _lastPoint; // needed to aloocate the storage - Object sacved at stack and not in heap
        }

        void toNextReport() { //set actualNodePointer to the next report
            if(_firstReport == NULL)
            {
                return;
            }
            else if(_firstReport == NULL && _actualReportPointer != NULL)
            {
                _firstReport = _actualReportPointer;
            }
            else if((_firstReport != NULL && _actualReportPointer == NULL) || (_actualReportPointer->getNextClassReport() == NULL &&_firstReport != NULL))
            {
                _actualReportPointer = _firstReport;
                return;
            }
            else
            {
                _actualReportPointer = _actualReportPointer->getNextClassReport();
            }
        }

        void toFirstReport() {
            _actualReportPointer = _firstReport;
        }

    public:
        ClassReportModuleHandler() {}

        void addReport(const char* className, String message, short errorCode, short priority, bool isError, bool lockReport = false)
        {
            ClassReportTemplate* newTempl = new ClassReportTemplate(className, message, errorCode, priority, isError);
            ClassReportTemplate* pointer;
            if(_firstReport == NULL)
            {
                this->_firstReport = newTempl;
                this->_actualReportPointer = _firstReport;
                pointer = _firstReport;
            }
            else
            {
                getLastNode()->setNextClassReport(newTempl);
                pointer = getLastNode();
            }

            if(lockReport)
            {
                pointer->lockReport();
            }
        }

        void lockActualReport()
        {
            _actualReportPointer->lockReport();
        }
};

/*
    ModuleStateSlave

    Used to define a Class State and Control the Class - special functions need to be implemented in Slave Class
*/
class ModuleStateSlave
{
    private:
        /*  
            classState:
                classState describes the current state of the class (as the Name says...)
                0 = classDisabled
                1 = class Idle
                2 = class Working
                3 = class Waiting
                4 = class WARN
                5 = class Error
                6 = Ermg. Class Shutdown
                7 = Ermg. Class restart
        */
        short classState = 0;        
        
    protected:
        void setClassState(short newState)
        {
            if(newState >= 0 && newState <= 7)
            {
                this->classState = newState;
            }            
        }

        short getClassState()
        {
            return this->classState;
        }

        void stopClass();
        void startClass();
        void restartClass();
    
    public:
        ModuleStateSlave() {}
};

/*
    Performance Indicator - check how often Classloop is called per Second
*/
class ClassRepeatChecker {
    private:
        ulong lastCall = 0;
        ulong callPerSecond = 0;
        int estCallIntervall = 0;
        bool alreadyReported = false;
        
        void internalRun()
        {
            //this->callPerSecond = 1000/(millis() - lastCall);
            if(millis() - lastCall <= 1000)
            {
                callPerSecond++;
            }
            else
            {
                callPerSecond = 1;
            }
            lastCall = millis();
            delay(1);
        }
    public:
        ClassRepeatChecker(int estCallIntervall)
        {
            this->estCallIntervall = estCallIntervall;
        }

        void setWarnIsReported(bool newVal = true)
        {
            this->alreadyReported = newVal;
        }

        bool getWarnIsReported()
        {
            return alreadyReported;
        }

        ulong getCallPerSecond()
        {
            return this->callPerSecond;
        }

        int getEstCallPerSecond()
        {
            return estCallIntervall;
        }

        void run()
        {
            internalRun();
        }

};

/*
    ClassModuleSlave

    ClassModuleSlave is part of every Class using this Libary.
    Every Slave Class need one ClassModuleSlave inherits all functionalities like:
        - ClassRepeatChecker -> getCallsPerSecond
        - ClassReportModuleHandler -> Reporting of error and ClassControl
        - ModuleStateSlave -> Control Class
*/
class ClassModuleSlave : protected ModuleStateSlave {
    friend class ClassModuleMaster;
    private:
        const char* className = "n.S";
        ClassModuleSlave* _nextSlave;

        ClassReportModuleHandler reportHandler;
        ClassRepeatChecker* _repeatChecker;

        ClassReportModuleHandler* getReportHandler()
        {
            return &reportHandler;
        }
    protected:
        //Helper
        void setNextModuleSlave(ClassModuleSlave* _nextSlave)
        {
            this->_nextSlave = _nextSlave;
        }

        ClassModuleSlave* getNextModuleSlave()
        {
            if(_nextSlave == NULL)
            {
                return NULL;
            }
            else
            {
                return _nextSlave;
            }
        }

        //repeat checker
        ulong getCallsPerSecond()
        {
            return this->_repeatChecker->getCallPerSecond();
        }

    public:
        ClassModuleSlave(const char* className, int estCallIntervall)
        {
            this->className = className;
            this->_repeatChecker = new ClassRepeatChecker(estCallIntervall);
        }

        //reporting
        void newReport(String message, short errorCode, short priority = 0, bool isError = false, bool lockReport = true)
        {
            reportHandler.addReport(className, message, errorCode, priority, isError, lockReport);
        }
        void run()
        {
            _repeatChecker->run();
        }
};

/*
    ClassModuleMaster

    only one instance is needed in the "main" Class - The Master control all registered Classes and Report Errors and States
*/
class ClassModuleMaster {
    private:
        ClassModuleSlave* _firstSlave = NULL;
        ClassModuleSlave* _actualSlavePointer = NULL;
        Filemanager* FM = NULL;
        LED* _workLed = NULL;
        LED* _errorLed = NULL;
        
        bool opticalSignalActive = false;
        ulong opticalSignalActuveUntil = 0;

        //set
        void toNextModuleSlave()
        {
            if(_firstSlave == NULL && _actualSlavePointer == NULL)
            {
                return;
            }
            else if(_firstSlave == NULL && _actualSlavePointer != NULL)
            {
                _firstSlave = _actualSlavePointer;
                return;
            }
            else if(_firstSlave != NULL && _actualSlavePointer == NULL)
            {
                _actualSlavePointer = _firstSlave;
            }
            else
            {
                if(_actualSlavePointer->getNextModuleSlave() == NULL)
                {
                    _actualSlavePointer = _firstSlave;
                }
                else
                {
                    _actualSlavePointer = _actualSlavePointer->getNextModuleSlave();
                }   
            }
        }

        /*
            mode = false -> blinkLed
            mode = true -> solidLed
            time in ms
        */
        void setOpticalSignal(short time, bool mode = false)
        {
            opticalSignalActive = true;
            _workLed->ledOff();
            _workLed->setLocked(true);
            if(mode)
            {
                _errorLed->ledOn();
            }
            else
            {
                _errorLed->blink(500);
            }
            opticalSignalActuveUntil = millis() + time;
        }

        void checkOpticalReport()
        {
            if(millis() >= opticalSignalActuveUntil && opticalSignalActive)
            {
                _errorLed->ledOff();
                _workLed->setLocked(false);
                _workLed->ledOn();
                opticalSignalActive = false;
                opticalSignalActuveUntil = 0;
            }
        }

        void ModuleControl()
        {
            if(_firstSlave == NULL)
            {
                return;
            }

            _actualSlavePointer = _firstSlave;

            while(_actualSlavePointer != NULL) {
                _actualSlavePointer->run();
                ClassReportModuleHandler* _cacheReportHandler = _actualSlavePointer->getReportHandler(); //set report Handler of the current checked Module
                _cacheReportHandler->toFirstReport(); //set to the First report
                while(_cacheReportHandler->getActualReport() != NULL)
                {
                    ClassReportTemplate* _actualReport = _cacheReportHandler->getActualReport();
                    if(_actualReport->getError()) //Show an Error Message
                    {
                        showErrorWarnMessage(_actualReport, true);
                    }
                    else //Show Warn Message
                    {
                        showErrorWarnMessage(_actualReport ,false);
                    }
                    _cacheReportHandler->removeActualReport();
                    _cacheReportHandler->toNextReport();
                }
                checkClassRepeatIntervall();
                _actualSlavePointer = _actualSlavePointer->getNextModuleSlave();
            }
        }


        void showErrorWarnMessage(ClassReportTemplate* _actualReport, bool error=false)
        {
            SysLogger tempLogger(FM, "ReportHandler");
            if(error)
            {
                tempLogger.logIt("MasterReport", "##########-ERROR-##########", 5);
                tempLogger.logIt("MasterReport", " ", 5);
                String message = "Reporting Class: " + String(_actualReport->getClassname());
                tempLogger.logIt("MasterReport", message, 5);
                message = "Reason: : " + String(_actualReport->getErrorMessage());
                tempLogger.logIt("MasterReport", message, 5);
                message = "Error-Code: " + String(_actualReport->getErrorCode());
                tempLogger.logIt("MasterReport", message, 5);
                tempLogger.logIt("MasterReport", " ", 5);
                tempLogger.logIt("MasterReport", "##########-ERROR-##########", 5);
                setOpticalSignal(10000, true);
            }
            else
            {
                tempLogger.logIt("MasterReport", "##########-WARN-##########", 4);
                tempLogger.logIt("MasterReport", " ", 4);
                String message = "Reporting Class: " + String(_actualReport->getClassname());
                tempLogger.logIt("MasterReport", message, 4);
                message = "Reason: : " + String(_actualReport->getErrorMessage());
                tempLogger.logIt("MasterReport", message, 4);
                message = "Error-Code: " + String(_actualReport->getErrorCode());
                tempLogger.logIt("MasterReport", message, 4);
                tempLogger.logIt("MasterReport", " ", 4);
                tempLogger.logIt("MasterReport", "##########-WARN-##########", 4);
                #ifdef opticalOnWarn
                        setOpticalSignal(10000);
                #endif
            }
        }


        void checkClassRepeatIntervall()
        {
            if(_actualSlavePointer->_repeatChecker->getCallPerSecond() < _actualSlavePointer->_repeatChecker->getEstCallPerSecond() - ulong(2))
            {
                if(!_actualSlavePointer->_repeatChecker->getWarnIsReported())
                {
                    _workLed->blink(1000);
                    SysLogger tempLogger(FM, "ReportHandler");
                    tempLogger.logIt("IntervallChecker", "######-Speed-######", 1);
                    tempLogger.logIt("IntervallChecker", "ClassCall/s is fewer than defined!", 1);
                    tempLogger.logIt("IntervallChecker", "Classname: " + String(_actualSlavePointer->className), 1);
                    tempLogger.logIt("IntervallChecker", "Excepted Calls/s: " + String(_actualSlavePointer->_repeatChecker->getEstCallPerSecond()), 1);
                    tempLogger.logIt("IntervallChecker", "Actual Calls/s: " + String(_actualSlavePointer->_repeatChecker->getCallPerSecond()), 1);
                    tempLogger.logIt("IntervallChecker", "######-END-######", 1);
                    _actualSlavePointer->_repeatChecker->setWarnIsReported(true);
                }
                
            }
            if(_actualSlavePointer->_repeatChecker->getCallPerSecond() >= _actualSlavePointer->_repeatChecker->getEstCallPerSecond())
            {
                if(_actualSlavePointer->_repeatChecker->getWarnIsReported())
                {
                    _actualSlavePointer->_repeatChecker->setWarnIsReported(false);
                    _workLed->ledOn();
                }
            }
        }
    public:
    /*
        ClassModuleMaster(ClassModuleSlave* _firstSlave, LED* errorLed, LED* workLed)
        {
            this->_firstSlave = _firstSlave;
        }
    */
        ClassModuleMaster()
        {
            LED dummy;
            this->_errorLed = &dummy;
            this->_workLed = &dummy;
        }

        ClassModuleMaster(LED* _errorLed, LED* _workLed)
        {
            this->_errorLed = _errorLed;
            this->_workLed = _workLed;
        }

        ClassModuleMaster(LED* _errorLed)
        {
            this->_errorLed = _errorLed;
        }

        void addModuleSlave(ClassModuleSlave* _newSlave)
        {
            if(this->_firstSlave == NULL)
            {
                _firstSlave = _newSlave;
            }
            else
            {
                _newSlave->setNextModuleSlave(_firstSlave);
                _firstSlave = _newSlave;
            }
        }

        void run()
        {
            ModuleControl();
            checkOpticalReport();
        }
};
#endif //J54J6_MODULESTATE_H