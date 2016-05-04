//KONNO Yusuke
//Dragon readout software
//Sub function for Domino readout
//KONNO Yusuke
//Kyoto University
//last update: 2013.05.26

#include "DSubFunc.h"

using namespace std;

//ch number to chip number in the data format

int chconv(int ch)
{
	int chip = -2;
	if((ch>15) || (ch<0)){
		cout << "ch : (high gain:0-7 low gain:8-15)" << endl;
		return -1;
	}else if((ch<=7)&&(ch%2==0)){
		chip = 7-ch;
	}else if((ch<=7)&&(ch%2==1)){
		chip = 16-ch;
	}else if((ch>7)&&(ch%2==0)){
		chip = 14-ch;
	}else if((ch>7)&&(ch%2==1)){
		chip = 23-ch;
	}

	return chip;
}

//check number of events
int checkeventnum(const char* filename)
{
	int eventnum;
	double tmp;
	size_t fileSize;
	ifstream* fin;
	fin = new ifstream(filename, ios::in | ios::binary);
	if(!fin)
	{
		cout << "Cannot Open File: " << filename  << endl;
		return -1;
	}
	fileSize = (size_t)fin->seekg(0, std::ios::end).tellg();
	int footer=4;
	int data_per_ev =16*(2*READDEPTH +footer);
	//int data_per_ev =2+2+4+4+4+8+8+2*8+2*8+2*8*2*READDEPTH; //bytes
	tmp = (double)fileSize/data_per_ev;
	eventnum = (int)tmp;
	cout << "Number of Events: " << eventnum << endl;

	fin->close();
	delete fin;
	return eventnum;
}

bool EvLoopHandler(int& event)
{
	TTimer timer("gSystem->ProcessEvents();",50,kFALSE);
	timer.TurnOn();

	char ch;
	if ((ch=cin.get()) == '\n') event++;
	else if(ch=='q')  return false;
	else
	  {
	    if(ch=='b') event--;
	    while(cin.get()!='\n') ;
	  }
	timer.TurnOff();
	return true;
}

char* BaseName(char *name)
{
  if (name) {
    if (name[0] == '/' && name[1] == '\0')
      return name;
    char *cp;
    if ((cp = (char*)strrchr(name, '/')))
      return ++cp;
    return name;
  }
  cerr << "error: BaseName"<<endl;
  return 0;
}
