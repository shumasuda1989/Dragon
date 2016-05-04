//Dragon readout software
//make offset table
//KONNO Yusuke
//Kyoto University
//2012.12.27
//time stamp included format

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <cstdlib>

using namespace std;

//#define READDEPTH 60
//#define READDEPTH 1024
//#define TOTALDEPTH 1024
#define TOTALDEPTH 4096

int Mkoffset(const char* ifile = "hoge.dat", const char* ofile = "offset.dat", const long READDEPTH=1024);

int main(int argc, char* argv[])
{
	if(argc != 4){
		cout << endl << "Usage: " << argv[0] << " <input filename> <output filename> <read depth>" << endl;
		return -1;
	}

	char* ifile = argv[1];
	char* ofile = argv[2];
	long READDEPTH = atoi(argv[3]);

	cout << "ifile:" << ifile << " ofile:" << ofile  << " READDEPTH:" << READDEPTH << endl;

	Mkoffset(ifile, ofile, READDEPTH);

	return 0;
}
	
int Mkoffset(const char* ifile, const char* ofile, const long READDEPTH){

	int* stop = new int[8];
	double **x = new double*[16];
	long **count = new long*[16];
	for(int i=0;i<16;i++)
	{
		x[i] = new double[4096];
		count[i] = new long[4096];
	}

	ofstream fout(ofile);
	ifstream fin(ifile, ios::in | ios::binary);

	if(!fin){
		cout << "cannot open file" << endl;
		return 1;
	}

	for(int i=0;i<8;i++){
		stop[i] = 0;
	}
	for(int i=0;i<16;i++){
		for(int j=0;j<4096;j++){
			x[i][j] = 0;
			count[i][j] = 0;
		}
	}

	fin.seekg((16*(2*READDEPTH+4)),ios::cur);//skip first event
	
	int tmp=0;
	int buf=0;
	int event=0;
	int ch=0;
	while(!fin.eof()) {

		//get first cell numbers
		fin.seekg((16*3),ios::cur);

		if(fin.eof()){
			cout << "eof" << endl;
			break;
		}
		if(fin.bad()){
			cout << "bad" << endl;
			break;
		}
		if(fin.fail()){
			cout << "fail" << endl;
			break;
		}
		
		for(int i=0;i<8;i++){
			fin.read((char*)&buf,1);
			buf <<= 8;
			fin.read((char*)&buf,1);

			stop[i] = buf;
			buf = 0;
		}

		//get ADC counts
		for(int k=0;k<2;k++){
			for(int j=0;j<READDEPTH;j++){
				for(int i=0;i<8;i++){
					fin.read((char*)&buf,1);
					buf <<= 8;
					fin.read((char*)&buf,1);
					
					if(j>2){	//skip fist 3 cells

						ch = stop[i]/1024;
						tmp=(stop[i]+j)%TOTALDEPTH;
						//tmp = 1024 * ch + ((stop[i]+j)%1024);
						
						if((tmp<0||tmp>4095))
							cout << "nyaaaaaaaaaaaaaaaaaaaaaaaaaan" << "1024*" << ch << "+((" << stop[i] << "+" << j << ")%1024=" << tmp << "k=" << k << "i=" << i << endl;
						x[i+8*k][tmp] += buf;
						count[i+8*k][tmp]++;
					}
					tmp = 0;
					buf = 0;
				}
			}
		}

		event++;
		if(event%1000==0)
		{
			cout << "events: " << event << endl;
			//break;
		}

	}

			cout << "total: " << event << endl;
			cout << "counts for each capacitor (DRS4_chip7_channel0): " << endl;

	for(int j=0;j<4096;j++){
		for(int i=0;i<16;i++){
			if(count[i][j]==0){
				x[i][j] = 0;
				if(j<TOTALDEPTH)
					cout << endl << "-------------warning! 0 count capacitor exists!----------------------" << endl;
			}else{
				x[i][j] /= count[i][j];
				if(i==8 || i==9)
					x[i][j] -= 0; //TAGmean
			}

			fout.width(9);
			fout << x[i][j] << " ";

			if(j<TOTALDEPTH && i==15)
				cout << count[i][j] << " ";

		}
		fout << endl;
	}
	cout << endl;

	cout << "nyan" << endl;
	fin.close();
	cout << "nyan" << endl;
	//fout.close();
	cout << "nyan" << endl;

	delete[] stop;
	cout << "nyanstop" << endl;
	for(int i=0;i<16;i++)
	{
		delete[] x[i];
	cout << "nyanx" << i << endl;
		delete[] count[i];
	cout << "nyancount" << i << endl;
	}
	delete[] x;
	cout << "nyanx" << endl;
	delete[] count;
	cout << "nyancount" << endl;

	return 0;
}

