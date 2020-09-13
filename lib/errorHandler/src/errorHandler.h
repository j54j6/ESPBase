#ifndef J54J6_ERROR_HANDLER_H
#define J54J6_ERROR_HANDLER_H
#include <Arduino.h>
#include "logging.h"
#include "led.h"

struct classErrorReport {
    bool error = false; // set to true if want to get underControl of ErrorHandler or wnat to report Error in other Case ErrorHandler will ignore given values - need to implement stopClass(), waitClass(), pauseClass(), startClass(), restartClass(), continueClass()
    int ErrorCode = -1;
    short priority = -1; /*
                            priority: 0 for ignoring like bool error = false
                                min: 1 - Class can work without problems
                                     2 - Class can work but with less functionality
                                     3 - Class can't work but try to fix it automatically - Handler wait for further Error - after 3 times -> auto set to prio 4 -> class restart
                                     4 - Class can't work, theoretically able to fix problem but failed - only restart Class by Handler
                                     5 - Class can't work, can't fix it automatically - only restart of class by Handler
                                     6 - Class can't work, can't fix it automatically - device can be Damaged - ErrorHandler stop complete Device
                                    
                            */
    const char* message = "";
};

class ErrorSlave {
    protected:
        bool errorReported = false;
        bool warnReported = false;
        ErrorSlave* _next = _NULL;
        bool lockClass = false; //must implemented! - if true - no class function should be able to work
        const char* className = "placeholder";

    public:
        classErrorReport error;
        void setErrorReported(bool val)
        {
            this->errorReported = val;
        }

        void setWarnReported(bool val)
        {
            this->warnReported = val;
        }

        bool getErrorReported()
        {
            return this->errorReported;
        }

        bool getWarnReported()
        {
            return this->warnReported;
        }

        void resetErrorReport()
        {
            error.error = false;
            error.ErrorCode = _NULL;
            error.priority = -1;
            error.message = _NULL;
        }

        classErrorReport getCurrentErrorState() //return per Class error
        {
            return this->error;
        }
        ErrorSlave* getThisAsErrorClass()
        {
            ErrorSlave* node = this;
            return node;
        }

        ErrorSlave* getINode()
        {
            return this;
        }

        ErrorSlave* getNext()
        {
            return _next;
        }
        void setNext(ErrorSlave* next)
        {
            this->_next = next;
        }

        const char* getClassName()
        {
            return className;
        }

        void setClassName(const char* newVal)
        {
            this->className = newVal;
        }
        virtual void startClass() = 0;
        virtual void stopClass() = 0;
        virtual void pauseClass() = 0;
        virtual void restartClass() = 0;
        virtual void continueClass() = 0;
};

class ErrorHandler {
    private:
        LED* _errorLed;
        LED* _workLed;
        ErrorSlave* _firstNode;
        /*
            If ErrorSlave::classErrorReport.error = true - every timewill logged and Optical Message will sent,
        */
        int _level = 0; /* 
                            _level is to set the "critical"_Level from which the Errorhandler will report Warnings and Errors
                            Handling like priority of ErrorSlave:classErrorReport
                            _level {0 - 6}
                            Report if n >= _level -> with ReportTypes defiend below
                        */

        bool opticalOnWarn = true; //set Optical message when ErrorSlave::classErrorReport.error = false and ErrorSlave::classErrorReport.priority > _level
        bool SerialOnWarn = true; //Serial Message when ErrorSlave::classErrorReport.error = false and ErrorSlave::classErrorReport.priority > _level

    public:
        ErrorHandler(ErrorSlave* firstNode, LED* errorLed, LED* workLed):_errorLed(errorLed), _workLed(workLed),_firstNode(firstNode){_workLed->ledOn();};

        ErrorHandler(ErrorSlave* firstNode):_firstNode(firstNode){};


        void addNewNode(ErrorSlave* newNode, const char* name)
        {
            ErrorSlave* actualNode = _firstNode;

            int counter = 1;

            while(!_firstNode->getNext() == _NULL)
            {
                if(actualNode->getNext() == _NULL)
                {
                    break;
                }
                else
                {
                    actualNode = actualNode->getNext();
                }
                counter++;
            }        
            actualNode->setNext(newNode);
        }

        void checkForError()
        {
            classErrorReport actualError;
            ErrorSlave* actualNode = _firstNode;

            while(!actualNode->getNext() == _NULL)
            {
                if(actualNode->getCurrentErrorState().error == true)
                {
                    preFormattedErrorNotification(actualNode->getClassName(), actualNode->getCurrentErrorState().message, actualNode->getCurrentErrorState().priority, actualNode->getCurrentErrorState().ErrorCode);
                    _errorLed->ledOn();
                    _workLed->ledOff();
                    actualNode->setErrorReported(true);
                    if(actualNode->error.priority == 4 || actualNode->error.priority == 5)
                    {
                        actualNode->restartClass();
                    }
                    else if(actualNode->error.priority == 6)
                    {
                        actualNode->stopClass();
                    }
                    actualNode->resetErrorReport();
                    return;
                }
                if(actualNode->getCurrentErrorState().error == false && actualNode->getCurrentErrorState().priority >= _level)
                {
                    if(SerialOnWarn)
                    {
                        preFormattedWarnNotification(actualNode->getClassName(), actualNode->getCurrentErrorState().message, actualNode->getCurrentErrorState().priority, actualNode->getCurrentErrorState().ErrorCode);
                    }
                    if(opticalOnWarn)
                    {
                        _errorLed->blink(1000);
                        _workLed->ledOff();
                    }
                    actualNode->setWarnReported(true);
                    actualNode->resetErrorReport();
                }

                if(actualNode->getNext() == _NULL)
                {
                    break;
                }
                actualNode = actualNode->getNext();
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

#endif