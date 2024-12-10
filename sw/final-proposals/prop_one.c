#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

int main () {
	FILE *file;

	uint32_t val;
	uint32_t direction = 0;
	uint32_t output = 1;

	file = fopen("/dev/prop_one_controller" , "rb+" );

	if (file == NULL) {
		printf("failed to open file\n");
		exit(1);
	}
	//Set the period to something

	while(1){
		//Move to button 1 in the file 
		fseek(file, 0x0, SEEK_SET);

		//Check to see if the button_1 is pressed
		fread(&val, 4, 1, file);
		fflush(file);
		usleep(100);

		printf("Button 1: %d\n", val); 

		if(!val) direction = 1;

		//Check to see if the button_2 is pressed
		fseek(file, 0x4, SEEK_SET);
		fread(&val, 4, 1, file);
		fflush(file);
		usleep(100);

		printf("Button 2: %d\n", val);

		if(!val) direction = 0;

		output = direction == 1 ? output * 2 : output / 2;

		if(output >= 128) output = 1;
	       	else if(output == 0) output = 64;

		fseek(file, 0x8, SEEK_SET);
		fwrite(&output, 4, 1, file);
		fflush(file);

		printf("output: %d\n", output);

		usleep(250000);
	}
	fclose(file);

	return 0;
}
