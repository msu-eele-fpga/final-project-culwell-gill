# final_project.c

## About
The final_project.c file is a C file that provides instructions to the FPGA to drive the core functionality of the RGB_LEDS. The file communicates to the rgb_led HDL via the linux kernel, utilizing the device driver and device tree to index and communicate with the avalon buss wrapper registers. The file passes through the output registers of the adc to the input registers of the rgb_controller. 


## Function 
The file reads from the adc controller driver and writes to the rgb_led_controller driver via the registers

## File Open

```c
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
```

First, the offsets of the registers are defined, the offests are used later when the file does read and write operations. The file then opens the driver files while verifying their existence. The period is written into the controller.

## File Read & Write

```c
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
```
In the while loop, the file repeats the same process for all three LED registers, Red is the only example shown. The file reads the ADC value from the registers after applying the appropriate register offset to seek the correct data, it then "flushes" the value in that register to the system- this tells the code to write the value to the OS before continuing the code. The HDL in the rgb_controller interprets the register data and then instantiates the changes in the LEDs according to the register data.    


## File Close

```c

	fclose(rgb_controller_file);
	fclose(adc_file);

	return 0;
}
```

Should the program exit the while loop, the files are closed. 