// intended to be compiled with Open Watcom
// https://github.com/open-watcom/open-watcom-v2

#include <conio.h>
#include <dos.h>
#include <i86.h>
#include <stdio.h>
#include <string.h>  // memset
#include <malloc.h>
#include <stdbool.h> // true/false/bool
#include <process.h> // exit
#include <time.h>    // clock_t

static unsigned char scrbuf[1024000]={0};

struct s_performance {
	unsigned int size; // buffer size
        unsigned int bandwidth_per_sec; // enough for 486/Pentium architecture with dozen of Mb/s
        unsigned int read_per_sec;
        unsigned int copy_per_sec; // enough for 486/Pentium architecture with dozen of Mb/s
};

void SetVideoMode(unsigned char modeNum) {
        union REGS regs;
	memset(&regs, 0, sizeof(regs));
	regs.x.eax = modeNum; // AH=00: Set mode to AL.
	int386(0x10, &regs, &regs);
}

void QuitError(const char *msg) {
	SetVideoMode(0x3);
	printf(msg);
	exit(1);
}

void DisplayPerf(struct s_performance *perf) {
	int i,maxT;
	printf("buffer size | speed       buffer size | speed\n");

	maxT=11;
	for (i=0;i<11;i++) {
		if (!perf[i].bandwidth_per_sec) {
			maxT=i;
			break;
		}
	}
	for (i=0;i<maxT;i++) {
		printf(" %7dkib | %5dMib/s |",perf[i].size/1000,perf[i].bandwidth_per_sec/1000000); i++;
		printf(" %7dkib | %5dMib/s |",perf[i].size/1000,perf[i].bandwidth_per_sec/1000000); i++;
		if (i<maxT) printf(" %7dkib | %5dMib/s\n",perf[i+1].size/1000,perf[i+1].bandwidth_per_sec/1000000); else printf("\n");
	}
	for (i=0;i<maxT;i++) {
		printf(" %7dkib | %5dMib/s |",perf[i].size/1000,perf[i].read_per_sec/1000000); i++;
		printf(" %7dkib | %5dMib/s |",perf[i].size/1000,perf[i].read_per_sec/1000000); i++;
		if (i<maxT) printf(" %7dkib | %5dMib/s\n",perf[i+1].size/1000,perf[i+1].read_per_sec/1000000); else printf("\n");
	}
	for (i=0;i<maxT;i++) {
		printf(" %7dkib | %5dMib/s |",perf[i].size/1000,perf[i].copy_per_sec/1000000); i++;
		printf(" %7dkib | %5dMib/s |",perf[i].size/1000,perf[i].copy_per_sec/1000000); i++;
		if (i<maxT) printf(" %7dkib | %5dMib/s\n",perf[i+1].size/1000,perf[i+1].copy_per_sec/1000000); else printf("\n");
	}
}

void readMem(unsigned char *ptr, unsigned int size);
#pragma aux readMem = \
			"reloop:"\
			"mov eax,ds:[edi]"\
			"mov eax,ds:[edi+4]"\
			"mov eax,ds:[edi+8]"\
			"mov eax,ds:[edi+12]"\
			"mov eax,ds:[edi+16]"\
			"mov eax,ds:[edi+20]"\
			"mov eax,ds:[edi+24]"\
			"mov eax,ds:[edi+28]"\
			"add edi,32"\
			"dec ecx"\
			"jnz reloop"\
			modify [eax edi ecx] \
			parm [edi] [ecx] 


struct s_performance *BenchW(unsigned char *memory, unsigned int maxiTest) {
	struct s_performance *perf;
	clock_t start_time,end_time,cur_time;
	unsigned int i=0,j;
	unsigned int size;
	unsigned int bandwidth,bmax;
	unsigned int startTest;

	perf=malloc(sizeof(struct s_performance)*11);
        memset(perf,0,sizeof(struct s_performance)*11);
if (maxiTest>100000) {
	printf("testing RAM read\n");
	startTest=1000;
} else {
	startTest=65536;
}
	// initiate any cache
	memset(memory,0,maxiTest); // sinon ça plaaaaaaaaaante!
	memset(memory,0,1000);

	i=0;
	for (size=startTest;size<=maxiTest;size*=2) {
		bmax=0;
		for (j=0;j<3;j++) {
			// test size
			bandwidth=0;
			start_time=clock();
			do {
				cur_time=clock();
			} while (cur_time==start_time);
			start_time=cur_time;
			end_time=start_time+CLOCKS_PER_SEC/10;

			// chrono started, push bytes!
			do {
				memset(memory,cur_time,size);
				bandwidth+=size;
			} while (clock()<end_time);
			if (bandwidth>bmax) bmax=bandwidth;
		}

		perf[i].size=size;
		perf[i].bandwidth_per_sec=bmax*10;
		i++;
	}
if (maxiTest>100000) printf("testing RAM write\n");
	i=0;
	for (size=startTest;size<=maxiTest;size*=2) {
		bmax=0;
		for (j=0;j<3;j++) {
			// test size
			bandwidth=0;
			start_time=clock();
			do {
				cur_time=clock();
			} while (cur_time==start_time);
			start_time=cur_time;
			end_time=start_time+CLOCKS_PER_SEC/10;

			// chrono started, push bytes!
			do {
				readMem(memory,size>>5);
				bandwidth+=size;
			} while (clock()<end_time);
			if (bandwidth>bmax) bmax=bandwidth;
		}

		perf[i].size=size;
		perf[i].read_per_sec=bmax*10;
		i++;
	}
if (maxiTest>100000) printf("testing RAM copy\n");
	i=0;
	for (size=startTest;size<=maxiTest;size*=2) {
		bmax=0;
		for (j=0;j<3;j++) {
			// test size
			bandwidth=0;
			start_time=clock();
			do {
				cur_time=clock();
			} while (cur_time==start_time);
			start_time=cur_time;
			end_time=start_time+CLOCKS_PER_SEC/10;

			// chrono started, push bytes!
			do {
				memcpy(memory,scrbuf,size);
				bandwidth+=size;
			} while (clock()<end_time);
			if (bandwidth>bmax) bmax=bandwidth;
		}

		perf[i].size=size;
		perf[i].copy_per_sec=bmax*10;
		i++;
	}
	return perf;
}

#define TEXT_BUFFER 0xB8000
#define MCGA_BUFFER 0xA0000

void main(int argc, char **argv) {
	unsigned char *memory;
	unsigned char *screen;
	char c;
	struct s_performance *perfRAM,*perfTXT,*perfVGA;

	SetVideoMode(0x3);
	SetVideoMode(0x12);
	printf("CLOCKS_PER_SEC=%d\n",CLOCKS_PER_SEC);

	memory=malloc(1024000); // 1M
	if (!memory) QuitError("Cannot alloc 1M RAM\n");

	perfRAM=BenchW(memory,1024000);

	SetVideoMode(0x3);
	free(memory);
	memory=(unsigned char *)0xB8000;
	perfTXT=BenchW(memory,65536);

	SetVideoMode(0x13);
	memory=(unsigned char *)0xA0000;
	perfVGA=BenchW(memory,65536);

	SetVideoMode(0x3); SetVideoMode(0x12); printf("Memory results (Write/Read/Copy RAM->RAM)\n");
	DisplayPerf(perfRAM);
	printf("TextMode memory results (Write/Read/Copy RAM->Video)\n");
	DisplayPerf(perfTXT);
	printf("VGA memory results (Write/Read/Copy RAM->Video)\n");
	DisplayPerf(perfVGA);

	free(perfRAM); free(perfTXT); free(perfVGA);
}

