#include <stdint.h>		/* intptr_t for 64bits machines */
#include <pthread.h>
#include <stdio.h>		/* incloure definicions de funcions estandard */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "winsuport2.h"
#include "memoria.h"	/* incloure definicions de funcions propies */
#include "semafor.h"
#include "missatge.h"
#include <signal.h>

#define MAX_THREADS	10
#define MAXFANTASMES 9
#define MIN_FIL 7		/* definir limits de variables globals */
#define MAX_FIL 25
#define MIN_COL 10
#define MAX_COL 80


				/* definir estructures d'informacio */
typedef struct {		/* per un objecte (menjacocos o fantasma) */
	int f;				/* posicio actual: fila */
	int c;				/* posicio actual: columna */
	int d;				/* direccio actual: [0..3] */
  float r;            /* per indicar un retard relati */
	char a;				/* caracter anterior en pos. actual */
} objecte;


/* variables globals */
int n_fil1, n_col;		/* dimensions del camp de joc */
char tauler[70];		/* nom del fitxer amb el laberint de joc */
char c_req;			    /* caracter de pared del laberint */

objecte mc;      		/* informacio del menjacocos */
objecte f1;			    /* informacio del fantasma 1 */

int df[] = {-1, 0, 1, 0};	/* moviments de les 4 direccions possibles */
int dc[] = {0, -1, 0, 1};	/* dalt, esquerra, baix, dreta */

int cocos;			/* numero restant de cocos per menjar */
int retard;		    /* valor del retard de moviment, en mil.lisegons */
int id_fi1, *fi1, id_fi2, *fi2;
int id_nfActius;
int *nfActius;
pthread_t tid[MAX_THREADS];

//IDENTIFICADORS DE MEMORIA COMPARTIDA
void* p_tauler;
int id_tauler;                                             /*Variable que guarda el tamany del tauler*/
char id_fi1c[10],id_fi2c[10];
pid_t tpid[MAXFANTASMES];		/*Creacio de la taula de processos*/
int id_fantasma;
int id_bustia[MAXFANTASMES];
char a4[20];
char *token;
int id_sem;
bool caza;
char missatgeE[100];
char missatgeR[100];
bool cazaTodos=false;
int outModeC=0;
pthread_t tid[MAX_THREADS];



void modeCaceria(){
    
    objecte seg;
    int resultat;
    int k, vk, nd, vd[3];
    int retardFantasma;
    char buffer[10];
    int nMostraFantasma=id_fantasma+1;
    sprintf(buffer, "%d", nMostraFantasma);
    
    do{
        resultat = 0; nd = 0;
        retardFantasma=f1.r;  // Retardo en función del índice
       

        //waitS(id_sem);
        for (k=-1; k<=1; k++) {   /* provar direccio actual i dir. veines */
            vk = (f1.d + k) % 4; /* direccio veina */
            if (vk < 0) vk += 4;  /* corregeix negatius */
            seg.f = f1.f + df[vk]; /* calcular posicio en la nova dir.*/
            seg.c = f1.c + dc[vk];
            
            seg.a = win_quincar(seg.f,seg.c); /* calcular caracter seguent posicio */
           
            if ((seg.a==' ') || (seg.a=='.') || (seg.a=='0')) {
                vd[nd] = vk;  /* memoritza com a direccio possible */
                nd++;
            }
           
        }
        if (nd == 0) {  /* si no pot continuar, */
            f1.d = (f1.d + 2) % 4;  /* canvia totalment de sentit */
        } else {
            if (nd == 1) {  /* si nomes pot en una direccio */
                f1.d = vd[0];  /* li assigna aquesta */
            } else {  /* altrament */
                f1.d = vd[rand() % nd];  /* segueix una dir. aleatoria */
            }
            
            seg.f = f1.f + df[f1.d];  /* calcular seguent posicio final */
            seg.c = f1.c + dc[f1.d];
            
            seg.a = win_quincar(seg.f,seg.c); /* calcular caracter seguent posicio */
            
            
            win_escricar(f1.f,f1.c,f1.a,NO_INV);  /* esborra posicio anterior */            

            f1.f = seg.f; f1.c = seg.c; f1.a = seg.a;  /* actualitza posicio */
                      
            
            win_escricar(f1.f, f1.c, buffer[0], INVERS);
                    
            if (f1.a == '0') resultat = 1; /* ha capturat menjacocos */
        }
        //signalS(id_sem);
        if (resultat) {
            *fi2=resultat;
            break;  // Sale del bucle si ha capturado al personaje principal
        }
        
        win_retard(retardFantasma*retard);  // Retardo


    }while(outModeC!=-1 && *fi1==0 && *fi2==0);
    
}





void* info(void* nul){
    do{
        receiveM(id_bustia[id_fantasma],missatgeR);
        int valor = atoi(missatgeR); // Convierte el mensaje a un entero

        if (valor >= 0 && valor <= 8){
            outModeC=valor;
            modeCaceria();
        }else{
            outModeC=valor;
        }
        
    }while(*fi1==0 && *fi2==0);  
    return NULL;  
    
}





int main(int n_args, char *ll_args[])
{   
    //TAULER//
	id_tauler = atoi(ll_args[1]);      /* obtenir dimensions del camp de joc */
    n_fil1 = atoi(ll_args[2]);		
	n_col = atoi(ll_args[3]);
	

    //FI1 y FI2// 
    id_fi1=atoi(ll_args[4]);
    id_fi2=atoi(ll_args[5]);
   
    //Retard// 
    retard=atoi(ll_args[6]);
    
    //SEMAFOR//
    id_sem = atoi(ll_args[7]);

    //NUM_PROCES//
    id_fantasma = atoi(ll_args[8]);

    //BUSTIA//
    for (int i = 0; i < 9; i++)
	{
		id_bustia[i] = atoi(ll_args[i + 9]);	// Carreguem els ids de les busties en un array de caracters
	}
    
    id_nfActius= atoi(ll_args[18]);
    nfActius=map_mem(id_nfActius);
    
    

    receiveM(id_bustia[id_fantasma],a4);

    token = strtok(a4, ";");

    // Guardar cada salida en las variables del objeto f1
    f1.f = atoi(token);

    token = strtok(NULL, ";");
    f1.c = atof(token);

    token = strtok(NULL, ";");
    f1.d = atof(token);

    token = strtok(NULL, ";");
    f1.r = atof(token);

    

    p_tauler = map_mem(id_tauler);  /* obtenir adres. de mem. compartida */
	if (p_tauler == (int*) -1)
	{
    	fprintf(stderr, "proces (%d): error en identificador de memoria \n",(int)getpid());
    	exit(0);
  	}
	win_set(p_tauler,n_fil1,n_col);	/* crea acces a finestra oberta pel proces pare */
	

	
	fi1 = map_mem(id_fi1);		/* obtenir adres. de mem. compartida */
	fi2 = map_mem(id_fi2);		/* obtenir adres. de mem. compartida */
	
    pthread_create(&tid[0], NULL, info, (void *)(intptr_t)0);

    objecte aux;
    objecte seg;
    int resultat;
    int k, vk, nd, vd[3];
    int retardFantasma;
    int nMostraFantasma;
    char buffer[10];
    nMostraFantasma=id_fantasma;
    
    
    f1.a='.';
    
    nMostraFantasma=id_fantasma+1;
    sprintf(buffer, "%d", nMostraFantasma);
    caza=false;
    bool acabaBucle=false;
    
    do {  // Bucle propio de la función
        win_set(p_tauler,n_fil1,n_col);
        resultat = 0; nd = 0;
        retardFantasma=f1.r;  // Retardo en función del índice
        

        //waitS(id_sem);
        for (k=-1; k<=1; k++) {   /* provar direccio actual i dir. veines */
            vk = (f1.d + k) % 4; /* direccio veina */
            if (vk < 0) vk += 4;  /* corregeix negatius */
            seg.f = f1.f + df[vk]; /* calcular posicio en la nova dir.*/
            seg.c = f1.c + dc[vk];
            
            seg.a = win_quincar(seg.f,seg.c); /* calcular caracter seguent posicio */
           
            if ((seg.a==' ') || (seg.a=='.') || (seg.a=='0')) {
                vd[nd] = vk;  /* memoritza com a direccio possible */
                nd++;
            }
           
        }
        if (nd == 0) {  /* si no pot continuar, */
            f1.d = (f1.d + 2) % 4;  /* canvia totalment de sentit */
        } else {
            if (nd == 1) {  /* si nomes pot en una direccio */
                f1.d = vd[0];  /* li assigna aquesta */
            } else {  /* altrament */
                f1.d = vd[rand() % nd];  /* segueix una dir. aleatoria */
            }
    
            seg.f = f1.f + df[f1.d];  /* calcular seguent posicio final */
            seg.c = f1.c + dc[f1.d];
            
            seg.a = win_quincar(seg.f,seg.c); /* calcular caracter seguent posicio */
            
            //prismatics
            aux.f=seg.f;
            aux.c=seg.c;
            aux.a=seg.a;

            acabaBucle=false;
            //caza=false;
            while (!acabaBucle) {
                aux.f = df[f1.d]+aux.f;
                aux.c = dc[f1.d]+aux.c;
                aux.a = win_quincar(aux.f, aux.c);
                if ((aux.a == '+') || (aux.a >= '1' && aux.a <= '9') || (aux.a == '0')) {
                    acabaBucle=true;
                    if(aux.a == '0'){
                        caza = true;
                        sprintf(missatgeE, "%d", id_fantasma);						
                        for(int i =0; i<*nfActius;i++){
                            if(i!=id_fantasma){
                                sendM(id_bustia[i],missatgeE,25);
                            }
                                         
                        }
                                         
                    }else{
                        outModeC=-1;
                        caza=false;
                        sprintf(missatgeE, "%d", outModeC);	
                        for(int i =0; i<*nfActius;i++){
                            if(i!=id_fantasma){
                                sendM(id_bustia[i],missatgeE,25); 
                            }
                                         
                        }
                    }
                    
                }
                
            }
            
            win_escricar(f1.f,f1.c,f1.a,NO_INV);  /* esborra posicio anterior */            

            f1.f = seg.f; f1.c = seg.c; f1.a = seg.a;  /* actualitza posicio */
                      
            if(caza){
                win_escricar(f1.f, f1.c, buffer[0], INVERS);
            }else{
                win_escricar(f1.f, f1.c, buffer[0], NO_INV);
            }
             
            if (f1.a == '0') resultat = 1; /* ha capturat menjacocos */
        }
        //signalS(id_sem);
        if (resultat) {
            *fi2=resultat;
            break;  // Sale del bucle si ha capturado al personaje principal
        }
        
        win_retard(retardFantasma*retard);  // Retardo
    } while (*fi1==0 && *fi2==0);
  
}