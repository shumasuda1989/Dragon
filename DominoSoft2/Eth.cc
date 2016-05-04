//Dragon readout software
//TCP/IP data transfer
//KONNO Yusuke
//Kyoto University
//last update: 2013.05.26


using namespace std;


#define Linux

#ifdef Windows
 #define __USE_W32_SOCKETS

 #include <stdio.h>
 #include <winsock.h>
 #include <fstream>


#else
 #include <cstdlib>
 #include <cstdio>
 #include <cstring>
 #include <iostream>

 #include <arpa/inet.h>
 #include <unistd.h>

 #include <sys/types.h>
 #include <sys/socket.h>
 #include <netinet/in.h>
#endif

#define BUFSIZE 256000
//#define READDEPTH 1024
int main(int argc, char* argv[]){

	int READDEPTH;
	int event;
	int option;
	char* filename;
	unsigned char rcvdBuffer[BUFSIZE];
  char* sitcpIpAddr;
  int unitLen;
  struct sockaddr_in sitcpAddr;
  int i;
	int k=0, l=0;
  int fBytes = 0;
  int rBytes;
  //ifstream fin;
  //ofstream fout("./hoge.dat", ios::in | ios::binary);
  FILE *fw;
//  FILE *fd;
#ifdef Windows
  u_short sitcpPort;
  WSADATA wsaData;
  SOCKET sock;
 
#else
  unsigned int sitcpPort;
  int sock;

#endif

  /* Check inut values */
  /* Get IP address and port # of a SiTCP */
  if(argc != 8){
    printf("Usage: %s <IP address> <Destination Port #> <Unit length (byte)> <event No> <READEPTH> <option 0:datarun 1:warmup> <filename>\n\n", argv[0]);
    exit(EXIT_FAILURE);
  }else{
    sitcpIpAddr = argv[1];
    sitcpPort   = (u_short)atoi(argv[2]);
    unitLen     = atoi(argv[3]);
	  event=atoi(argv[4]);
		READDEPTH=atoi(argv[5]);
		option=atoi(argv[6]);
		filename = argv[7];
  }

  /* Create a Socket */
  puts("\nCreate socket...\n");

#ifdef Windows
  if(WSAStartup(MAKEWORD(1,1),&wsaData)){
    puts("ERROR:WSAStartup()");
    WSACleanup();
    return -1;
  }
#endif

  sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  sitcpAddr.sin_family      = AF_INET;
  sitcpAddr.sin_port        = htons(sitcpPort);
  sitcpAddr.sin_addr.s_addr = inet_addr(sitcpIpAddr);

  if(connect(sock, (struct sockaddr*) &sitcpAddr, sizeof(sitcpAddr))<0){
    puts("Connect() faild");
#ifdef Windows
    closesocket(sock);
    WSACleanup();
#else
    close(sock);
#endif
    exit(EXIT_FAILURE);
  }

//  fd = fopen("./debug.dat","w");
  fw = fopen(filename,"wb");
  char* rcvdBuffer_temp = reinterpret_cast<char*>(rcvdBuffer);

  while(1){
    for(fBytes = 0; fBytes<unitLen;fBytes+=rBytes){
      if((rBytes = recv(sock, rcvdBuffer_temp+fBytes, unitLen-fBytes, 0)) <= 0){
	puts(" ");
	puts("recv() faild");
	
#ifdef Windows
	closesocket(sock);
	WSACleanup();
#else
	close(sock);
#endif
	exit(EXIT_FAILURE);
      }
    }
	//for(i=0;i<unitLen;i++) printf("%.2x",rcvdBuffer[i]);
    //printf("\n");
	
/*	for(i=0;i<unitLen;i++) {fprintf(fd,"%.3u",rcvdBuffer[i]);
	fprintf(fd," ");}
	fprintf(fd,"%s","\n");
	*/

	for(i=0;i<unitLen;i++){
		if(option==0)
		{
		  fwrite(&rcvdBuffer[i],sizeof(unsigned char),1,fw); //write data
		}

//		fprintf(fd,"%.3u",rcvdBuffer[i]);
//		fprintf(fd," ");
//		fout.put(rcvdBuffer[i]);
	}
//		fprintf(fd,"%s","\n");

	if(k==(READDEPTH*2)+1){
		l++;
		k=0;
		fflush(fw);


		if((l%100)==0){
		  //printf("%u\n",l);
		  cout << "\r" << l << flush;
		}

		if(option==0){
			if(l==event){
			  puts("\n\nClose socket");
			  return 0;
			}
		}
	}
	else
	{
		k++;
	}



	//i=0;

	//while(i<unitLen) {cout<<hex<<rcvdBuffer[i];i++;}

	//cout<<rcvdBuffer;
	
  }

#ifdef Windows
  closesocket(sock);
  WSACleanup();
//  fclose(fd);
  fclose(fw);
  //fout.close();
#else
  close(sock);
#endif
  return 0;
}

