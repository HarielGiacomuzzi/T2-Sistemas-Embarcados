#include <hellfire.h>
#include <noc.h>
#include "image.h"

typedef struct{
	int i;
	int j;
}Tuple;

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

uint8_t *getChunkFromImage(int linha, int coluna, uint8_t *img){
	uint8_t temp[1024];
	int i,j,k;
	k = 0;
	for(i = linha; i < linha+33; i++){
		for(j =coluna; j < coluna+33; j++){
			temp[k++] = img[width * i + j];
		}	
	}
	return temp;
}

void putChunkInImage(int linha, int coluna, uint8_t *img, uint8_t *data){
	uint8_t temp[1024];
	int i,j,k;
	k = 0;
	for(i = linha; i < linha+33; i++){
		for(j =coluna; j < coluna+33; j++){
			img[width *i + j] = data[k++];
		}	
	}
	return temp;
}

void auxTask(){
	int i,j;
	if (hf_comm_create(hf_selfid(), 200, 0)){
		printf("problema ao criar porta...");	
		panic(0xff);
	}
	uint16_t cpu, task, size;
	int buf[3];
	int val;
	uint8_t *img;
	int8_t buff[1024];
	//recebe configuracoes do processo
	hf_recvack(&cpu, &task, buf, &size, 0);
	hf_recvack(&cpu, &task, img, &size, 0);
	// printf("i: %d  j: %d", buf[0], buf[1]);

	 memcpy(buff, getChunkFromImage(i, j, img), 1024);
	 val = hf_sendack(buf[3], 200,buff, sizeof(buff), 0, 1000);
	 if (val) printf("sender, hf_sendack(): error %d\n", val);	
	 hf_recvack(&cpu, &task, buff, &size, 0);
	 putChunkInImage(buf[0], buf[1], img, buf);
}

void master(){
	
	int i = 0;
	int k = 0;
	int j = 0;
	uint16_t cpu, task, val, size;
	int8_t *buf;
	int buff[3];
	Tuple memory[9];
	uint8_t *img;
	uint16_t totalPixels = width * height;
	img = (uint8_t *) malloc(totalPixels);	
	buf = (uint8_t *) malloc(totalPixels);	

	if (hf_comm_create(hf_selfid(), 100, 0)){
		printf("problema ao criar porta...");	
		panic(0xff);
	}

	delay_ms(10);

	while(1){
		printf("\nvou mandar");
		for(k = 1; k <= 8; k++){	
			hf_spawn(auxTask, 0, 0, 0, "auxTask", 4096);
			buff[0] = i;
			buff[1] = j;
			buff[2] = k;
			//memcpy(buf, getChunkFromImage(i, j, img), 1024);
			val = hf_sendack(hf_cpuid(), 200,buff, sizeof(buff), 0, 1000);
			if (val) printf("sender, hf_sendack(): error %d\n", val);	
			val = hf_sendack(hf_cpuid(), 200,&img, sizeof(uint8_t), 0, 1000);
			if (val) printf("sender, hf_sendack(): error %d\n", val);	
			memory[k].i = i;
			memory[k].j = j;
			i+=32;
			if(i >= width){
				j+=32;
				i=0;
			}
		}
		while(1){
			hf_recvack(&cpu, &task, buf, &size, 0);
			printf("Oi recebi algo!\n");
			putChunkInImage(memory[cpu].i,memory[cpu].j,img, buf);
			i += 32;
			if(i >= width){
				j+=32;
				i=0;
			}
			if(i >= width && j >= height){
				printf("\nResultado:\n0x%d,", img);
				continue;
			}
			if(i < width && j < height){
				memcpy(buf, getChunkFromImage(i,j,img), 1024);
				hf_sendack(cpu, 100, buf, sizeof(buf), 0, 1000);
				memory[cpu].i = i;
				memory[cpu].j = j;
			}
		}
	}
}

void slave(){
	
	int8_t buf[1024];
	int val;
	uint16_t cpu, task, size;
	uint8_t data[1024];

	if (hf_comm_create(hf_selfid(), hf_cpuid()*100, 0))
		panic(0xff);

	while(1){
		val = hf_recvack(&cpu, &task, buf, &size, 0);
			if (val){
				printf("error %d\n", val);
			}
			else{
				memcpy(data, buf, 1024);
				do_gausian(data, 32,32);
				do_sobel(data, 32,32);
				printf("Ja terminei, vou mandar de volta\n");
				val = 1;
				while(val != 0){
					int r = random() % 20;
					delay_ms(r);
					val = hf_sendack(cpu, hf_cpuid()*100, data, 1024, 0, 500);
				}
				//printf("%d\n", data);
			}
	}
}

void app_main(void) {
	if (hf_cpuid() == 0){
		hf_spawn(master, 0, 0, 0, "master", 16384);
	}else{
		hf_spawn(slave, 0, 0, 0, "slave", 16384);	
	}
}















