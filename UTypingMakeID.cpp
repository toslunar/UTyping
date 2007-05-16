#include <stdio.h>
#include "utuid.h"

int main(){
	FILE *fp;
	fp = fopen("UTyping_ID.txt", "w");
	if(!fp){
		throw __LINE__;
	}
	int ID[4];
	getUTypingUserID(ID);
	for(int i=0; i<4; i++, fprintf(fp, i==4 ? "\n" : " ")){
		fprintf(fp, "%04X", ID[i]);
	}
	fclose(fp);
	return 0;
}
