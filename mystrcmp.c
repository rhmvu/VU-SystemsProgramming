#include <stdio.h>

int mystrcmp(char *strptr1, char *strptr2) {
	char *string1 = strptr1;
	char *string2 = strptr2;
	/* This function returns the length of the input string */
	int identical = 1;
	while (identical == 1){
		if(*string1 != *string2){
			identical = 0;
		}
		if(*string1 == '\0' && *string2 == '\0'){
			return identical;
		}
		*string1++;
		*string2++;
	}
	return identical;
}

int main(int argc, char **argv) {
	int identical;
	if (argc!=3) {
		printf("Usage: mystrcmp <string1> <string2>\n\n");
		return 1;
	}
	identical = mystrcmp(argv[1],argv[2]);
	if(identical){
		printf("The two strings are identical.\n");
	}else{
		printf("The two strings are different.\n");
	}
	return 0;
}
