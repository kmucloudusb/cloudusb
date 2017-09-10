#include <stdio.h>

int main(){
	FILE *wfile;
	wfile = fopen("/home/pi/capstone/cloudeusb/gadget/init_file.txt","w");
	fprintf(wfile, "hello!!");
	close(wfile);
	return 0;
}
