#ifndef __INCLUDE_H__
#define __INCLUDE_H__

#include <Wire.h>
#include <SoftwareSerial.h>
#include <math.h>
#include <SD.h>
#include <EEPROM.h>
#include <Servo.h>
#include "TYPEDEF.h"
#include "FLAG.h"
#include "DEFINE.h"

#if XB_LIB == 0
#include <xbee.h>

#elif XB_LIB == 1
#include <xbee_mod.h>

#endif

#endif
