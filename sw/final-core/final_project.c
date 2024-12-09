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
	FILE *rgb_controller_file;
	FILE *adc_file;

	uint32_t val;

	rgb_controller_file = fopen("/dev/rgb_controller" , "rb+" );
	adc_file = fopen("/dev/adc", "rb+");

	if (rgb_controller_file == NULL || adc_file == NULL) {
		printf("failed to open file(s)\n");
		exit(1);
	}
	//Set the period to something
	val = 0x400;
	fseek(rgb_controller_file, 0, SEEK_SET);
	fwrite(&val, 4, 1, rgb_controller_file);

	while(1){
//RED
		//Set ADC file to chanel 0 (offset 0) 
		fseek(adc_file, CHANEL_ZERO_OFFSET, SEEK_SET);

		//Read chanel 0 into val
		fread(&val, 4, 1, adc_file);
		fflush(adc_file);
		usleep(100);
		val /= 2;

		//set rgb controller file to red location	
    		fseek(rgb_controller_file, RED_DUTY_CYCLE_OFFSET, SEEK_SET);
	
		//Write value into red
		fwrite(&val, 4, 1, rgb_controller_file);
		fflush(rgb_controller_file);
//GREEN
		//Set ADC file to chanel 0 (offset 0) 
		fseek(adc_file, CHANEL_ONE_OFFSET, SEEK_SET);

		//Read chanel 0 into val
		fread(&val, 4, 1, adc_file);
		fflush(adc_file);
		usleep(100);
		val /= 2;

		//set rgb controller file to red location	
    		fseek(rgb_controller_file, GREEN_DUTY_CYCLE_OFFSET, SEEK_SET);
	
		//Write value into red
		fwrite(&val, 4, 1, rgb_controller_file);
		fflush(rgb_controller_file);
//BLUE
		//Set ADC file to chanel 0 (offset 0) 
		fseek(adc_file, CHANEL_TWO_OFFSET, SEEK_SET);

		//Read chanel 0 into val
		fread(&val, 4, 1, adc_file);
		fflush(adc_file);
		usleep(100);
		val /= 2;

		//set rgb controller file to red location	
    		fseek(rgb_controller_file, BLUE_DUTY_CYCLE_OFFSET, SEEK_SET);
	
		//Write value into red
		fwrite(&val, 4, 1, rgb_controller_file);
		fflush(rgb_controller_file);
	}

	fclose(rgb_controller_file);
	fclose(adc_file);

	return 0;
}
