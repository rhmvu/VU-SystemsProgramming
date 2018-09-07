#include <stdio.h>

int mystrlen(char *input_string) {
	/* This function returns the length of the input string */
	int length = 0;
	char *char_pointer = input_string;

	for (char_pointer; *char_pointer!='\0';char_pointer++){
		length++;
	}
	return length;
}

int main(int argc, char **argv) {
	int length;
	if (argc!=2) {
		printf("Usage: strlen <input_string_with_no_space_inside_it>\n\n");
		return 1;
	}
	length = mystrlen(argv[1]);
	printf("The length is: %d characters.\n",length);
	return 0;
}