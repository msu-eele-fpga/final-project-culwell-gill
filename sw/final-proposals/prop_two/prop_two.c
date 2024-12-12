#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#define PERIOD_OFFSET 0x0
#define RED_DUTY_CYCLE_OFFSET 0x4
#define GREEN_DUTY_CYCLE_OFFSET 0x8
#define BLUE_DUTY_CYCLE_OFFSET 0xc
#define CHANEL_ZERO_OFFSET 0x0
#define CHANEL_ONE_OFFSET 0x4
#define CHANEL_TWO_OFFSET 0x8

int main () {
	FILE *file;

	uint32_t val;

	file = fopen("/dev/prop_two_controller" , "rb+" );

	if (file == NULL) {
		printf("failed to open file\n");
		exit(1);
	}
	//Set the period to something

	while(1){
//RED
		//Set ADC file to chanel 0 (offset 0) 
		fseek(file, 0x4, SEEK_SET);

		//Check to see if the button is pressed
		fread(&val, 4, 1, file);
		fflush(file);
		usleep(100);

		if(!val){ //Button has been pressed
			uint32_t to_write = 0; //Make something we can use the pointer to

			fseek(file, 0x8, SEEK_SET);
			fread(&val, 4, 1, file);
			fflush(file);
			printf("%d\n", val);

			for(int i = 0; i < val; i++){
				to_write = 1;

				fseek(file, 0x0, SEEK_SET);
				fwrite(&to_write, 4, 1, file);
				fflush(file);

				printf("%d\n", to_write);

				usleep(500000); //Half a second between blinks

				to_write = 0;

				fseek(file, 0x0, SEEK_SET);
				fwrite(&to_write, 4, 1, file);
				fflush(file);

				printf("%d\n", to_write);

				usleep(500000);
			}
		}
	}
	fclose(file);

	return 0;
}
