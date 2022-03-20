#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>


void* writer(void* arg);
void* reader(void* arg);

sem_t roomEmpty;
sem_t turnstile;
sem_t mutex;
int brReaders = 0;//broji koliko je citalaca u sobi odnosno baferu

int main(int argc,char*argv[]) {

	int brW;
	int brR;
	printf("Unesite broj citalaca:\n");
	scanf("%d", &brR);
	printf("Unesite broj pisaca:\n");
	scanf("%d", &brW);
	int a[brW];
	int b[brR];
	pthread_t writers[brW], readers[brR];

	for (int i = 0; i < brW; i++) {
		a[i] = i;
	}
	for (int i = 0; i < brR; i++) {
		b[i] = i;
	}

	sem_init(&roomEmpty, 0, 1);//inicijalizujem na 1 a 0 znaci da se deli izmedju niti//bice vrednost 1 ako nema niti, a 0 ako ima
	sem_init(&turnstile, 0, 1);
	sem_init(&mutex, 0, 1);

	for (int i = 0; i < brR; i++) {
		if(pthread_create(&readers[i], NULL, reader, (void*)&b[i])!=0)//prvi parametar adresa se samo prosledjuje,drugi NULL znaci da se koriste standardni atributi ,trece kao rutina ,zadnje argumenti
				perror("Greska prilikom kreiranja niti");
				
	}
	for (int i = 0; i < brW; i++) {
		if(pthread_create(&writers[i], NULL, writer, (void*)&a[i])!=0)//pthread_create(pthread_t *restrict thread,const pthread_attr_t* restrict attr,	void* (*start_routine)(void*),	void* restrict arg);
		perror("Greska prilikom kreiranja niti"); 
			
	}
	for (int i = 0; i < brR; i++) {
		pthread_join(readers[i], NULL);//pthread_join(pthread_t thread, void **retval)//xslicno kao wait ali za threads

	}
	for (int i = 0; i < brW; i++) {
		pthread_join(writers[i], NULL);
	}

	printf("Kraj");
	return 0;
}


void* writer(void* arg) {
	sem_wait(&turnstile);//turnstile za Writere je kao mutex i kad ga smanji sa 1 na 0 onda nece moci Readeri da pristupe cekace da zavrsi. Ako je 0 onda ce cekati da zavrsi zadnji reader i ubacice se odmah posle
	sem_wait(&roomEmpty);//ako je 1 smanjice na 0 a ako je 0 morace da saceka//oni trebaju da provere roomempty dok je u okviru turnstile
	printf("%d. pise\n", *(int*)arg + 1);
	sem_post(&turnstile);//vraca vrednost na 1
	sem_post(&roomEmpty);//kada zavrsi bice prazna soba odnosno roomempty 1
}

void* reader(void* arg) {
	sem_wait(&turnstile); //da bih izbegla moguce gladovanje Writera nece se dodavati novi readeri tj da ne bi samo readeri dolazi i onda bi previse cekao writer ovako se bar jedan ubaci jer moze i zauvek da ceka
							//ako bi dosao writer u medjuvremenu zbog turnstile ne bi mogli novi readeri tj. bice privremeno blokirani a ovaj writer ce cekati da soba bude prazna tj. svi readeri odu
	sem_post(&turnstile);  
	sem_wait(&mutex);//mutex je kao token dozvoljava jednoj niti da bude procesuirana, znaci kad vise citalaca dodje
	brReaders++;//dosao je jos jedan citalac

	if (brReaders == 1) {
		sem_wait(&roomEmpty);//prvi reader mora da ceka ako nije prazna 
	}

	sem_post(&mutex);
	//sada citalac korsti(cita) bazu podataka,kriticna sekcija
	printf("%d. cita\n", *(int*)arg + 1);

	sem_wait(&mutex);
	brReaders -= 1;//smanjuje se broj readera koji su u sobi
	if (brReaders == 0) {
		sem_post(&roomEmpty);//kada nema vise citalaca vracam vrednost rommEmpty na 1
	}
	sem_post(&mutex);//vraca se mutex na 1
}
