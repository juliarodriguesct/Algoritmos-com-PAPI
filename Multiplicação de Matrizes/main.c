#include <stdio.h>
#include <stdlib.h>
#include <papi.h>

#define NUM_EVENTS 1

double** AlocaMatriz (int N, int memoria) {
    int i;
    double **matriz;
    posix_memalign((void**)&matriz, 64, memoria);
    for (i=0;i<N;i++) {
        posix_memalign((void**)&matriz[i], 64, memoria);
    }
    return matriz;
}


double** DesalocaMatriz (int N, double** matriz) {
	int i;
	for (i=0;i<N;i++) {
		free(matriz[i]);
	}
	free(matriz);
	return NULL;
}

void PreencheVetor (int n, double **m) {
    int i, j;
	for (i=0;i<n;i++) {
        for (j=0;j<n;j++){
            m[i][j] = (double) -1000 + (rand() % (2000));
        }
	}
}


void ZeraVetor (double v[]) {
	v[0] = 0, v[1] = 0, v[2] = 0, v[3] = 0, v[4] = 0;
}

void ZeraMatriz (int n, double **m) {
	int i, j;
	for (i=0;i<n;i++) {
        for (j=0;j<n;j++){
            m[i][j] = 0;
        }
	}
}


MultiplicaMatriz (int N, double **A_matriz, double **X_matriz, double **R_matriz) {
    int i, j, k;
    float soma=0;

    for (i=0;i<N;i++) { 
        for (j=0;j<N;j++) {
            for (k=0;k<N;k++) {
                soma += A_matriz[i][k]*X_matriz[k][j];;
            }
            R_matriz[i][j] = soma;
     soma=0;
        }
    }

}

void MediaArquivo (FILE* arq, int memoria, double medias[]) {
    fprintf(arq, "%d ", memoria);
    fprintf(arq, "%.2lf ", medias[0]/10);
    fprintf(arq, "%lf ", medias[1]/10);
    fprintf(arq, "%lf ", medias[2]/10);
    fprintf(arq, "%lf ", medias[3]/10);
    fprintf(arq, "%.10lf\n", medias[4]/10);
    ZeraVetor(medias);
}

void NumeroDeCiclos (int n, double **matriz1, double **matriz2, double **matriz, int memoria, FILE *arq, double vet_valores[]) {
    long long values[NUM_EVENTS];
    int Events[NUM_EVENTS] = {PAPI_TOT_CYC}, EventSet = PAPI_NULL;
    int Papi;

    Papi = PAPI_library_init(PAPI_VER_CURRENT);
    Papi = PAPI_create_eventset(&EventSet);
    Papi = PAPI_add_events(EventSet, Events, NUM_EVENTS);
    Papi = PAPI_start(EventSet);

    MultiplicaMatriz(n, matriz1, matriz2, matriz);

    Papi = PAPI_stop(EventSet, values);

    fprintf(arq, "%lli ", values[0]);
    vet_valores[1]+= (double)values[0];

    PAPI_shutdown();
}


void NumeroDeFlips (int n, double **matriz1, double **matriz2, double **matriz, int memoria, FILE *arq, double vet_valores[]) {
    long long values[NUM_EVENTS];
    int Events[NUM_EVENTS] = {PAPI_FP_INS}, EventSet = PAPI_NULL;
    int Papi;

    Papi = PAPI_library_init(PAPI_VER_CURRENT);
    Papi = PAPI_create_eventset(&EventSet);
    Papi = PAPI_add_events(EventSet, Events, NUM_EVENTS);
    Papi = PAPI_start(EventSet);

    MultiplicaMatriz(n, matriz1, matriz2, matriz);

    Papi = PAPI_stop(EventSet, values);

    fprintf(arq, "%lli ", values[0]);
    vet_valores[2]+= (double)values[0];

    PAPI_shutdown();
}

void NumeroDeFlops (int n, double **matriz1, double **matriz2, double **matriz, int memoria, FILE *arq, double vet_valores[]) {
    long long values[NUM_EVENTS];
    int Events[NUM_EVENTS] = {PAPI_FP_OPS}, EventSet = PAPI_NULL;
    int Papi;

    Papi = PAPI_library_init(PAPI_VER_CURRENT);
    Papi = PAPI_create_eventset(&EventSet);
    Papi = PAPI_add_events(EventSet, Events, NUM_EVENTS);
    Papi = PAPI_start(EventSet);

    MultiplicaMatriz(n, matriz1, matriz2, matriz);

    Papi = PAPI_stop(EventSet, values);

   	fprintf(arq, "%lli ", values[0]);
   	vet_valores[3]+= (double)values[0];

    PAPI_shutdown();
}

void TempoTotal (int n, double **matriz1, double **matriz2, double **matriz, int memoria, FILE *arq, double vet_valores[]) {
    long long values[NUM_EVENTS];
    int EventSet = PAPI_NULL;
    int Papi;

    Papi = PAPI_library_init(PAPI_VER_CURRENT);

    float t0 = (float) PAPI_get_virt_usec();
    MultiplicaMatriz(n, matriz1, matriz2, matriz);
   	float t1 = (float)  PAPI_get_virt_usec();

  	float tempo_exec = (t1-t0)/1000000;

    fprintf(arq, "%.2f ", (float) (memoria*3)/tempo_exec);
    vet_valores[0]+= (double) memoria/tempo_exec;

    fprintf(arq, "%.10f\n", tempo_exec);
    vet_valores[4]+= (double) tempo_exec;

    PAPI_shutdown();
}

int main () {

	double **matriz1;
	double **matriz2;
	double **matriz;
	int memoria, elementos = 2048, count = 0, count2 = 0; 
    FILE* arq = fopen("resultados.txt", "w"); 
    FILE* saida = fopen("medias", "w");
    srand((unsigned)time(NULL)); 

    double medias [5];
    ZeraVetor(medias);


	while(count < 1){
        memoria = elementos*elementos*sizeof(double); 
		matriz1 = AlocaMatriz(elementos, memoria);
        matriz2 = AlocaMatriz(elementos, memoria);
        matriz = AlocaMatriz(elementos, memoria);
      
		for(count2 = 0; count2 < 10; count2++){
			
			PreencheVetor(elementos, matriz1); 
			
			PreencheVetor( elementos, matriz2); 
			NumeroDeCiclos(elementos, matriz1, matriz2, matriz, memoria, arq, medias); 
     	    NumeroDeFlips(elementos, matriz1, matriz2, matriz, memoria, arq, medias);
            NumeroDeFlops(elementos, matriz1, matriz2, matriz, memoria, arq, medias);
    		TempoTotal(elementos, matriz1, matriz2, matriz, memoria, arq, medias);
    		printf("ok linha %d\n", count2);
		}

		matriz1 = DesalocaMatriz (elementos, matriz1);
		matriz2 = DesalocaMatriz (elementos, matriz2);
		matriz = DesalocaMatriz (elementos, matriz);

		MediaArquivo(saida, memoria, medias);

		count++;
		elementos*=2;
	}

    fclose(arq);
    fclose(saida);
	return 0;
}