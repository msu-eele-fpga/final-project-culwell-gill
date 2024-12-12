# prop_two.c

## About
The prop_two.c file is a C file that provides the core functionality of the proposal two program: the blinking count LED.  


## Function 
prop_two.c reads and writes to the prop_two registers via the kernel, driver, and device tree structures. Using this data, the file blinks an LED according to the value set by the switches on the DE-10 Nano (4 bit nibble).

## Building, Installing, and Running 
- The file needs to be cross-compiled in linux for the arm processors using "/usr/bin/arm-linux -gnueabihf -gcc -o prop_two prop_two.c"
- Copy the program to the tftp server at "/srv/nfs/de10nano/ubuntu-rootfs/home/soc"
- Using Putty, log into the root folder of the FPGA and run the program with "./prop_two" 
- The does not use user inputs and will run automatically



## File Open

```c
int main () {
	FILE *file;

	uint32_t val;

	file = fopen("/dev/prop_two_controller" , "rb+" );

	if (file == NULL) {
		printf("failed to open file\n");
		exit(1);
	}
```
The appropriate libraries are included and the device driver file is opened. The program verifies that the driver file exists. 

## Button Check

```c

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
```
The status of the button is continuously checked in a while loop. Should the button be pressed, the program then looks at the values of the switches in the switch register and begins an i-loop. The i-loop performs the blinking via the registers and HDL. The usleep command provides a delay in microseconds.

Should the program exit the while loop, the files are closed. 