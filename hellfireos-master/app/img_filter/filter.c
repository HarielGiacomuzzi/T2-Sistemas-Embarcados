#include <hellfire.h>
#include <noc.h>
//#include <string.h>
#include "image.h"

uint8_t gausian(uint8_t buffer[5][5]){
	int32_t sum = 0, mpixel;
	uint8_t i, j;

	int16_t kernel[5][5] =	{	{2, 4, 5, 4, 2},
					{4, 9, 12, 9, 4},
					{5, 12, 15, 12, 5},
					{4, 9, 12, 9, 4},
					{2, 4, 5, 4, 2}
				};
	for (i = 0; i < 5; i++)
		for (j = 0; j < 5; j++)
			sum += ((int32_t)buffer[i][j] * (int32_t)kernel[i][j]);
	mpixel = (int32_t)(sum / 159);

	return (uint8_t)mpixel;
}

uint32_t isqrt(uint32_t a){
	uint32_t i, rem = 0, root = 0, divisor = 0;

	for (i = 0; i < 16; i++){
		root <<= 1;
		rem = ((rem << 2) + (a >> 30));
		a <<= 2;
		divisor = (root << 1) + 1;
		if (divisor <= rem){
			rem -= divisor;
			root++;
		}
	}
	return root;
}

uint8_t sobel(uint8_t buffer[3][3]){
	int32_t sum = 0, gx = 0, gy = 0;
	uint8_t i, j;

	int16_t kernelx[3][3] =	{	{-1, 0, 1},
					{-2, 0, 2},
					{-1, 0, 1},
				};
	int16_t kernely[3][3] =	{	{-1, -2, -1},
					{0, 0, 0},
					{1, 2, 1},
				};
	for (i = 0; i < 3; i++){
		for (j = 0; j < 3; j++){
			gx += ((int32_t)buffer[i][j] * (int32_t)kernelx[i][j]);
			gy += ((int32_t)buffer[i][j] * (int32_t)kernely[i][j]);
		}
	}
	
	sum = isqrt(gy * gy + gx * gx);

	if (sum > 255) sum = 255;
	if (sum < 0) sum = 0;

	return (uint8_t)sum;
}

void do_gausian(uint8_t *img, int32_t width, int32_t height){
	int32_t i, j, k, l;
	uint8_t image_buf[5][5];
	
	for(i = 0; i < height; i++){
		if (i > 1 || i < height-1){
			for(j = 0; j < width; j++){
				if (j > 1 || j < width-1){
					for (k = 0; k < 5;k++)
						for(l = 0; l < 5; l++)
							image_buf[k][l] = image[(((i + l-2) * width) + (j + k-2))];

					img[((i * width) + j)] = gausian(image_buf);
				}else{
					img[((i * width) + j)] = image[((i * width) + j)];
				}
			}
		}
	}
}

void do_sobel(uint8_t *img, int32_t width, int32_t height){
	int32_t i, j, k, l;
	uint8_t image_buf[3][3];
	
	for(i = 0; i < height; i++){
		if (i > 0 || i < height){
			for(j = 0; j < width; j++){
				if (j > 0 || j < width){
					for (k = 0; k < 3;k++)
						for(l = 0; l < 3; l++)
							image_buf[k][l] = image[(((i + l-1) * width) + (j + k-1))];

					img[((i * width) + j)] = sobel(image_buf);
				}else{
					img[((i * width) + j)] = image[((i * width) + j)];
				}
			}
		}
	}
}

void task(void){
	uint32_t i, j, k = 0;
	uint8_t *img;
	uint32_t time;
	
	while(1) {
		img = (uint8_t *) malloc(height * width);
		if (img == NULL){
			printf("\nmalloc() failed!\n");
			for(;;);
		}

		printf("\n\nstart of processing!\n\n");

		time = _readcounter();

		do_gausian(img, width, height);
		do_sobel(img, width, height);

		time = _readcounter() - time;

		printf("done in %d clock cycles.\n\n", time);

		printf("\n\nint32_t width = %d, height = %d;\n", width, height);
		printf("uint8_t image[] = {\n");
		for (i = 0; i < height; i++){
			for (j = 0; j < width; j++){
				printf("0x%x", img[i * width + j]);
				if ((i < height-1) || (j < width-1)) printf(", ");
				if ((++k % 16) == 0) printf("\n");
			}
		}
		printf("};\n");

		free(img);

		printf("\n\nend of processing!\n");
		panic(0);
	}
		
}

uint8_t *getChunkFromImage(int pieceID, uint8_t *img){
	uint8_t temp[1024];
	int i,j,k;
	k = 0;
	for(i = 0; i < 33; i++){
		for(j =0; j < 33; j++){
			temp[k++] = img[width * i + j];
		}	
	}
	return temp;
}

void master(){
	
	int32_t i = 0;
	uint32_t msg = 0;
	uint32_t aux = 0;
	int8_t buf[1028];
	uint16_t val;
	uint8_t *img;
	uint16_t totalPixels = width * height;
	img = (uint8_t *) malloc(totalPixels);	

	if (hf_comm_create(hf_selfid(), 3, 0))
		panic(0xff);

	delay_ms(100);

	//memcpy(buf, img, 1024);

	while(1){
		printf("\nvou mandar");
		for(i = 1; i <= 8; i++){
			memcpy(buf, ++aux, 4);
			memcpy(buf+4, getChunkFromImage(0, img), 1024);
			val = hf_sendack(i, i*100,buf, sizeof(buf), 0, 1000);
			if (val) printf("sender, hf_sendack(): error %d\n", val);
		}
		while(1){}
	}
}

void slave(){
	
	int8_t buf[1028];
	uint16_t cpu, task, size, val;
	uint8_t data[1024];
	int id;

	if (hf_comm_create(hf_selfid(), hf_cpuid()*100, 0))
		panic(0xff);

	delay_ms(30);

	while(1){
		printf("\nestou esperando");	
		val = hf_recvack(&cpu, &task, buf, sizeof(dataChunk), 0);
			if (val){
				printf("error %d\n", val);
			}
			else{
				memcpy(&id, buf, 4);
				memcpy(data, buf+4, 1024);
				printf("\nDataChunk.id: %i, Datachunk.data: %d", id, data);
				printf("\nMessage Received ! From, CPU: %d Task:%d, EU SOU: %d",cpu, task, hf_cpuid());
			}
			while(1){}
	}
}

void app_main(void) {
	if (hf_cpuid() == 0){
		hf_spawn(master, 0, 0, 0, "master", 4096);
	}else{
		hf_spawn(slave, 0, 0, 0, "slave", 4096);	
	}
}














