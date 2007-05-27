#ifndef __UTYPING_USERID
#define __UTYPING_USERID

#include <windows.h>

void UTypingUserIDHash(int *ID, unsigned char *buf, int len){
	const int ci[4]={3141, 5926, 5358, 9793};
	for(int i=0; i<4; i++){
		ID[i] = 0;
	}
	for(int i=0; i<len; i++){
		ID[0] ^= buf[i] * (2*i+1) * ci[0];
		ID[0] &= 0xffff;
		for(int j=1; j<4; j++){
			ID[j] ^= ID[j-1] * ci[j];
			ID[j] &= 0xffff;
		}
	}
}

bool getUTypingUserID(int *ID){
	for(int i=0; i<4; i++){
		ID[i] = 0;
	}
	unsigned char buf[MAX_COMPUTERNAME_LENGTH + 1];
	DWORD len = MAX_COMPUTERNAME_LENGTH + 1;
	for(int i=0; i<(int)len; i++){
		buf[i] = 0;
	}
	if(!GetComputerName(buf, &len)){
//printf("Ž¸”s\n");
		return false;
	}
	
//printf("%s\n",buf);
	UTypingUserIDHash(ID, buf, MAX_COMPUTERNAME_LENGTH + 1);
	return true;
}

#endif
