/*
                  argv[1]      argv[2] argv[3]              argv[4]
 >fumenrevise.exe hanbetsu.txt lor.txt revised_hanbetsu.txt 0.05

  argv[1] : もとの譜面のファイル名
  argv[2] : list of replays を記したファイル名
  argv[3] : 補正された譜面をこの名前で保存
  argv[4] : ``epsilon'' : 誤差限界
            時間差 epsilon 以上の音符しか見つからない場合はミスとみなす

  argv[2] の各行にはリプレイのファイル名を書く．1 行に 1 個．最大 10 個．



  リプレイはデバッグモードで作ったものを使用することを強く推奨します．
  （譜面の各音符に対しリプレイ中の 1 つの音符を対応させたいので）
  （ちなみにこのプログラムはリプレイの文字情報は読んでません）


  「小節線」「拍線」「打ち切り点」は
  次に「音符」が現れたときに同じ時刻でまとめて出力します．
  したがって，「音符」のない時刻の「小節線」「拍線」「打ち切り点」はうまく出力されません．
      最後の音符以降の「小節線」「拍線」「打ち切り点」は出力されません．
  これらは手作業で書き込む必要があります．

  argv[1] に入れるものは，実際の譜面ではなく
  その曲の小節線・拍線を音符にしたものにしておくと
  あとあと楽かもしれません．


-- 
mos

*/

#include<stdio.h>
#include<math.h>
#include<string.h>



int main(int args, char* argv[]){
    if(args < 4+1) return 1;

    FILE * pfread, * pflor, * pfreplay[30], * pfwrite;
    double d;
    char c;
//    char *str;
    int nreplay = 0;	// number of available replays
    bool waiting_slash, waiting_star, waiting_minus, waiting_equal;
    char buf[256];	// temporary
    char kanjibuf[256];	// to store KanjiLyrics 
    double data[30][1000];
    int datalength[30];
    int iter[30];

    double epsilon;

    if((pfread = fopen (argv[1], "r")) == NULL)
        return 1;
    if((pflor   = fopen (argv[2], "r")) == NULL)
        return 1;
    if((pfwrite = fopen (argv[3], "w")) == NULL)
        return 1;
    sscanf(argv[4], "%lf", &epsilon);

    // argv[2] に載っているリプレイファイルを開く
    for(int i=0; i<30; i++){
        if(fgets(buf, sizeof(buf), pflor) == NULL){
           fprintf(stdout, "%d replays found, no more replays.\r\n", i);
           break;
        }
        for(int k=0; k<256; k++){
            if(buf[k]=='\n' || buf[k]=='\r' || buf[k]=='\0'){
                buf[k] = '\0';
                break;
            }
        }

        if((pfreplay[i] = fopen(buf, "rb")) == NULL){
            printf("replay #%d unavailable\r\n", i);
            break;
        }
        nreplay++;
    }
    fclose(pflor);


    // リプレイを開いて時刻データを格納（文字データは無視）
    for(int i=0; i<nreplay; i++){
        fprintf(stdout, "[replay file %2d of %2d] ", i+1, nreplay);

        int j;
        data[i][0] = -90;
        for(j=1; j<1000; j++){
            if((c = getc(pfreplay[i])) == EOF){
                fprintf(stdout, "eof\n");
                break;
            }
            if((fread(&d, sizeof(d), 1, pfreplay[i])) < 1){
                fprintf(stderr, "replay format error at #%d: <%c %f>\r\n", j, c, d);
                break;
            }      
      
            data[i][j] = d;
        }
        datalength[i] = j;
        iter[i] = 0;
    }

    // 譜面ファイルの各行について……
    waiting_equal = waiting_minus = waiting_slash = waiting_star = false;
  while(fgets(buf, sizeof(buf), pfread) != NULL){
    switch(buf[0]){
        case '@': // 音声ファイル指定なのでそのまま通す
            fprintf(pfwrite, "%s", buf);
            break;

        case '=': // これらは次の '+' でまとめて処理
            waiting_equal = true;
            break;
        case '-': 
            waiting_minus = true;
            break;
        case '/': 
            waiting_slash = true;
            break;
        case '*': // KanjiLyrics は記憶しておく
            waiting_star  = true;
            sscanf(buf+1, "%lf %s", &d, kanjibuf);
            break;

        case '+': // 音符の場合
            if(sscanf(buf+1, "%lf %s", &d, buf) == EOF){
                fprintf(stdout, "error at %lf %s\n", d, buf);
                break;
            }

            int howmanyhas = 0;
            double moveby = 0;
            double d1, d2;
            double d_new;

            for(int i=0; i<nreplay; i++){
                while(data[i][iter[i]] < d && iter[i] < datalength[i])
                    iter[i]++;
                	// so that [iter[i]] <= d < [iter[i+1]] 

                d1 = data[i][iter[i]] - d;
                d2 = data[i][iter[i+1]] - d;

                if(fabs(d1) < epsilon || fabs(d2) < epsilon){
                    howmanyhas++;
                    if(fabs(d1) < fabs(d2))
                        moveby += d1;
                    else
                        moveby += d2;
                }
            }
            if(howmanyhas > 0)
                d_new = d + moveby/howmanyhas;
            else 
                d_new = d;
                                    
            // '=' '-' '*' '/' をまとめて処理
            if(waiting_equal)
                fprintf(pfwrite, "=%lf\r\n", d_new);
            if(waiting_minus)
                fprintf(pfwrite, "-%lf\r\n", d_new);
            if(waiting_slash)
                fprintf(pfwrite, "/%lf\r\n", d_new);
            if(waiting_star )
                fprintf(pfwrite, "*%lf %s\r\n", d_new, kanjibuf);

            waiting_equal = waiting_minus = waiting_slash = waiting_star = false;

            fprintf(pfwrite, "+%lf %s\r\n", d_new, buf);
    }


  }
    

    for(int i=0; i<nreplay; i++)
        fclose(pfreplay[i]);
    fclose(pfread);
    fclose(pfwrite);

    return 0;
}

