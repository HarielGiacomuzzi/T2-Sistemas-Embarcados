#include <hellfire.h>
#include <noc.h>

void sender0(void)
{
	int32_t i = 0,msg = 0;
	int8_t buf[500];
	uint16_t val;
	
	if (hf_comm_create(hf_selfid(), 3, 0))
		panic(0xff);
	
	delay_ms(40);

	while (1){
		for (i = 1; i <= 2; i++, msg++){
			if (hf_cpuid() != i){
				sprintf(buf, "hello from cpu %d thread %d msg %d\n", hf_cpuid(), hf_selfid(), msg);
				val = hf_sendack(i, i*1000, buf, sizeof(buf), 0, 500);
				if (val) printf("sender, hf_sendack(): error %d\n", val);
			}
		}
		while(1){}
	}
}

void sender1(void)
{
	int32_t i = 0, msg = 0;
	int8_t buf[500];
	uint16_t cpu, task, size, val;
	
	if (hf_comm_create(hf_selfid(), 1000, 0))
		panic(0xff);
	
	delay_ms(30);

	while (1){
		val = hf_recvack(&cpu, &task, buf, &size, 0);
		if (val)
			printf("error %d\n", val);
		else
			 printf("Message Received ! From, CPU: %d Task:%d MSG:%s\n",cpu, task,buf);

		if (hf_cpuid() != i){
			sprintf(buf, "hey from cpu %d thread %d msg %d\n", hf_cpuid(), hf_selfid(), msg);
			val = hf_sendack(3, 3000, buf, sizeof(buf), 0, 500);
			if (val) printf("Error on sending message to CPU: 3, Port: 3000, Error number:%d\n", val);
			msg++;
		}
		while(1){}
	}
}

void sender2(void)
{
        int32_t i = 0, msg = 0;
        int8_t buf[500];
	uint16_t cpu, task, size, val;

        if (hf_comm_create(hf_selfid(), 2000, 0))
                panic(0xff);

	delay_ms(30);

        while (1){
		val = hf_recvack(&cpu, &task, buf, &size, 0);
                if (val)
                        printf("error %d\n", val);
                else
                         printf("Message Received ! From, CPU: %d Task:%d MSG:%s\n",cpu, task,buf);

		if (hf_cpuid() != i){
			sprintf(buf, "hello from cpu %d thread %d msg %d\n", hf_cpuid(), hf_selfid(), msg);
			val = hf_sendack(4, 4000, buf, sizeof(buf), 0, 500);
			if (val) printf("Error on sending message to CPU: 4, Port: 4000, Error number:%d\n", val);
		}
		while(1){}
        }
}

void sender3(void)
{
        int32_t i = 0, msg = 0;
        int8_t buf[500];
	uint16_t cpu, task, size, val;

        if (hf_comm_create(hf_selfid(), 3000, 0))
                panic(0xff);

	delay_ms(20);

        while (1){
		val = hf_recvack(&cpu, &task, buf, &size, 0);
                if (val)
                        printf("error %d\n", val);
                else
                         printf("Message Received ! From, CPU: %d Task:%d MSG:%s\n",cpu, task,buf);

		if (hf_cpuid() != i){
			sprintf(buf, "hello from cpu %d thread %d msg %d\n", hf_cpuid(), hf_selfid(), msg);
			val = hf_sendack(5, 5000, buf, sizeof(buf), 0, 500);
			if (val) printf("Error on sending message to CPU: 5, Port: 5000, Error number:%d\n", val);
		}
		while(1){}
        }
}

void sender4(void)
{
        int32_t i = 0, msg = 0;
        int8_t buf[500];
	uint16_t cpu, task, size, val;

        if (hf_comm_create(hf_selfid(), 4000, 0))
                panic(0xff);

	delay_ms(20);

        while (1){
		val = hf_recvack(&cpu, &task, buf, &size, 0);
                if (val)
                        printf("error %d\n", val);
                else
                         printf("Message Received ! From, CPU: %d Task:%d MSG:%s\n",cpu, task,buf);

		if (hf_cpuid() != i){
			sprintf(buf, "hello from cpu %d thread %d msg %d\n", hf_cpuid(), hf_selfid(), msg);
			val = hf_sendack(5, 5000, buf, sizeof(buf), 0, 500);
			if (val) printf("Error on sending message to CPU: 5, Port: 5000, Error number:%d\n", val);
                }
		while(1){}
        }
}

void receiver5(void)
{
        int8_t buf[500];
        uint16_t cpu, task, size, val;

        if (hf_comm_create(hf_selfid(), 5000, 0))
                panic(0xff);

        while (1){
                val = hf_recvack(&cpu, &task, buf, &size, 0);
                if (val)
                        printf("error %d\n", val);
                else
                        printf("Message Received ! From, CPU: %d Task:%d MSG:%s\n",cpu, task,buf);
        }
}


void app_main(void)
{
	if (hf_cpuid() == 0){
		hf_spawn(sender0, 0, 0, 0, "sender0", 2048);
	}
	if (hf_cpuid() == 1){
                hf_spawn(sender1, 0, 0, 0, "sender1", 2048);
        }
	if (hf_cpuid() == 2){
                hf_spawn(sender2, 0, 0, 0, "sender2", 2048);
	}
	if (hf_cpuid() == 3){
                hf_spawn(sender3, 0, 0, 0, "sender3", 2048);
	}
	if (hf_cpuid() == 4){
                hf_spawn(sender4, 0, 0, 0, "sender4", 2048);
	}
	if (hf_cpuid() == 5){
		hf_spawn(receiver5, 0, 0, 0, "receiver5", 2048);
	}
	
}
