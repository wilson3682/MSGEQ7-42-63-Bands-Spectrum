/********************************************************************************************************************************
*
*  Project:  21 Band Spectrum Analyzer 
*  Target Platform: ESP32 38-PIN DEV Board
*  Version: 8.0
*  
********************************************************************************************************************************/

#pragma once
#include "Settings.h"

//**************************************************************************************************
//                                          D B G P R I N T                                        *
//**************************************************************************************************
// Send a line of info to serial output.  Works like vsprintf(), but checks the DEBUG flag.        *
// Print only if DEBUG flag is true.  Always returns the formatted string.                         *
// Usage dbgprint("this is the text you want: %d", variable);
//**************************************************************************************************
char * dbgprint(const char * format, ...) {
  if (DEBUG) {
    static char sbuf[DEBUG_BUFFER_SIZE];             // For debug lines
    va_list varArgs;                                 // For variable number of params
    va_start(varArgs, format);                       // Prepare parameters
    vsnprintf(sbuf, sizeof(sbuf), format, varArgs);  // Format the message
    va_end(varArgs);                                 // End of using parameters
    if (DEBUG)                                       // DEBUG on?
    {
      Serial.print("Debug: ");                       // Yes, print prefix
      Serial.println(sbuf);                          // and the info
    }
    return sbuf;                                     // Return stored string
  }
}
