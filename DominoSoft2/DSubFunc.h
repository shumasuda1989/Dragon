//KONNO Yusuke
//Dragon readout software
//Sub function header for Domino readout
//KONNO Yusuke
//Kyoto University
//last update: 2013.05.26

#ifndef DSubFunc_H
#define DSubFunc_H

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <sstream>
#include <math.h>
#include <vector>

#include <TTimer.h>

#include "Config.h"

int chconv(int ch);
int checkeventnum(const char* filename);
bool EvLoopHandler(int& event);
char* BaseName(char *name);

#endif
