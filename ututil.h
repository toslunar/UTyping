#ifndef __UTYPING_UTILITY
#define __UTYPING_UTILITY

/* 雑多な関数 */

bool readInt(int &n, FILE *fp);
void writeInt(int n, FILE *fp);
bool readChars(char *str, int n, FILE *fp);
void writeChars(const char *str, int n, FILE *fp);

void getDirFromPath(char *directoryName, const char *fileName);

//char *fgetline(char buf[], FILE *fp);

void getOrdinal(char *buf, int n);
/* 序数を取得 */

int getDateInt();
/* 日付を整数で保存 */
void getDateStrFromInt(char *buf, int date);
/* 整数で保存した日付を文字列にする */

void getDateStr(char *buf);
/* 日付と時刻をbufに格納 */

void outputError(char *str);
/* エラー出力 */

bool readInt(int &n, FILE *fp){
	int b[4];
	for(int i=0; i<4; i++){
		b[i] = getc(fp);
		if(b[i] == EOF){
			return false;
		}
	}
	n = b[0] | b[1]<<8 | b[2]<<16 | b[3]<<24;
	return true;
}

void writeInt(int n, FILE *fp){
	int b[4];
	for(int i=0; i<4; i++){
		b[i] = n >> (8*i);
	}
	for(int i=0; i<4; i++){
		putc(b[i], fp);
	}
	return;
}

bool readChars(char *str, int n, FILE *fp){
	int b[16];
	for(int i=0; i<n; i++){
		b[i] = getc(fp);
		if(b[i] == EOF){
			return false;
		}
	}
	for(int i=0; i<n; i++){
		str[i] = b[i];
	}
	str[n] = '\0';
	return true;
}

void writeChars(const char *str, int n, FILE *fp){
	int b[16];
	for(int i=0; i<n; i++){
		b[i] = str[i];
	}
	for(int i=1; i<n; i++){
		if(b[i-1] == '\0'){
			b[i] = '\0';
		}
	}
	for(int i=0; i<n; i++){
		putc(b[i], fp);
	}
	return;
}

void getDirFromPath(char *directoryName, const char *fileName){
	strcpy(directoryName, fileName);
	for(int i = strlen(directoryName) - 1; i >= 0; i--){	/* 後ろからたどる */
		if(directoryName[i] == '/' || directoryName[i] == '\\'){	/* '/'や'\'が現れたら */
			directoryName[i + 1] = '\0';	/* そこまでなので、次を'\0'に */
			return;
		}
	}
	/* 最後まで現れなかったら */
	directoryName[0] = '\0';
	return;
}

#define fgetline(buf, fp) fgetline_1(buf, sizeof(buf), fp)
/* fpから1行読む。ただし、最後の'\n'は削除 */

char *fgetline_1(char *buf, size_t size, FILE *fp){
	if(fgets(buf, size, fp) == NULL){
		return NULL;
	}
	int len = strlen(buf) - 1;
	if(buf[len] == '\n'){
		buf[len] = '\0';
	}
	return buf; 
}

void getOrdinal(char *buf, int n){
	if((n / 10) % 10 == 1){	/* 下二桁11,12,13(10-19すべて)はth */
		sprintf(buf, "%2dth", n);
	}else{
		switch(n % 10){	/* 1の位で判定 */
		case 1:
			sprintf(buf, "%2dst", n);
			break;
		case 2:
			sprintf(buf, "%2dnd", n);
			break;
		case 3:
			sprintf(buf, "%2drd", n);
			break;
		default:
			sprintf(buf, "%2dth", n);
			break;
		}
	}
}

int getDateInt(){
	DATEDATA date;
	GetDateTime(&date);
	return (date.Year << 16) | (date.Mon << 8) | date.Day;
}

void getDateStrFromInt(char *buf, int date){
	if(date > 0){
		sprintf(buf, "%4d/%2d/%2d", (date>>16)&0xffff, (date>>8)&0xff, date&0xff);
	}else{
		strcpy(buf,"----/--/--");
	}
}

void getDateStr(char *buf){
	DATEDATA date;
	GetDateTime(&date);
	sprintf(buf, "%d/%02d/%02d %02d:%02d:%02d", date.Year, date.Mon,
		date.Day, date.Hour, date.Min, date.Sec);
}

void outputError(char *str){
	char buf[256];
	getDateStr(buf);
	FILE *fp;
	fp = fopen("エラーログ.txt","a");
	fprintf(fp, "[%s] %s\n", buf, str);
	fclose(fp);
}

/* ------------------------------------------------------------ */
#if 0
HANDLE newThread(LPTHREAD_START_ROUTINE lpStartAddress,LPVOID lpParameter);
void deleteThread(HANDLE handle);

//#ifdef UTYPING_USE_MULTI_THREAD

/* DWORD WINAPI (*func)(LPVOID)を新しいスレッドとして作る */
HANDLE newThread(LPTHREAD_START_ROUTINE lpStartAddress,LPVOID lpParameter){
	DWORD dwID;
	return CreateThread(NULL,0,lpStartAddress,lpParameter,0,&dwID);
}

void deleteThread(HANDLE handle){
	WaitForSingleObject(handle, INFINITE);	/* 3000ぐらいにしておいた方がいいかもしれない */
	/*
	if(!TerminateThread(handle, 0)){
		throw __LINE__;
	}
	*/
	CloseHandle(handle);
}

//#endif
#endif


#endif
