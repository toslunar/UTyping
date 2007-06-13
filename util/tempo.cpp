#include<stdio.h>
#include<time.h>

clock_t start;

void setTime(){
	start = clock();
}

double getTime(){
	return (double)(clock() - start)/CLOCKS_PER_SEC;
}

int main(void){
START:
	getchar();
	setTime();
	int n = 1;
	double a = 0.0;	/* t[0]*(-n+1) + t[1]*(-n+3) + ... + t[n-1]*(n-1) */
	double b = 0.0;	/* t[0] + t[1] + ... + t[n-1] */
	while(1){
		char ch = getchar();
		if(ch == 'q'){
			goto START;
		}
		if(ch != '\n'){
			continue;
		}
		double t = getTime();
		a -= b;
		a += t * n;
		b += t;
		n++;
		double dt;
		dt = a / ((n-1)*n*(n+1)/6);
		printf("%f\n",60.0/dt);
	}
	return 0;
}
