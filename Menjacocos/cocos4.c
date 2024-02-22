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



#define MAX_THREADS	3
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
objecte f1[9];			    /* informacio del fantasma 1 */

int df[] = {-1, 0, 1, 0};	/* moviments de les 4 direccions possibles */
int dc[] = {0, -1, 0, 1};	/* dalt, esquerra, baix, dreta */

int cocos;			/* numero restant de cocos per menjar */
int retard;		    /* valor del retard de moviment, en mil.lisegons */
int id_fi1, *fi1, id_fi2, *fi2;
int nFantasmes=-1; //número de fantasma per guardar-ho en el vector a partir de la lectura del fitxer de joc
int numFantasmes=0; //número de fantasmes existents
int idFantasmes=1; //identificador thread fantasma
pthread_t tid[MAX_THREADS];
int numProcesos = 0; 
char missatgeE[100];

//TAULER//
char id_taulerx[10];
char n_fil1x[10];
char n_colx[10];


//FI1 i FI2//
char id_fi1c[10];
char id_fi2c[10];

//RETARD
char retardx[10];


//IDENTIFICADORS DE MEMORIA COMPARTIDA
void *p_tauler;
int id_tauler;                                             /*Variable que guarda el tamany del tauler*/

int ncocos = 0,id_ncocos;
pid_t tpid[MAXFANTASMES];		/*Creacio de la taula de processos*/

char infoTemps[64];
char infoCocos[12];
int minuts=0,segons=0,hora=0;


//NÚMERO DE XOCS (FASE 4)
int xocs=0;
char auxXocs[2];

//SEMÀFORS
int id_sem;
int id_bustias[9];
char a4[20], a5[20];

char id_mis1[25], id_mis2[25], id_mis3[25], id_mis4[25], id_mis5[25], id_mis6[25], id_mis7[25], id_mis8[25], id_mis9[25];
char numProcesosx[25];
int *nfActius;
int id_nfActius;
char nfActiusx[25];

/* funcio per realitzar la carrega dels parametres de joc emmagatzemats */
/* dins d'un fitxer de text, el nom del qual es passa per referencia a  */
/* 'nom_fit'; si es detecta algun problema, la funcio avorta l'execucio */
/* enviant un missatge per la sortida d'error i retornant el codi per-	*/
/* tinent al SO (segons comentaris al principi del programa).		    */
void carrega_parametres(const char *nom_fit)
{

  FILE *fit;
  int ret = 0; //variable per a indicar errors
  fit = fopen(nom_fit,"rt");		/* intenta obrir fitxer */
  if (fit == NULL)
  {	fprintf(stderr,"No s'ha pogut obrir el fitxer \'%s\'\n",nom_fit);
  	exit(2);
  }

  if (!feof(fit)) fscanf(fit,"%d %d %s %c\n",&n_fil1,&n_col,tauler,&c_req);
  else {
	fprintf(stderr,"Falten parametres al fitxer \'%s\'\n",nom_fit);
	fclose(fit);
	exit(2);
	}
  if ((n_fil1 < MIN_FIL) || (n_fil1 > MAX_FIL) ||
	(n_col < MIN_COL) || (n_col > MAX_COL))
  {
	fprintf(stderr,"Error: dimensions del camp de joc incorrectes:\n");
	fprintf(stderr,"\t%d =< n_fil1 (%d) =< %d\n",MIN_FIL,n_fil1,MAX_FIL);
	fprintf(stderr,"\t%d =< n_col (%d) =< %d\n",MIN_COL,n_col,MAX_COL);
	fclose(fit);
	exit(3);
  }

  if (!feof(fit)) fscanf(fit,"%d %d %d %f\n",&mc.f,&mc.c,&mc.d,&mc.r);
  else {
	fprintf(stderr,"Falten parametres al fitxer \'%s\'\n",nom_fit);
	fclose(fit);
	exit(2);
	}
  if ((mc.f < 1) || (mc.f > n_fil1-3) ||
	(mc.c < 1) || (mc.c > n_col-2) ||
	(mc.d < 0) || (mc.d > 3))
  {
	fprintf(stderr,"Error: parametres menjacocos incorrectes:\n");
	fprintf(stderr,"\t1 =< mc.f (%d) =< n_fil1-3 (%d)\n",mc.f,(n_fil1-3));
	fprintf(stderr,"\t1 =< mc.c (%d) =< n_col-2 (%d)\n",mc.c,(n_col-2));
	fprintf(stderr,"\t0 =< mc.d (%d) =< 3\n",mc.d);
	fclose(fit);
	exit(4);
  }

  int iteracio=0;
  while((ret==0) && (!feof(fit)))
	{
    iteracio=1;
    nFantasmes++;
      
    fscanf(fit,"%d %d %d %f\n",&f1[nFantasmes].f,&f1[nFantasmes].c,&f1[nFantasmes].d,&f1[nFantasmes].r);
      
      
    if ((f1[nFantasmes].f < 1) || (f1[nFantasmes].f > n_fil1-3) ||
      (f1[nFantasmes].c < 1) || (f1[nFantasmes].c > n_col-2) ||
      (f1[nFantasmes].d < 0) || (f1[nFantasmes].d > 3))
    {
      ret = 1; //error
      fprintf(stderr,"Error: parametres fantasma 1 incorrectes:\n");
      fprintf(stderr,"\t1 =< f1.f (%d) =< n_fil1-3 (%d)\n",f1[nFantasmes].f,(n_fil1-3));
      fprintf(stderr,"\t1 =< f1.c (%d) =< n_col-2 (%d)\n",f1[nFantasmes].c,(n_col-2));
      fprintf(stderr,"\t0 =< f1.d (%d) =< 3\n",f1[nFantasmes].d);
      fclose(fit);
      exit(5);
    }
    if(ret==0) 
    {
      numFantasmes++;
    }
    
    
  }

 

  if (iteracio == 0)
  {
      fprintf(stderr,"Falten parametres al fitxer \'%s\'\n",nom_fit);
      fclose(fit);
      exit(2);
  }
  else if(ret == 0)
  {
    fclose(fit);	
    printf("Joc del MenjaCocos\n\tTecles: \'%c\', \'%c\', \'%c\', \'%c\', RETURN-> sortir\n",
		  TEC_AMUNT, TEC_AVALL, TEC_DRETA, TEC_ESQUER);
    printf("prem una tecla per continuar:\n");
    getchar();
  }



   
}




/* funcio per inicialitar les variables i visualitzar l'estat inicial del joc */
void inicialitza_joc(void)
{
  int r,i,j;
  char strin[12];
  int nBucleFantasma=0;
  //int nMostraFantasma=nBucleFantasma;
  //char buffer[10];
  r = win_carregatauler(tauler,n_fil1-1,n_col,c_req);
  win_update();
  if (r == 0)
  {
    mc.a = win_quincar(mc.f,mc.c);
    if (mc.a == c_req) r = -6;		/* error: menjacocos sobre pared */
    else
    {
      while((nBucleFantasma<numFantasmes))  //si hi ha algún error, s'abortarà amb l'inicialització del joc
      {
        f1[nBucleFantasma].a = win_quincar(f1[nBucleFantasma].f,f1[nBucleFantasma].c);
        if (f1[nBucleFantasma].a == c_req) r = -7;	/* error: fantasma sobre pared */
        else
        {
          cocos = 0;			/* compta el numero total de cocos */
          for (i=0; i<n_fil1-1; i++)
            for (j=0; j<n_col; j++)
              if (win_quincar(i,j)=='.') cocos++;
        
          win_escricar(mc.f,mc.c,'0',NO_INV);
          win_update();
        }
        nBucleFantasma++;
      }
    
      if (mc.a == '.') cocos--;	/* menja primer coco */
	    
      sprintf(strin,"Cocos: %d", cocos); 
      win_escristr(strin);
    }
  }
  
  if (r != 0)
  {	win_fi();
	fprintf(stderr,"Error: no s'ha pogut inicialitzar el joc:\n");
	switch (r)
	{ case -1: fprintf(stderr,"  nom de fitxer erroni\n"); break;
	  case -2: fprintf(stderr,"  numero de columnes d'alguna fila no coincideix amb l'amplada del tauler de joc\n"); break;
	  case -3: fprintf(stderr,"  numero de columnes del laberint incorrecte\n"); break;
	  case -4: fprintf(stderr,"  numero de files del laberint incorrecte\n"); break;
	  case -5: fprintf(stderr,"  finestra de camp de joc no oberta\n"); break;
	  case -6: fprintf(stderr,"  posicio inicial del menjacocos damunt la pared del laberint\n"); break;
	  case -7: fprintf(stderr,"  posicio inicial del fantasma damunt la pared del laberint\n"); break;
	}
	exit(7);
  }
}


/* funcio per moure el menjacocos una posicio, en funcio de la direccio de   */
/* moviment actual; retorna -1 si s'ha premut RETURN, 1 si s'ha menjat tots  */
/* els cocos, i 0 altrament */
void * mou_menjacocos(void * null)
{
  //char strin[12];
  objecte seg;
  int tecla, resultat;
  int retardMenjacocos;


  do{
    retardMenjacocos=mc.r;
    resultat=0;
    tecla = win_gettec();
    if (tecla != 0)
    switch (tecla)		/* modificar direccio menjacocos segons tecla */
    {
      case TEC_AMUNT:	  mc.d = 0; break;
      case TEC_ESQUER:  mc.d = 1; break;
      case TEC_AVALL:	  mc.d = 2; break;
      case TEC_DRETA:	  mc.d = 3; break;
      case TEC_RETURN:  resultat = -1; break;
    }
    seg.f = mc.f + df[mc.d];	/* calcular seguent posicio */
    seg.c = mc.c + dc[mc.d];
    seg.a = win_quincar(seg.f,seg.c);	/* calcular caracter seguent posicio */
    if ((seg.a == ' ') || (seg.a == '.') || (seg.a >= '1' && seg.a <= '9'))
    {
     
      win_escricar(mc.f,mc.c,' ',NO_INV);		/* esborra posicio anterior */
      mc.f = seg.f; mc.c = seg.c;			/* actualitza posicio */
      win_escricar(mc.f,mc.c,'0',NO_INV);		/* redibuixa menjacocos */
      if (seg.a == '.')
      {
        cocos--;
        //sprintf(strin,"Cocos: %d", cocos);  
        //win_escristr(strin);
     
        if (cocos == 0) resultat = 1; //guanya l'usuari (1)
      }
      if(seg.a >= '1' && seg.a <= '9'){
        *fi2 = 1;
      }
    }
    else if (xocs<=2)
	{			
			xocs++;					
	}
    if ((xocs==2) && (numProcesos<numFantasmes))
    {
      tpid[numProcesos]=fork();
			if(tpid[numProcesos]==(pid_t)0)
			{
        sprintf(id_fi1c,"%i",id_fi1);
        sprintf(id_fi2c,"%i",id_fi2);
        sprintf(numProcesosx,"%i",numProcesos);
        //sprintf(a4, "%i", id_bustias[numProcesos]);
        sprintf(missatgeE, "%d;%d;%d;%f", f1[numProcesos].f, f1[numProcesos].c,f1[numProcesos].d,f1[numProcesos].r);						
        sendM(id_bustias[numProcesos],missatgeE,100);
        //*nfActius++;				/* inicialitza variable compartida */
        *nfActius=*nfActius+1;
   
        execlp("./fantasma4", "fantasma4", id_taulerx, n_fil1x, n_colx, id_fi1c, id_fi2c, retardx, a5, numProcesosx, id_mis1, id_mis2, id_mis3, id_mis4, id_mis5, id_mis6, id_mis7, id_mis8, id_mis9, nfActiusx, (char *)0);
				
        exit(0);
			}
			else if (tpid[numProcesos] > 0) numProcesos++;

 
		  xocs=0;	
    }
    
    *fi1=resultat;
     win_retard(retardMenjacocos*retard);  // retard

  }while(*fi1==0 && *fi2==0);
  return (NULL);
}


void * cronometre(void * index)
{	
	
	bool acaba=false;
	do{
		if(segons==60){
			segons=0;
			minuts++;
			if(minuts==60){
				minuts=0;
				hora++;
			}       
    }
    
    
	  //win_escristr(infoTemps);
	  win_retard(1000);
    segons++;
	  if(((*fi1)==1)||((*fi2)==1)){
		  acaba=true;      
		  //fprintf(stderr, "fi1:%i  \t fi2:%i",*fi1,*fi2);
	  }
	}while(acaba==false);
  return (NULL);

}



void* info(void* nul)
{
	char info[50];
  int auxCocos=0;
  int auxHores=0;
  int auxMinuts=0;
  int auxSegons=0;
	while(!*fi1 && !*fi2)
	{ 
      if((auxCocos!=cocos) || (auxHores!=hora) || (auxMinuts!=minuts) || (auxSegons!=segons)){
          info[0]='\0'; 
          sprintf(infoTemps,"Temps %i:%i:%i",hora,minuts,segons);  
          sprintf(infoCocos,"Cocos: %d", cocos); 								
          strcat(info, infoTemps);
          strcat(info, " - ");				
          strcat(info, infoCocos);
          
          win_escristr(info);
          auxCocos=cocos;
          auxHores=hora;
          auxMinuts=minuts;
          auxSegons=segons;
      }
				
     	
			//win_retard(retard);
  }
  return NULL;
}







/* programa principal				    */
int main(int n_args, const char *ll_args[])
{
  int rc;		/* variables locals */
  id_sem = ini_sem(1);    /* crear semafor IPC inicialment obert */    
  sprintf(a5,"%i",id_sem);    /* convertir identificador sem. en string */
   

  if ((n_args != 2) && (n_args !=3))
  {	fprintf(stderr,"Comanda: cocos0 fit_param [retard]\n");
  	exit(1);
  }
  carrega_parametres(ll_args[1]);

  if (n_args == 3) retard = atoi(ll_args[2]);
  else retard = 100;

  rc = win_ini(&n_fil1,&n_col,'+',INVERS);	/* intenta crear taulell */
  if (rc != 0)		/* si aconsegueix accedir a l'entorn CURSES */
  {
    //TAULER   
    id_tauler = ini_mem(rc);   //crear zona de memòria compartida amb els bytes que ocupa el mapa del joc
   	p_tauler = map_mem(id_tauler); //estableix un identificador
    //set_filcol(p_tauler,n_fil1, n_col);
    win_set(p_tauler,n_fil1,n_col);
    
    inicialitza_joc();
    
  	
    sprintf(id_taulerx, "%d", id_tauler);      							
		sprintf(n_fil1x, "%d", n_fil1);
		sprintf(n_colx, "%d", n_col);

  
    
    id_fi1 = ini_mem(sizeof(int));	/* crear zona mem. compartida */
  	fi1 = map_mem(id_fi1);		/* obtenir adres. de mem. compartida */
  	*fi1 = 0;				/* inicialitza variable compartida */
  	sprintf(id_fi1c,"%i",id_fi1);

  	id_fi2 = ini_mem(sizeof(int));	/* crear zona mem. compartida */
  	fi2 = map_mem(id_fi2);		/* obtenir adres. de mem. compartida */
  	*fi2 = 0;
  	sprintf(id_fi2c,"%i",id_fi2);
   
   
    sprintf(retardx, "%d", retard);

    
    id_nfActius = ini_mem(sizeof(int));	/* crear zona mem. compartida */
  	nfActius = map_mem(id_nfActius);		/* obtenir adres. de mem. compartida */
  	*nfActius = 1;				/* inicialitza variable compartida */
    sprintf(nfActiusx, "%d", id_nfActius);

    for (int i = 0; i < 9 ; i++)
	  {
		  id_bustias[i] = ini_mis();	    /* crear una bustia IPC per cada fantasma*/
		  //fprintf(stderr, "\nPrimer: %i", id_mis[i]);
	  }
    
    sprintf (id_mis1, "%i", id_bustias[0]);
    sprintf (id_mis2, "%i", id_bustias[1]);
    sprintf (id_mis3, "%i", id_bustias[2]);
    sprintf (id_mis4, "%i", id_bustias[3]);
    sprintf (id_mis5, "%i", id_bustias[4]);
    sprintf (id_mis6, "%i", id_bustias[5]);
    sprintf (id_mis7, "%i", id_bustias[6]);
    sprintf (id_mis8, "%i", id_bustias[7]);
    sprintf (id_mis9, "%i", id_bustias[8]);	
  			/********** bucle principal del joc **********/
    
    pthread_create(&tid[0], NULL, mou_menjacocos, (void *)(intptr_t)0);

    pthread_create(&tid[1], NULL, cronometre, (void *)(intptr_t)0);

    pthread_create(&tid[2], NULL, info, (void *)(intptr_t)0);


    //pthread_create(&tid[1], NULL, cronometre, (void *)(intptr_t)0);
    
    //FANTASMES// 

    
    
			tpid[numProcesos]=fork();   /* crea un nou proces (fill)*/ 
			if(tpid[numProcesos]==(pid_t)0)
			{
        
        
        //id_bustias[numProcesos] = ini_mis();    /* crear bustia IPC */    
        //sprintf(a4,"%i",id_bustias[numProcesos]);  /* convertir identif. bustia en string */
        sprintf(numProcesosx,"%i",numProcesos);

        sprintf(missatgeE, "%d;%d;%d;%f", f1[numProcesos].f, f1[numProcesos].c,f1[numProcesos].d,f1[numProcesos].r);						
        sendM(id_bustias[numProcesos],missatgeE,100);
        
			 	execlp("./fantasma4", "fantasma4", id_taulerx, n_fil1x, n_colx, id_fi1c, id_fi2c, retardx, a5, numProcesosx, id_mis1, id_mis2, id_mis3, id_mis4, id_mis5, id_mis6, id_mis7, id_mis8, id_mis9, nfActiusx,(char *)0);
        exit(0);
			}	
			else if (tpid[numProcesos] > 0) numProcesos++;
			
		

  
    do
    {
      win_retard(retard);
      win_update();
     
    }while(!*fi1 && !*fi2);

    

    for(int j = 0; j < 1; j++)                                      /*finalizacion de threads*/
  	{
    		pthread_join(tid[j], NULL);
  	}
	

  	for (int i = 0; i <= numProcesos; i++){      
    		waitpid(tpid[i], NULL, 0);      /* espera finalitzacio d'un fill */
  	}
  
    

    win_fi();

    elim_mem(id_tauler);     /* elimina zones de memoria compartida */
	  elim_mem(id_fi1);
	  elim_mem(id_fi2);
    elim_sem(id_sem);
    elim_mem(id_nfActius);
    
    if (*fi1 == -1) printf("S'ha aturat el joc amb tecla RETURN!\n");
    else { if (*fi1) printf("Ha guanyat l'usuari!\n");
      else printf("Ha guanyat l'ordinador!\n"); }


  }
  else
  { 
    fprintf(stderr,"Error: no s'ha pogut crear el taulell:\n");
	  switch (rc)
    { case -1: fprintf(stderr,"camp de joc ja creat!\n");
        break;
      case -2: fprintf(stderr,"no s'ha pogut inicialitzar l'entorn de curses!\n");
        break;
      case -3: fprintf(stderr,"les mides del camp demanades son massa grans!\n");
        break;
      case -4: fprintf(stderr,"no s'ha pogut crear la finestra!\n");
        break;
    }
	  exit(6);
  }
  return(0);
}