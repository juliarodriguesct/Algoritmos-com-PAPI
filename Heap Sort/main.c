#include <stdio.h>
#include <stdlib.h>
#include <papi.h>

#define NUM_EVENTS 1
#define LOOPS 22
#define TAM 1024

void PreencheVetor (long int *v, int n) {
    int i;
    for (i=0;i<n;i++) {
        v[i] = -1000 + (rand() % (2000)); 
    }
}


void ZerarVetor (double v[]) {
    v[0] = 0, v[1] = 0, v[2] = 0, v[3] = 0, v[4] = 0;
}

void HeapSort (long int *a, int n) {
    int i;
    for (i = (n - 2) / 2; i >= 0; i--) {
        DownHeap(a, n, i);
    }
    for (i = 0; i < n; i++) {
        int t = a[n - i - 1];
        a[n - i - 1] = a[0];
        a[0] = t;
        DownHeap(a, n - i - 1, 0);
    }
}

void DownHeap (long int *a, int n, int i) {
    while (1) {
        int j = Maximo(a, n, i, 2 * i + 1, 2 * i + 2);
        if (j == i) {
            break;
        }
        int t = a[i];
        a[i] = a[j];
        a[j] = t;
        i = j;
    }
}

int Maximo (long int *a, int n, int i, int j, int k) {
    int m = i;
    if (j < n && a[j] > a[m]) {
        m = j;
    }
    if (k < n && a[k] > a[m]) {
        m = k;
    }
    return m;
}

void MediaArquivo (FILE* arq, int memoria, double medias[]) {
    fprintf(arq, "%d ", memoria);
    fprintf(arq, "%.2lf ", medias[0]/10);
    fprintf(arq, "%lf ", medias[1]/10);
    fprintf(arq, "%lf ", medias[2]/10);
    fprintf(arq, "%lf ", medias[3]/10);
    fprintf(arq, "%.10lf\n", medias[4]/10);
    ZerarVetor(medias);
}

void NumeroDeCiclos (long int *v, int n, int memoria, FILE *arq, double medias[]) {

	PreencheVetor(v, n);
    
    long long values[NUM_EVENTS];
    int Events[NUM_EVENTS] = {PAPI_TOT_CYC}, EventSet = PAPI_NULL;  
    int Papi;

    Papi = PAPI_library_init(PAPI_VER_CURRENT);
    Papi = PAPI_create_eventset(&EventSet);
    Papi = PAPI_add_events(EventSet, Events, NUM_EVENTS);

    Papi = PAPI_start(EventSet);
    HeapSort(v, n);
    Papi = PAPI_stop(EventSet, values);

    fprintf(arq, "%lli ", values[0]);
    medias[2]+= (double)values[0];

    PAPI_shutdown();    
} 

void NumeroDeFlips (long int *v, int n, int memoria, FILE *arq, double medias[]) {

	PreencheVetor(v, n);

    long long values[NUM_EVENTS];
    int Events[NUM_EVENTS] = {PAPI_FP_INS}, EventSet = PAPI_NULL;   
    int Papi;

    Papi = PAPI_library_init(PAPI_VER_CURRENT);
    Papi = PAPI_create_eventset(&EventSet);
    Papi = PAPI_add_events(EventSet, Events, NUM_EVENTS);

    Papi = PAPI_start(EventSet);
    HeapSort(v, n);
    Papi = PAPI_stop(EventSet, values);
  
    fprintf(arq, "%lli ", values[0]);
    medias[2]+= (double)values[0];
       
    PAPI_shutdown();    
}

void NumeroDeFlops (long int *v, int n, int memoria, FILE *arq, double medias[]) {

	PreencheVetor(v, n);

    long long values[NUM_EVENTS];
    int Events[NUM_EVENTS] = {PAPI_FP_OPS}, EventSet = PAPI_NULL;   
    int Papi;

    Papi = PAPI_library_init(PAPI_VER_CURRENT);
    Papi = PAPI_create_eventset(&EventSet);
    Papi = PAPI_add_events(EventSet, Events, NUM_EVENTS);
    
    Papi = PAPI_start(EventSet);
    HeapSort(v, n);
    Papi = PAPI_stop(EventSet, values);
  
    fprintf(arq, "%lli ", values[0]);
   	medias[3]+= (double)values[0];
        
    PAPI_shutdown();    
}

void TempoTotal (long int *v, int n, int memoria, FILE *arq, double medias[]) {

	PreencheVetor(v, n);

    long long values[NUM_EVENTS];
    int Papi;
    int EventSet = PAPI_NULL;

    Papi = PAPI_library_init(PAPI_VER_CURRENT);
    Papi = PAPI_create_eventset(&EventSet);
    
    Papi = PAPI_start(EventSet);
    float t0 = (float) PAPI_get_virt_usec();
    HeapSort(v, n);

    Papi = PAPI_start(EventSet);
    float t1 = (float)  PAPI_get_virt_usec();
    Papi = PAPI_stop(EventSet, values);

    float tempo_execucao = (t1-t0)/1000000;
  
    fprintf(arq, "%.2lf ", (double) memoria/tempo_execucao);
    medias[0]+= (double) memoria/tempo_execucao;

    fprintf(arq, "%.10f\n", tempo_execucao);
    medias[4]+= (double) tempo_execucao;
 
       
    PAPI_shutdown();    
}


int main(){
    long int *vetor = NULL;
    int memoria = TAM*TAM*TAM, elementos, count = 0, count2 = 0;
    FILE* arq = fopen("resultados.txt", "w");
    FILE* saida = fopen("medias", "w");

    double medias [5];
    ZerarVetor(medias);
	srand((unsigned)time(NULL)); 
    
    while(count < LOOPS){
        elementos = memoria/sizeof(long int); 
        posix_memalign((void**)&vetor, 64, memoria); 
        for(count2 = 0; count2 < 10; count2++){
            NumeroDeCiclos(vetor,elementos,memoria, arq, medias);
            NumeroDeFlips(vetor,elementos,memoria, arq, medias); 
            NumeroDeFlops(vetor,elementos,memoria, arq, medias); 
            TempoTotal(vetor,elementos,memoria, arq, medias); 
        }
        free(vetor);
		count++;

		MediaArquivo(saida, memoria, medias);
		memoria = memoria * 2;
    }
  
    fclose(arq);
    fclose(saida);
    return 0;

}
