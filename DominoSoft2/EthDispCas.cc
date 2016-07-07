//Dragon readout software
//TCP/IP data transfer
//Event Display
//KONNO Yusuke
//Kyoto University

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <arpa/inet.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <pthread.h>

#include <iostream>
#include <fstream>

#include <TApplication.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <TAxis.h>

//#define BUFSIZE 256000
#define BUFSIZE 1024000
#define READDEPTH 68
//#define READDEPTH 1000
#define EVENTLENGTH 16*(2*READDEPTH+4) //Byte
#define UNITLENGTH EVENTLENGTH*10 //Byte
//#define TOTALDEPTH 1024
#define TOTALDEPTH 4096

using namespace std;

struct MyThreadParam{
	pthread_mutex_t *mutex;
	unsigned char *threadbuf;
	int *isEnd;
	int *okEnd;
	clock_t *clockstamp;
	time_t *timestamp;
	long *writebyte;
};

void createThread(pthread_t *thread, pthread_mutex_t *mutex, unsigned char *threadbuf, int *isEnd, int *okEnd, clock_t *clockstamp, time_t *timestamp, long* writebyte);

void *EventDisp(void *arg);

int main(int argc, char* argv[]){

	int event;
	int option;
	char* filename;
	unsigned char rcvdBuffer[BUFSIZE];

	char* sitcpIpAddr;
	struct sockaddr_in sitcpAddr;
	int fBytes = 0;
	int rBytes;
	FILE *fw;

	unsigned int sitcpPort;
	int unitLen;
	int sock;

	/* Check inut values */
	/* Get IP address and port # of a SiTCP */
	if(argc != 4){
		printf("Usage: %s <IP address> <event No> <filename>\n\n", argv[0]);
		exit(EXIT_FAILURE);
	}else{
		sitcpIpAddr = argv[1];
		event=atoi(argv[2]);
		filename = argv[3];
	}
	sitcpPort = 24;
	unitLen = UNITLENGTH;

	long eventlength = EVENTLENGTH;
	long writebyte=0;
	long writeevent=0;
	long writeeventTmp=0;

	/* Create a Socket */
	puts("\nCreate socket...\n");

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	sitcpAddr.sin_family      = AF_INET;
	sitcpAddr.sin_port        = htons(sitcpPort);
	sitcpAddr.sin_addr.s_addr = inet_addr(sitcpIpAddr);

	if(connect(sock, (struct sockaddr*) &sitcpAddr, sizeof(sitcpAddr))<0){
		puts("Connect() faild");
		close(sock);
		exit(EXIT_FAILURE);
	}

	fw = fopen(filename,"wb");
	char* rcvdBuffer_temp = reinterpret_cast<char*>(rcvdBuffer);

	TApplication app("app", &argc, argv);

	//Event Display Thread
	pthread_t thread;
	pthread_mutex_t *mutex;
	unsigned char *threadbuf;
	int *isEnd;
	int *okEnd;
	clock_t *clockstamp;
	time_t *timestamp;
	long* writebytemoni;

	mutex = (pthread_mutex_t *)malloc( sizeof(pthread_mutex_t) );
	pthread_mutex_init(mutex,NULL);
	threadbuf = (unsigned char*)malloc(BUFSIZE);
	isEnd = (int*)malloc(sizeof(int));
	okEnd = (int*)malloc(sizeof(int));
	*isEnd=0;
	*okEnd=0;
	clockstamp = (clock_t*)malloc(sizeof(time_t));
	timestamp = (time_t*)malloc(sizeof(time_t));
	writebytemoni = (long*)malloc(sizeof(long));
	*writebytemoni=0;

	createThread(&thread, mutex, threadbuf, isEnd, okEnd, clockstamp, timestamp, writebytemoni);
	pthread_detach(thread);

	//Read Data Loop
	while(1){
		for(fBytes = 0; fBytes<unitLen;fBytes+=rBytes){
			if((rBytes = recv(sock, rcvdBuffer_temp + fBytes, unitLen-fBytes, 0)) <= 0){
				puts(" ");
				puts("recv() faild");
				close(sock);
				exit(EXIT_FAILURE);
			}
		}

		for(int i=0;i<unitLen;i++){
			if(event>0)
			{
				fwrite(&rcvdBuffer[i],sizeof(unsigned char),1,fw); //write data
			}
		}

		writebyte+=unitLen;
		writeevent = writebyte/eventlength;
		if(pthread_mutex_trylock(mutex) == 0)
		{
			for(int i=0;i<eventlength;i++)
			{
				threadbuf[i] = rcvdBuffer[i];
			}
			time(timestamp);
			*clockstamp=clock();
			*writebytemoni=writebyte;
			pthread_mutex_unlock(mutex);
		}
		
		if(writeevent-writeeventTmp>100){
			fflush(fw);
			printf("%d\n",writeevent);
			writeeventTmp=writeevent;
		}
		if(event>0){
			if(writeevent>=event)
			{
				fclose(fw);
				close(sock);

				pthread_mutex_lock(mutex);
				*isEnd = 1;
				pthread_mutex_unlock(mutex);

				while(1)
				{
					pthread_mutex_lock(mutex);
						if(*okEnd == 1)
						{
							pthread_mutex_destroy(mutex);
							free(mutex);
							free(threadbuf);
							return 0;
						}
						else
						{
							pthread_mutex_unlock(mutex);
							sleep(1);
						}
				}
			}
		}
	}

	app.Run();

	return 0;
}

void createThread(pthread_t *thread, pthread_mutex_t *mutex, unsigned char *threadbuf, int *isEnd, int *okEnd, clock_t *clockstamp, time_t *timestamp, long* writebyte)
{
	struct MyThreadParam *param;
	unsigned int size;

	size = sizeof(struct MyThreadParam);
	param = (struct MyThreadParam *)malloc(size);

	if(param)
	{
		param->mutex = mutex;
		param->threadbuf = threadbuf;
		param->isEnd = isEnd;
		param->okEnd = okEnd;
		param->clockstamp = clockstamp;
		param->timestamp = timestamp;
		param->writebyte = writebyte;

		pthread_create(thread, NULL, &EventDisp, param);

		pthread_detach(*thread);
	}

}

void *EventDisp(void *arg)
{
	struct MyThreadParam *threadParam;
	threadParam = (struct MyThreadParam *)arg;

	string offsetfile = "offset.dat";
	double offset[16][4096];

	ifstream ofs(offsetfile.c_str());
	for(int j=0;j<4096;j++)
	{
		for(int i=0;i<16;i++)
		{
			ofs >> offset[i][j];
		}
	}

	TGraph* gWave[16];
	for(int i=0;i<16;i++)
	{
		gWave[i] = new TGraph();
	}
	TGraph* gPacket = new TGraph();
	TCanvas* cHG = new TCanvas("cHG","High Gain",0,0,1800,200);
	TCanvas* cLG = new TCanvas("cLG","Low Gain",0,250,1800,200);
	TCanvas* cTAG = new TCanvas("cTAG","TIME TAG",0,480,400,200);
	TCanvas* cPacket = new TCanvas("cPacket", "Packet Monitor",1500,500,400,250);
	cHG->Draw();
	cHG->Divide(8);
	cLG->Draw();
	cLG->Divide(8);
	cTAG->Draw();
	cTAG->Divide(2);

	//Event Display Loop
	int buf1, buf2, buf3;
	int cell[8];
	double subtADC;
	clock_t cprev, cnow;
	time_t tprev, tnow, tstart;
	tstart = time(NULL);
	tprev = tstart;
	tnow = tstart;
	cnow=clock();
	cprev=cnow;
	long writebyteprev=0;
	long writebyte=0;
	long writebytediff=0;
	double tdiff;
	double cdiff;
	double tfromstart;
	double datarate;
	int ntime=0;
	char* tstr;
	char* tstr2;

	UInt_t pps_c;
	UInt_t tenMhz_c;
	UInt_t event_c; //event counter
	UInt_t trigger_c; //trigger counter
	ULong_t clock_c; //clock counter

	while(1)
	{
		pthread_mutex_lock(threadParam->mutex);
		
		if(*(threadParam->isEnd)==1)
		{
			cout << "Event Display thread finish!" << endl;
			*(threadParam->okEnd)=1;
			pthread_mutex_unlock(threadParam->mutex);
			return 0;
		}

		writebyteprev=writebyte;
		writebyte=*(threadParam->writebyte);
		writebytediff=writebyte-writebyteprev;

		tprev=tnow;
		tnow=*(threadParam->timestamp);
		tdiff=difftime(tnow,tprev);
		tfromstart=difftime(tnow,tstart);

		cprev=cnow;
		cnow=*(threadParam->clockstamp);
		cdiff=1.0*(cnow-cprev)/CLOCKS_PER_SEC;

		if(tdiff!=0 && cdiff>0 && writebytediff>0)
		{
			if(ntime>0)
			{
				datarate=8*writebytediff/cdiff; //bps
				gPacket->SetPoint(ntime-1,tfromstart,datarate);
				cout << "writebytediff:" << writebytediff << " cdiff:" << cdiff << " cnow:" << cnow << " cprev:" << cprev << " CLOCKS_PER_SEC:" << CLOCKS_PER_SEC  <<endl;
			}
			ntime++;
		}
		else
		{
			pthread_mutex_unlock(threadParam->mutex);
			continue;
		}

		pps_c=0;
		for(int i=0;i<2;i++)
		{
			pps_c*=256;
			buf1=threadParam->threadbuf[2+i];
			pps_c+=buf1;
		}
		tenMhz_c=0;
		for(int i=0;i<4;i++)
		{
			tenMhz_c*=256;
			buf1=threadParam->threadbuf[4+i];
			tenMhz_c+=buf1;
		}
		for(int i=0;i<4;i++)
		{
			event_c*=256;
			buf1=threadParam->threadbuf[8+i];
			event_c+=buf1;
		}
		for(int i=0;i<4;i++)
		{
			trigger_c*=256;
			buf1=threadParam->threadbuf[12+i];
			trigger_c+=buf1;
		}
		for(int i=0;i<8;i++)
		{
			clock_c*=256;
			buf1=threadParam->threadbuf[16+i];
			clock_c+=buf1;
		}
		cout << "PPS Count : " << pps_c << endl;  
		cout << "TenMHz Count : " << tenMhz_c << "*100nsec = " << tenMhz_c*100/1000000 << "msec" <<  endl;  
		cout << "Event Count : " << event_c << endl;
		cout << "Trigger Count : " << trigger_c << endl;
		cout << "Clock Count : " << clock_c << "*7.5nsec = " << clock_c*7.5/1000000 << "msec" << endl;

		for(int i=0;i<8;i++)
		{
			buf1=threadParam->threadbuf[16*3+2*i];
			buf2=threadParam->threadbuf[16*3+2*i+1];
			buf3=buf1*256+buf2;
			cell[i]=buf3;
			cout << "capacitorID:" << cell[i] << endl;

			for(int j=0; j<READDEPTH ;j++)
			{
				buf1=threadParam->threadbuf[16*4+16*j+2*i];
				buf2=threadParam->threadbuf[16*4+16*j+2*i+1];
				buf3=buf1*256+buf2;

				subtADC=buf3-offset[i][(cell[i]+j)%TOTALDEPTH];
				//subtADC=buf3;

				//gWave[i]->SetPoint(j,j,subtADC);
				gWave[i]->SetPoint(j,cell[i]+j,subtADC);
				//cout << "chip:" << i << " timeslice:" << j << " subtADC:" << subtADC << " buf1:" << buf1 << " buf2:" << buf2 << " buf3:" << buf3 << endl;

				buf1=threadParam->threadbuf[16*4+16* READDEPTH + 16*j+2*i];
				buf2=threadParam->threadbuf[16*4+16* READDEPTH + 16*j+2*i+1];
				buf3=buf1*256+buf2;

				subtADC=buf3-offset[i+8][(cell[i]+j)%TOTALDEPTH];
				//subtADC=buf3;
				
				//gWave[8+i]->SetPoint(j,j,subtADC);
				gWave[8+i]->SetPoint(j,cell[i]+j,subtADC);
			}
		}

		pthread_mutex_unlock(threadParam->mutex);

		/*
		for(int i=0;i<8;i++)
		{
			cHG->cd(i+1);
			gWave[i]->Draw("al");

			cLG->cd(i+1);
			gWave[8+i]->Draw("al");
		}
		*/

		cHG->cd(1);
		gWave[0]->Draw("al");
		gWave[0]->SetTitle("HG1");

		cHG->cd(2);
		gWave[8]->Draw("al");
		gWave[8]->SetTitle("HG2");

		cHG->cd(3);
		gWave[2]->Draw("al");
		gWave[2]->SetTitle("HG3");

		cHG->cd(4);
		gWave[10]->Draw("al");
		gWave[10]->SetTitle("HG4");

		cHG->cd(5);
		gWave[4]->Draw("al");
		gWave[4]->SetTitle("HG5");

		cHG->cd(6);
		gWave[12]->Draw("al");
		gWave[12]->SetTitle("HG6");

		cHG->cd(7);
		gWave[6]->Draw("al");
		gWave[6]->SetTitle("HG7");

		cLG->cd(1);
		gWave[1]->Draw("al");
		gWave[1]->SetTitle("LG1");

		cLG->cd(2);
		gWave[9]->Draw("al");
		gWave[9]->SetTitle("LG2");

		cLG->cd(3);
		gWave[3]->Draw("al");
		gWave[3]->SetTitle("LG3");

		cLG->cd(4);
		gWave[11]->Draw("al");
		gWave[11]->SetTitle("LG4");

		cLG->cd(5);
		gWave[5]->Draw("al");
		gWave[5]->SetTitle("LG5");

		cLG->cd(6);
		gWave[13]->Draw("al");
		gWave[13]->SetTitle("LG6");

		cLG->cd(7);
		gWave[7]->Draw("al");
		gWave[7]->SetTitle("LG7");

		cTAG->cd(1);
		gWave[14]->Draw("al");
		gWave[14]->SetTitle("HG TAG");

		cTAG->cd(2);
		gWave[15]->Draw("al");
		gWave[15]->SetTitle("LG TAG");

		cPacket->cd();
		gPacket->Draw("al");
		gPacket->SetTitle("Data Rate");
		gPacket->GetXaxis()->SetTitle("Time [sec]");
		gPacket->GetYaxis()->SetTitle("Data Rate [bps]");
		gPacket->GetYaxis()->SetTitleOffset(1.3);

		cHG->Update();
		cLG->Update();
		cTAG->Update();
		cPacket->Update();

		sleep(1);
	}
}

