# prop_one.c

## About
The prop_one.c file is a C file that provides the core functionality of the proposal one program: the running LED.  


## Function 
prop_one.c reads and writes to the prop_one registers via the kernel, driver, and device tree structures. Using this data, the file makes a running LED that changes directions depending on which button is pressed.

## Building, Installing, and Running 
- The file needs to be cross-compiled in linux for the arm processors using "/usr/bin/arm-linux -gnueabihf -gcc -o prop_one prop_one.c"
- Copy the program to the tftp server at "/srv/nfs/de10nano/ubuntu-rootfs/home/soc"
- Using Putty, log into the root folder of the FPGA and run the program with "./prop_one" after installing the device driver 
- The does not use user inputs and will run automatically

## File Open

```c
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
```
The appropriate libraries are included and the device driver file is opened. The program verifies that the driver file exists. 

## Button Check

```c
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

```
The program continuously checks the status of the buttons in a while loop. Should one of the buttons be pressed, the direction bit is changed to a one or a zero depending on the most recent button press.   


## LED Register write

```c
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
```
After determining the direction, the output vector is modified with a bit-shift left or right. Output is then pointed to the register for a file write, the HDL then instantiates an LED accordingly. 

Should the program exit the while loop, the files are closed. 