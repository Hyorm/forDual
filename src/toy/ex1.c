#include <stdio.h>
#include <crown.h>

int main(int argc, char *argv[]) {

	int a, b;

	SYM_int(a);
	SYM_int(b);	
	b = b+3*a+2;

	if(b==11){
		printf("b is 11\n");
	}else if(b==40){
		printf("b is 40\n");
	}else{
		printf("b is not 11\n");
	}

	return 0;
}
