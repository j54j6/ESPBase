#ifndef J54J6_MODULESTATE_H
#define J54J6_MODULESTATE_H

#include <Arduino.h>
#include "errorHandler.h"
#include "led.h"

/*
    ClassReportTemplate

    Object contains all Data of one Report 
    ClassReportTemplate is part of ordered List with Pointer
*/
struct ClassReportTemplate {
    friend class ClassReportModuleHandler;
    friend class ClassModuleMaster;
    private:
        const char* className = "n.S";
        ClassReportTemplate* _nextNode = NULL;
        bool error = false;
        const char* message = NULL;
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
            if(!reportIsLocked || (reportIsLocked && this->_nextNode == NULL))
            {
               this->_nextNode = _nextClassReport; 
            }
        }

        //Get
        bool getError()
        {
            return this->error;            
        }

        const char* getErrorMessage()
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
        ClassReportTemplate(const char* className, const char* message, short errorCode, bool isError)
        {
            this->className = className;
            this->message = message;
            this->errorCode = errorCode;
            this->error = isError;
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
            if(this->_firstReport == NULL)
            {
                return;
            }

            if(this->_actualReportPointer != NULL && this->_firstReport != NULL)
            {
                ClassReportTemplate* _cachePointer = _firstReport;

                if(_cachePointer == _firstReport && _cachePointer->getNextClassReport() == NULL) //first Node is the report and the only registered - remove complete List (of 1 Item)
                {
                    _firstReport = NULL;
                    _actualReportPointer = NULL;
                }
                else if(_cachePointer == _firstReport && _cachePointer->getNextClassReport() != NULL) //first Node is the report - set the second report as first Node
                {
                    _firstReport = _firstReport->getNextClassReport();
                    _actualReportPointer = _firstReport;
                }
                else //actual Node in not the first Node
                {
                    while (_cachePointer->getNextClassReport() != NULL && _cachePointer->getNextClassReport() != _actualReportPointer) //set CachePointer to 1 Report above the actualReport
                    {
                        _cachePointer = _cachePointer->getNextClassReport();
                    }

                    if(_cachePointer->getNextClassReport()->getNextClassReport() == NULL) 
                    //if actualReport (_cachePointer->getnextClassReport()) don't have a next Node (_cachePointer->getNextClassReport()->getNextClassReport()) set prevNode of actualNode to NULL
                    {
                        _cachePointer->setNextClassReport(NULL);
                        _actualReportPointer = _cachePointer;
                    }
                    else
                    {
                        _cachePointer->setNextClassReport(_cachePointer->getNextClassReport()->getNextClassReport());
                    }
                    
                }               
            }
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

        void addReport(const char* className, const char* message, short errorCode, bool isError, bool lockReport = false)
        {
            ClassReportTemplate newTempl(className, message, errorCode, isError);
            if(_firstReport == NULL)
            {
                this->_firstReport = &newTempl;
                this->_actualReportPointer = _firstReport;
            }
            else
            {
                getLastNode()->setNextClassReport(&newTempl);
            }

            if(lockReport)
            {
                newTempl.lockReport();
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
        const char* className = "n.S";
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
        ModuleStateSlave* _nextNode;

        
        
    protected:
        void setNextModuleNode(ModuleStateSlave* newNextNode)
        {
            this->_nextNode = newNextNode;
        }

        ModuleStateSlave* getNextModuleNode()
        {
            return _nextNode;
        }
        
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

        virtual void stopClass();
        virtual void startClass();
        virtual void restartClass();
    
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
        
        void internalRun()
        {
            this->callPerSecond = 1000/(millis() - lastCall);
        }
    public:
        ulong getCallPerSecond()
        {
            return this->callPerSecond;
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
*/
class ClassModuleSlave {
    friend class ClassModuleMaster;
    private:
        const char* className = "n.S";
        ClassModuleSlave* _nextSlave;

        ClassReportModuleHandler reportHandler;
        ClassRepeatChecker repeatChecker;

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
        
        void run()
        {
            this->repeatChecker.run();
        }

    public:
        ClassModuleSlave(const char* className)
        {
            this->className = className;
        }

        //reporting
        void newReport(const char* message, short errorCode, bool isError = false, bool lockReport = true)
        {
            reportHandler.addReport(className, message, errorCode, isError, lockReport);
        }

        //repeat checker
        ulong getCallsPerSecond()
        {
            return this->repeatChecker.getCallPerSecond();
        }
};


class ErrorHandlerMaster {
    private:
        LED* _errorLed;
        LED* _workLed;
    
    public:
        ErrorHandlerMaster()
        {
            LED dummy;
            this->_errorLed = &dummy;
            this->_workLed = &dummy;
        }

        ErrorHandlerMaster(LED* _errorLed, LED* _workLed)
        {
            this->_errorLed = _errorLed;
            this->_workLed = _workLed;
        }

        void reportError(const char* className, const char* message, short priority, int code)
        {
            _errorLed->ledOn();
            _workLed->ledOff();
            logError(className, message, priority, code);
        }

        void logError(const char* className, const char* message, short priority, int code)
        {
            if(!Serial)
            {
                Serial.begin(115200);
            }


        }

        const char* getErrorHandlerReaction(int prio)
        {
            switch(prio)
            {
                case 4:
                    return "restart Class";
                    break;
                case 5:
                    return "restart Class";
                    break;
                case 6:
                    return "disable Class";
                    break;
                default:
                    return "nothing";
                    break;
            };
            return "false";
        }

        void preFormattedWarnNotification(const char* className , const char* message, int priority, int code)
        {
            String modifiedClassName = "ERROR-Handler";
            modifiedClassName += "->";
            modifiedClassName += className;
            logger logging;
            String FormatMessage = "\n\n\n##########-WARN-##########\n\n";
            FormatMessage += "Reporting Class: ";
            FormatMessage += className;
            FormatMessage += "\nInternal Class-Priority: ";
            FormatMessage += priority;
            FormatMessage += "\nReason: ";
            FormatMessage += message;
            FormatMessage += "\nError-Code: ";
            FormatMessage += code;
            FormatMessage += "\nAction: nothing - only WARN";
            FormatMessage += "\n\n###########-END-###########\n\n";
            logging.SFLog(modifiedClassName.c_str(), "ERROR-HANDLER-LOG", FormatMessage.c_str(), 1);
            return;
        }

        void preFormattedErrorNotification(const char* className, const char* message, int priority, int code, bool opticalReport = true)
        {
            String modifiedClassName = "ERROR-Handler";
            modifiedClassName += "->";
            modifiedClassName += className;
            logger logging;
            String FormatMessage = "\n\n\n\n##########-ERROR-##########\n\n";
            FormatMessage += "Reporting Class: ";
            FormatMessage += className;
            FormatMessage += "\nInternal Class-Priority: ";
            FormatMessage += priority;
            FormatMessage += "\nOptical Report: ";
            FormatMessage += opticalReport;
            FormatMessage += "\nReason: ";
            FormatMessage += message;
            FormatMessage += "\nError-Code: ";
            FormatMessage += code;
            FormatMessage += "\n Action: ";
            FormatMessage += getErrorHandlerReaction(priority);
            FormatMessage += "\n\n###########-END-###########\n\n\n";
            logging.SFLog(modifiedClassName.c_str(), "ERROR-HANDLER-LOG", FormatMessage.c_str(), 2);
            return;
        }
};



/*
    ClassModuleMaster

    only one instance is needed in the "main" Class - The Master control all registered Classes and Report Errors and States
*/

/*
class Test {
    ClassModuleSlave test2;

    void test()
    {
        test2.
    }
}
*/
class ClassModuleMaster {
    private:
        ClassModuleSlave* _firstSlave;
        ClassModuleSlave* _actualSlavePointer;
        ErrorHandlerMaster errorHandler;

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

        void reportControl()
        {
            if(_actualSlavePointer == NULL)
            {
                return;
            }

            _actualSlavePointer->reportHandler.toFirstReport(); //set to First Report to increment the complete list
            ClassReportModuleHandler* cacheReportHandler = _actualSlavePointer->getReportHandler();

            while(cacheReportHandler->getActualReport() != NULL)
            {
                ClassReportTemplate* actualReport = cacheReportHandler->getActualReport();

                
            }
        }

    protected:
        void removeActualReport()
        {
            
        }

    public:
        ClassModuleMaster(ClassModuleSlave* _firstSlave)
        {
            this->_firstSlave = _firstSlave;
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
            reportControl();
        }
    

};
#endif //J54J6_MODULESTATE_H