#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <papi.h>

#define NUM_EVENTS 1
#define PI -3.141517

double** AlocaMatriz (int N, int memoria) {
    int i;
    double **matriz;
    posix_memalign((void**)&matriz, 64, memoria);
    for (i=0;i<N;i++) {
        posix_memalign((void**)&matriz[i], 64, memoria);
    }
    return matriz;
}

double* AlocaVetor (int N, int memoria) {
		double* vetor;
  	posix_memalign((void**)&vetor, 64, memoria);
  	return vetor;
}

double** DesalocaMatriz (int N, double** matriz) {
	int i;
	for (i=0;i<N;i++) {
		free(matriz[i]);
	}
	free(matriz);
	return NULL;
}

double* DesalocaVetor (double* vetor) {
	free(vetor);
	return NULL;
}

void PreencheMatriz (int n, double** matriz) {
  int i, j;
	for (i=0;i<n;i++)
		for (j=0;j<n;j++)
			matriz[i][j] = 0;
	for (i=0;i<n;i++) matriz[i][i] = PI*2;
	for (i=1;i<n;i++) matriz[i][i-1] = PI;
	for (i=1;i<n;i++) matriz[i-1][i] = PI;
}

void PreencheVetor (int n, double* vetor) {
  int i;
  vetor[0] = 200;
  vetor[n-1] = 0;
    for (i=1;i<n-1;i++){
      vetor[i] = 150;
  	}
}

void ZeraVetor (double v[]) {
	v[0] = 0, v[1] = 0, v[2] = 0, v[3] = 0, v[4] = 0;
}

void PreLU (int N, double** L, double** U, double* X, double* Y) {
	int i, j;
	for (i=0;i<N;i++) {
    		X[i] = 0;
    		Y[i] = 0;
        for (j=0;j<N;j++) {
            U[i][j]=0;
            if (i==j) {
                L[i][j]=1;
            } else {
                L[i][j]=0;
            }
        }
    }
}

void FatoraçaoLU (int N, double** A, double** L, double** U) {
  int i = 0, j = 0, k = 0;
  double soma = 0;  
  
  for (i=0;i<N;i++) {
            U[0][i]=A[0][i];
        }
        for (i=1;i<N;i++) {
            L[i][0]=A[i][0]/U[0][0];
        }
        for (i=1;i<N;i++) {
            U[1][i]=A[1][i] - L[1][0]*U[0][i];
        }
        for (i=2;i<N;i++) {
            L[i][1]=(A[i][1]-L[i][0]*U[0][1])/U[1][1];
        }
        for (i=2;i<N;i++) {
            for(j=2;j<N;j++) {
                if (i<=j) {
                    for (k=0;k<=i-1;k++) {
                        soma+=L[i][k]*U[k][j];
                    }
                    U[i][j]=A[i][j]-soma;
                    soma=0;
                } else if (i>j) {
                    for (k=0;k<=j-1;k++) {
                        soma+=L[i][k]*U[k][j];
                    }
                    L[i][j]=(A[i][j]-soma)/U[j][j];
                    soma=0;
                }
            }
        }
}

void ResolveSistema (int N, double** L, double** U, double* B, double* X, double* Y) {
		 int i = 0, j = 0, k = 0;
     double soma=0;
  
		for (i=0;i<N;i++) {
            for (j=0;j<N;j++) {
                    if (i==j) {
                        if (i==0 && j==0) {
                            Y[i]=B[i]/L[i][j];
                        } else {
                            for (k=0;k<j;k++) {
                                soma+=Y[k]*L[i][k];
                            }
                            Y[i]=(B[i]-soma)/L[i][j];
                            soma=0;
                        }
                    }
            }
        }

        for (i=N-1;i>=0;i--) {
            for (j=N-1;j>=0;j--) {

                    if (i==j) {
                        if (i==N-1 && j==N-1) {
                            X[i]=Y[i]/U[i][j];
                        } else {
                            for (k=N-1;k>j;k--) {
                                soma+=X[k]*U[i][k];
                            }
                            X[i]=(Y[i]-soma)/U[i][j];
                            soma=0;
                        }
                    }
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

void NumeroDeCiclos (int elementos, double **A, double **L, double **U, double *B, double* X, double *Y,  int memoria, FILE *arq, double vet_valores[]) {
	
	PreLU(elementos, L, U, X, Y);
    
    long long values[NUM_EVENTS];
    int Events[NUM_EVENTS] = {PAPI_TOT_CYC}, EventSet = PAPI_NULL;
    int Papi;

    Papi = PAPI_library_init(PAPI_VER_CURRENT);
    Papi = PAPI_create_eventset(&EventSet);
    Papi = PAPI_add_events(EventSet, Events, NUM_EVENTS);
    Papi = PAPI_start(EventSet);

    FatoraçaoLU(elementos, A, L, U);
  	ResolveSistema(elementos, L, U, B, X, Y);

    Papi = PAPI_stop(EventSet, values);

    fprintf(arq, "%lli ", values[0]);
    vet_valores[1]+= (double)values[0];

    PAPI_shutdown();
}

void NumeroDeFlips (int elementos, double **A, double **L, double **U, double *B, double *X, double *Y, int memoria, FILE *arq, double vet_valores[]) {
    
	PreLU(elementos, L, U, X, Y);

    long long values[NUM_EVENTS];
    int Events[NUM_EVENTS] = {PAPI_FP_INS}, EventSet = PAPI_NULL;
    int Papi;

    Papi = PAPI_library_init(PAPI_VER_CURRENT);
    Papi = PAPI_create_eventset(&EventSet);
    Papi = PAPI_add_events(EventSet, Events, NUM_EVENTS);
    Papi = PAPI_start(EventSet);

    FatoraçaoLU(elementos, A, L, U);
  	ResolveSistema(elementos, L, U, B, X, Y);

    Papi = PAPI_stop(EventSet, values);

    fprintf(arq, "%lli ", values[0]);
    vet_valores[2]+= (double)values[0];

    PAPI_shutdown();
}

void NumeroDeFlops (int elementos, double **A, double **L, double **U, double *B, double *X, double *Y, int memoria, FILE *arq, double vet_valores[]) {
    
    PreLU(elementos, L, U, X, Y);

    long long values[NUM_EVENTS];
    int Events[NUM_EVENTS] = {PAPI_FP_OPS}, EventSet = PAPI_NULL;
    int Papi;

    Papi = PAPI_library_init(PAPI_VER_CURRENT);
    Papi = PAPI_create_eventset(&EventSet);
    Papi = PAPI_add_events(EventSet, Events, NUM_EVENTS);
    Papi = PAPI_start(EventSet);

    FatoraçaoLU(elementos, A, L, U);
  	ResolveSistema(elementos, L, U, B, X, Y);

    Papi = PAPI_stop(EventSet, values);

   	fprintf(arq, "%lli ", values[0]);
   	vet_valores[3]+= (double)values[0];

    PAPI_shutdown();
}

void TempoTotal (int elementos, double **A, double **L, double **U, double *B, double *X, double *Y, int *memoria, FILE *arq, double vet_valores[]) {
    
	PreLU(elementos, L, U, X, Y);

    long long values[NUM_EVENTS];
    int EventSet = PAPI_NULL;
    int Papi;

    Papi = PAPI_library_init(PAPI_VER_CURRENT);

    float t0 = (float) PAPI_get_virt_usec();
    FatoraçaoLU(elementos, A, L, U);
  	ResolveSistema(elementos, L, U, B, X, Y);
   	float t1 = (float)  PAPI_get_virt_usec();

  	float tempo_exec = (t1-t0)/1000000;

  	*memoria = elementos*elementos*sizeof(double)*3 + elementos*sizeof(double)*3;
  	double m = *memoria;

    fprintf(arq, "%.2lf ", (double) m/tempo_exec);
    vet_valores[0]+= (double) m/tempo_exec;

    fprintf(arq, "%.10f\n", tempo_exec);
    vet_valores[4]+= (double) tempo_exec;

    PAPI_shutdown();
}

int main () {
	int count = 0, count2 = 0, elementos = 128, memoria = 0;
	double **A, **L, **U, *B, *X, *Y;
	FILE* arq = fopen("resultados.txt", "w");
    FILE* saida = fopen("medias", "w");

    double medias [5];
    ZeraVetor(medias);

    while (count < 6) {
    	A = AlocaMatriz(elementos, sizeof(double)*elementos);
		L = AlocaMatriz(elementos, sizeof(double)*elementos);
		U = AlocaMatriz(elementos, sizeof(double)*elementos);
  		B = AlocaVetor(elementos, sizeof(double)*elementos);
  		X = AlocaVetor(elementos, sizeof(double)*elementos);
		Y = AlocaVetor(elementos, sizeof(double)*elementos);

		PreencheMatriz(elementos, A);
  		PreencheVetor(elementos, B);

  		for (count2 = 0; count2<10; count2++) {
  		NumeroDeCiclos(elementos, A, L, U, B, X, Y, memoria, arq, medias);
			NumeroDeFlips(elementos, A, L, U, B, X, Y, memoria, arq, medias);
			NumeroDeFlops(elementos, A, L, U, B, X, Y, memoria, arq, medias);
			TempoTotal(elementos, A, L, U, B, X, Y, &memoria, arq, medias);
  		}

		A = DesalocaMatriz(elementos, A);
  		L = DesalocaMatriz(elementos, L);
  		U = DesalocaMatriz(elementos, U);
  		B = DesalocaVetor(B);
  		X = DesalocaVetor(X);
  		Y = DesalocaVetor(Y);

		MediaArquivo(saida, memoria, medias);

  		count++;
  		elementos=elementos*2;
    } 
	
	fclose(arq);
	fclose(saida);
	return 0;
}