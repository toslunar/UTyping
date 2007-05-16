#include <stdio.h>
#include <string.h>
#include <math.h>
#include <vector>
#include <string>

using namespace std;

struct Info{
	double speed;	/* 1小節にかかる時間（正確には、四分音符4つにかかる時間） */
	int base;	/* 今何分音符単位で入力しているか */
	double time;
	int beat_nu, beat_de;	/* 一小節の分子/分母 */
	int beat_int;	/* 現在の拍数、端数 */
	double beat_frac;
	/* ただし、端数は(-1,0]に取る。例：2.25拍目（16分音符5個）は2,-0.75 */
};

void setSpeed(Info &info, char *str){
	int k;
	double t;
	/* 整数=実数 を読む */
	char *ptr = strtok(str, "= \t\n");
	if(ptr == NULL){
		throw __LINE__;
	}
	int n = sscanf(ptr, "%d", &k);
	if(n < 1){
		throw __LINE__;
	}
	ptr = strtok(NULL, " \t\n");
	if(ptr == NULL){
		throw __LINE__;
	}
	n = sscanf(ptr, "%lf", &t);
	if(n < 1){
		throw __LINE__;
	}
	if(k <= 0){	/* k分音符のkは正整数 */
		throw __LINE__;
	}
	if(t <= 0.0){	/* テンポが正でないとかおかしい */
		throw __LINE__;
	}
	info.speed =  60.0 / (t / k);
	return;
}

void setBase(Info &info, char *str){
	int k;
	int n = sscanf(str, "%d", &k);
	if(n < 1){
		throw __LINE__;
	}
	if(k <= 0){	/* k分音符のkは正整数 */
		throw __LINE__;
	}
	info.base = k;
	return;
}

void setBeat(Info &info, char *str){
	int nu, de;
	/* 整数/整数 を読む */
	char *ptr = strtok(str, "/ \t\n");
	if(ptr == NULL){
		throw __LINE__;
	}
	int n = sscanf(ptr, "%d", &nu);
	if(n < 1){
		throw __LINE__;
	}
	if(nu <= 0){	/*  0以下で拍子解除 */
		info.beat_nu = 0;
		info.beat_de = 0;
		info.beat_int = 0;
		info.beat_frac = 0.0;
		return;
	}
	ptr = strtok(NULL, ": \t\n");
	if(ptr == NULL){
		throw __LINE__;
	}
	n = sscanf(ptr, "%d", &de);
	if(n < 1){
		throw __LINE__;
	}
	if(de <= 0){	/* 分母は正整数 */
		throw __LINE__;
	}
	
	ptr = strtok(NULL, " \t\n");
	if(ptr == NULL){
		info.beat_int = 0;	/* 位置を設定しない場合、0拍目になる */
		info.beat_frac = 0.0;
	}else{
		double d;
		n = sscanf(ptr, "%lf", &d);
		if(n < 1){
			throw __LINE__;
		}
		info.beat_int = ceil(d);
		info.beat_frac = d - info.beat_int;
	}
	info.beat_nu = nu;
	info.beat_de = de;
	
	info.beat_int %= info.beat_nu;
	return;
}

void timeAdd(Info &info, vector<pair<double, int> > &timeArray){
	if(info.speed <= 0.0){
		throw __LINE__;
	}
	if(info.base <= 0){
		throw __LINE__;
	}
	double length = 1.0 / info.base;
	double dTime, dBeat;
	dTime = length * info.speed;
	if(info.beat_de > 0){	/* 拍子が定義されている */
		dBeat = length * info.beat_de;
		while(info.beat_frac + dBeat > -0.001){	/* 誤差が一応気になる。0.999拍以降にテンポが変わることはまずないだろう。 */
			timeArray.push_back(make_pair(
				info.time - info.speed * (info.beat_frac / info.beat_de),	/* info.time から足りない分増やす（beat_fracは負） */
				(info.beat_int == 0) ? -1 : -2));
			
			info.time += info.speed / info.beat_de;	/* 一拍分時間を進める */
			//length -= 1.0 / info.beat_de;
			dTime -= info.speed / info.beat_de;
			
			info.beat_int++;
			if(info.beat_int == info.beat_nu){
				info.beat_int = 0;
			}
			dBeat -= 1.0;
		}
		info.beat_frac += dBeat;
	}
	info.time += dTime;
	return;
}

void processCommand(char *str, Info &info){
	switch(str[0]){
	case 't':	/* t4=120 : 四分音符が一分に120個 */
		setSpeed(info, str + 1);
		break;
	case 'b':	/* 入力するn分音符のnを設定 */
	case 'l':	/* 入力するn分音符のnを設定 */
		setBase(info, str + 1);
		break;
	case 'B':	/* 拍子を設定（例：B4/4）、B0で解除 */
		setBeat(info, str + 1);
		break;
	case 'r':	/* r0.3で0.3秒進める */
		{
		double t;
		int n = sscanf(str + 1, "%lf", &t);
		if(n < 1){
			throw __LINE__;
		}
		info.time += t;
		}
		break;
	default:
		throw __LINE__;
	}
}

int main1(char *fileName){
	char directoryName[256];
	strcpy(directoryName, fileName);
	for(int i = strlen(directoryName) - 1; i >= 0; i--){	/* 後ろからたどる */
		if(directoryName[i] == '/' || directoryName[i] == '\\'){	/* '/'や'\'が現れたら */
			directoryName[i + 1] = '\0';	/* そこまでなので、次を'\0'に */
			goto L2;
		}
	}
	/* 最後まで現れなかったら */
	directoryName[0] = '\0';
L2:
	
	FILE *fp;
	fp = fopen(fileName, "r");
	if(!fp){
		throw __LINE__;
	}
	
	Info info;
	info.speed = -1.0;	/* 1小節にかかる時間 */
	info.base = 0;	/* 今何分音符単位で入力しているか */
	info.time = 0.0;
	
	info.beat_nu = 0;
	info.beat_de = 0;
	
	info.beat_int = 0;	/* 最初は0拍目とする */
	info.beat_frac = 0.0;
	
	vector<pair<double, int> > timeArray;
		/* intは、0:区切り, 1:歌詞の句の最初以外, 2:歌詞の句の最初 */
		/* -1:小節線, -2:「拍線」 */
	vector<string> lyricsArray;	/* 打つ歌詞（読み方） */
	vector<string> lyricsKanjiArray;	/* 流れる文字と別に表示する歌詞 */
	
	char buf[1024];
	
	if(fgets(buf, sizeof(buf), fp) == NULL){
		throw __LINE__;
	}
	int len = strlen(buf);
	if(buf[len-1] == '\n'){
		buf[len-1] = '\0';
	}
	char outFileName[256];
	sprintf(outFileName, "%s%s", directoryName, buf);	/* 1行目で出力ファイルを指定 */
	//sprintf(outFileName, "%s%s-debug.txt", directoryName, buf);	/* 1行目で出力ファイルを指定 */
	printf("%sに出力します。\n", outFileName);
	freopen(outFileName, "w", stdout);
	
	if(fgets(buf, sizeof(buf), fp) == NULL){
		throw __LINE__;
	}
	printf("@%s", buf);	/* 2行目は曲ファイルを書くのでそのまま出力 */
	
	while(fgets(buf, sizeof(buf), fp) != NULL){
		switch(buf[0]){
		case 'e':
			goto L1;	/* 以降の行を無視 */
		case '#':	/* コメント行 */
			break;
		case '@':	/* 1行命令 */
			processCommand(buf + 1, info);
			break;
		case '\'':	/* 打つ歌詞（半角スペース、タブ区切り） */
			for(char *ptr = strtok(buf + 1, " \t\n"); ptr != NULL;
					ptr = strtok(NULL, " \t\n")){
				lyricsArray.push_back(string(ptr));
			}
			break;
		case '\"':	/* 表示する歌詞（タブ区切り）（てか、普通区切らない） */
			for(char *ptr = strtok(buf + 1, "\t\n"); ptr != NULL;
					ptr = strtok(NULL, "\t\n")){
				lyricsKanjiArray.push_back(string(ptr));
			}
			break;
		default:
			for(char *ptr = buf; *ptr != '\0'; ptr++){
				switch(*ptr){
				case '[':	/* コマンド開始 */
					{
						ptr++;
						char *ptrCmd = ptr;	/* コマンドの開始位置 */
						while(*ptr != ']'){
							if(*ptr == '\0'){	/* コマンドが閉じられなかった */
								throw __LINE__;
							}
							ptr++;
						}
						*ptr = '\0';	/* ']'を'\0'に書き換える */
						processCommand(ptrCmd, info);
					}
					break;
				case ']':	/* コマンドを開いていないのに閉じるが来た */
					throw __LINE__;
				case '*':	/* 歌詞の1音節目 */
					/* 歌詞の区切りを入れて、歌詞を入れて、時間を経過させる */
					timeArray.push_back(make_pair(info.time, 0));
					timeArray.push_back(make_pair(info.time, 2));
					timeAdd(info, timeArray);
					break;
				case '+':	/* 歌詞の2音節目以降 */
					/* 歌詞を入れて、時間を経過させる */
					timeArray.push_back(make_pair(info.time, 1));
					timeAdd(info, timeArray);
					break;
				case '-':	/* 何もないところ */
					/* 時間を経過させる */
					timeAdd(info, timeArray);
					break;
				case '/':	/* 次の歌詞の前に打ち切られる場合 */
					/* 歌詞の区切れを入れて、時間を経過させる */
					timeArray.push_back(make_pair(info.time, 0));
					timeAdd(info, timeArray);
					break;
				}
			}
			break;
		}
	}
L1:
	timeArray.push_back(make_pair(info.time, 0));	/* 終了後、区切りを入れておく */
	
	/* ここで、任意の表示する歌詞対して、区切りが後ろに存在し、それが表示する歌詞の終わりとなる。 */
	
	vector<string>::iterator itrLyrics = lyricsArray.begin();
	vector<string>::iterator itrKanjiLyrics = lyricsKanjiArray.begin();
	bool flag = false;	/* 直前が区切りだったかのフラグ */
	for(vector<pair<double, int> >::iterator itr = timeArray.begin();
			itr != timeArray.end(); itr++){
		if((*itr).second <= -1){	/* 小節線などが入る時間 */
			if((*itr).second == -1){	/* 小節線 */
				printf("=");
			}else{	/* 拍線 */
				printf("-");
			}
			printf("%lf\n", (*itr).first);
		}else if((*itr).second >= 1){	/* 歌詞の入る時間 */
			if((*itr).second == 2){	/* 表示する歌詞の入る時間 */
				if(itrKanjiLyrics == lyricsKanjiArray.end()){
					throw __LINE__;
				}
				printf("*%lf %s\n", (*itr).first, (*itrKanjiLyrics).c_str());
				itrKanjiLyrics++;
			}
			if(itrLyrics == lyricsArray.end()){	/* 打つ歌詞が入るべきところに歌詞がない */
				throw __LINE__;
			}
			printf("+%lf %s\n", (*itr).first, (*itrLyrics).c_str());
			itrLyrics++;
			
			flag = false;
		}else{	/* 歌詞の区切りの入る時間 */
			if(!flag){	/* 区切りが連続することを防止 */
				printf("/%lf\n", (*itr).first);
			}
			flag = true;
		}
//printf("%lf %d\n",(*itr).first,(int)(*itr).second);
	}
	return 0;
}

int main(int argc, char **argv){
	if(argc < 2){
		return 0;
	}
	try{
		main1(argv[1]);
	}catch(int n){
		fprintf(stderr, "Error %d\n",n);
		getchar();
	}
	return 0;
}
