/*
Module : AAPLUTO.H
Purpose: Implementation for the algorithms which obtain the heliocentric position of Pluto
Created: PJN / 29-12-2003

Copyright (c) 2003 - 2013 by PJ Naughter (Web: www.naughter.com, Email: pjna@naughter.com)

All rights reserved.

Copyright / Usage Details:

You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise) 
when your product is released in binary form. You are allowed to modify the source code in any way you want 
except you cannot modify the copyright details at the top of each module. If you want to distribute source 
code with your application, then you are only allowed to distribute versions released by the author. This is 
to maintain a single distribution point for the source code. 

*/


/////////////////////// Macros / Defines //////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef __AAPLUTO_H__
#define __AAPLUTO_H__

#ifndef AAPLUS_EXT_CLASS
#define AAPLUS_EXT_CLASS
#endif


/////////////////////// Classes ///////////////////////////////////////////////
namespace AAPlus {

class AAPLUS_EXT_CLASS CAAPluto
{
public:
//Static methods
  static double EclipticLongitude(double JD);
  static double EclipticLatitude(double JD);
  static double RadiusVector(double JD);
};

}
#endif //__AAPLUTO_H__
