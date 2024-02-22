/*****************************************************************************/
/*									                                         */
/*				     cocos1.c				                                 */
/*									                                         */
/*     Programa inicial d'exemple per a les practiques 2.1 i 2.2 de FSO.     */
/*     Es tracta del joc del menjacocos: es dibuixa un laberint amb una      */
/*     serie de punts (cocos), els quals han de ser "menjats" pel menja-     */
/*     cocos. Aquest menjacocos es representara amb el caracter '0', i el    */
/*     moura l'usuari amb les tecles 'w' (adalt), 's' (abaix), 'd' (dreta)   */
/*     i 'a' (esquerra). Simultaniament hi haura un conjunt de fantasmes,    */
/*     representats per numeros de l'1 al 9, que intentaran capturar al      */
/*     menjacocos. En la primera versio del programa, nomes hi ha un fan-    */
/*     tasma.								                                 */
/*     Evidentment, es tracta de menjar tots els punts abans que algun fan-  */
/*     tasma atrapi al menjacocos.					                         */
/*									                                         */
/*  Arguments del programa:						                             */
/*     per controlar la posicio de tots els elements del joc, cal indicar    */
/*     el nom d'un fitxer de text que contindra la seguent informacio:	     */
/*		n_fil1 n_col fit_tauler creq				                         */
/*		mc_f mc_c mc_d mc_r						                             */
/*		f1_f f1_c f1_d f1_r						                             */
/*									                                         */
/*     on 'n_fil1', 'n_col' son les dimensions del taulell de joc, mes una   */
/*     fila pels missatges de text a l'ultima linia. "fit_tauler" es el nom  */
/*     d'un fitxer de text que contindra el dibuix del laberint, amb num. de */
/*     files igual a 'n_fil1'-1 i num. de columnes igual a 'n_col'. Dins     */
/*     d'aquest fitxer, hi hauran caracter ASCCII que es representaran en    */
/*     pantalla tal qual, excepte el caracters iguals a 'creq', que es visua-*/
/*     litzaran invertits per representar la paret.			                 */
/*     Els parametres 'mc_f', 'mc_c' indiquen la posicio inicial de fila i   */
/*     columna del menjacocos, aixi com la direccio inicial de moviment      */
/*     (0 -> amunt, 1-> esquerra, 2-> avall, 3-> dreta). Els parametres	     */
/*     'f1_f', 'f1_c' i 'f1_d' corresponen a la mateixa informacio per al    */
/*     fantasma 1. El programa verifica que la primera posicio del menja-    */
/*     cocos o del fantasma no coincideixi amb un bloc de paret del laberint.*/
/*	   'mc_r' 'f1_r' son dos reals que multipliquen el retard del moviment.  */ 
/*     A mes, es podra afegir un segon argument opcional per indicar el      */
/*     retard de moviment del menjacocos i dels fantasmes (en ms);           */
/*     el valor per defecte d'aquest parametre es 100 (1 decima de segon).   */
/*									                                         */
/*  Compilar i executar:					  	                             */
/*     El programa invoca les funcions definides a 'winsuport.h', les        */
/*     quals proporcionen una interficie senzilla per crear una finestra     */
/*     de text on es poden escriure caracters en posicions especifiques de   */
/*     la pantalla (basada en CURSES); per tant, el programa necessita ser   */
/*     compilat amb la llibreria 'curses':				                     */
/*									                                         */
/*	   $ gcc -Wall cocos0.c winsuport.o -o cocos0 -lcurses		             */
/*	   $ ./cocos0 fit_param [retard]				                         */
/*									                                         */
/*  Codis de retorn:						  	                             */
/*     El programa retorna algun dels seguents codis al SO:		             */
/*	0  ==>  funcionament normal					                             */
/*	1  ==>  numero d'arguments incorrecte 				                     */
/*	2  ==>  fitxer de configuracio no accessible			                 */
/*	3  ==>  dimensions del taulell incorrectes			                     */
/*	4  ==>  parametres del menjacocos incorrectes			                 */
/*	5  ==>  parametres d'algun fantasma incorrectes			                 */
/*	6  ==>  no s'ha pogut crear el camp de joc			                     */
/*	7  ==>  no s'ha pogut inicialitzar el joc			                     */
/*****************************************************************************/



#include <stdio.h>		/* incloure definicions de funcions estandard */
#include <stdlib.h>		/* per exit() */
#include <unistd.h>		/* per getpid() */
#include "winsuport.h"		/* incloure definicions de funcions propies */
#include <pthread.h>
#include <stdbool.h>
#include <string.h>

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
objecte f1[9];			    /* informacio del fantasma 1 */

int df[] = {-1, 0, 1, 0};	/* moviments de les 4 direccions possibles */
int dc[] = {0, -1, 0, 1};	/* dalt, esquerra, baix, dreta */

int cocos;			/* numero restant de cocos per menjar */
int retard;		    /* valor del retard de moviment, en mil.lisegons */
int fi1=0, fi2=0;
int nFantasmes=-1; //número de fantasma per guardar-ho en el vector a partir de la lectura del fitxer de joc
int numFantasmes=0; //número de fantasmes existents
int idFantasmes=1; //identificador thread fantasma
pthread_t tid[MAX_THREADS];
pthread_mutex_t fantasmamutex= PTHREAD_MUTEX_INITIALIZER;   /* crea un sem. Global*/
char infoTemps[64];
char infoCocos[12];
int minuts=0,segons=0,hora=0;

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
  //char strin[12];
  int nBucleFantasma=0;
  int nMostraFantasma=nBucleFantasma;
  char buffer[10];
  r = win_carregatauler(tauler,n_fil1-1,n_col,c_req);
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
          nMostraFantasma=nBucleFantasma+1;
          sprintf(buffer, "%d", nMostraFantasma);
          win_escricar(f1[nBucleFantasma].f,f1[nBucleFantasma].c,buffer[0], NO_INV);
        }
        nBucleFantasma++;
      }
      if (mc.a == '.') cocos--;	/* menja primer coco */
	    
      sprintf(infoCocos,"Cocos: %d", cocos); //win_escristr(strin);
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




void * mou_fantasma(void * index)
{
  int ind = (intptr_t) index;
  objecte seg;
  int resultat;
  int k, vk, nd, vd[3];
  int retardFantasma;
  int nMostraFantasma=ind;
  char buffer[10];
  
  do {  // Bucle propio de la función
    resultat = 0; nd = 0;
    retardFantasma=f1[ind].r;  // Retardo en función del índice
    pthread_mutex_lock(&fantasmamutex);
    for (k=-1; k<=1; k++) {   /* provar direccio actual i dir. veines */
      vk = (f1[ind].d + k) % 4; /* direccio veina */
      if (vk < 0) vk += 4;  /* corregeix negatius */
      seg.f = f1[ind].f + df[vk]; /* calcular posicio en la nova dir.*/
      seg.c = f1[ind].c + dc[vk];
      
      seg.a = win_quincar(seg.f,seg.c); /* calcular caracter seguent posicio */
      
      if ((seg.a==' ') || (seg.a=='.') || (seg.a=='0')) {
        vd[nd] = vk;  /* memoritza com a direccio possible */
        nd++;
      }
    }
    if (nd == 0) {  /* si no pot continuar, */
      f1[ind].d = (f1[ind].d + 2) % 4;  /* canvia totalment de sentit */
    } else {
      if (nd == 1) {  /* si nomes pot en una direccio */
        f1[ind].d = vd[0];  /* li assigna aquesta */
      } else {  /* altrament */
        f1[ind].d = vd[rand() % nd];  /* segueix una dir. aleatoria */
      }
  
      seg.f = f1[ind].f + df[f1[ind].d];  /* calcular seguent posicio final */
      seg.c = f1[ind].c + dc[f1[ind].d];
     
      seg.a = win_quincar(seg.f,seg.c); /* calcular caracter seguent posicio */
    
      win_escricar(f1[ind].f,f1[ind].c,f1[ind].a,NO_INV);  /* esborra posicio anterior */ 
      

      f1[ind].f = seg.f; f1[ind].c = seg.c; f1[ind].a = seg.a;  /* actualitza posicio */
      
      nMostraFantasma=ind+1;
      sprintf(buffer, "%d", nMostraFantasma);
      
      win_escricar(f1[ind].f,f1[ind].c,buffer[0],NO_INV);  /* redibuixa fantasma */
      
      
      if (f1[ind].a == '0') resultat = 1; /* ha capturat menjacocos */
    }
    
    pthread_mutex_unlock(&fantasmamutex);
    
    if (resultat) {
      fi2=resultat;
     
      break;  // Sale del bucle si ha capturado al personaje principal
    }
    win_retard(retardFantasma*retard);  // Retardo
  } while (!fi1 && !fi2);
  
  return (NULL);
}




/* funcio per moure el menjacocos una posicio, en funcio de la direccio de   */
/* moviment actual; retorna -1 si s'ha premut RETURN, 1 si s'ha menjat tots  */
/* els cocos, i 0 altrament */
void * mou_menjacocos(void * null)
{
  
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
      pthread_mutex_lock(&fantasmamutex);
      win_escricar(mc.f,mc.c,' ',NO_INV);		/* esborra posicio anterior */
      mc.f = seg.f; mc.c = seg.c;			/* actualitza posicio */
      win_escricar(mc.f,mc.c,'0',NO_INV);		/* redibuixa menjacocos */
      pthread_mutex_unlock(&fantasmamutex);
      if(seg.a >= '1' && seg.a <= '9'){
        fi2 = 1;
      }
      if (seg.a == '.')
      {
        cocos--;
          //win_escristr(strin);
        if (cocos == 0) resultat = 1; //guanya l'usuari (1)
      }
    }
    
    fi1=resultat;
     win_retard(retardMenjacocos*retard);  // retard

  }while(fi1==0 && fi2==0);
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
	  if(((fi1)==1)||((fi2)==1)){
		  acaba=true;      
		  //fprintf(stderr, "fi1:%i  \t fi2:%i",fi1,fi2);
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
	while(!fi1 && !fi2)
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
  return (NULL);
}




/* programa principal				    */
int main(int n_args, const char *ll_args[])
{
  int rc;		/* variables locals */

  //int p; //retard fantasma
  //srand(getpid());		/* inicialitza numeros aleatoris */

  if ((n_args != 2) && (n_args !=3))
  {	fprintf(stderr,"Comanda: cocos0 fit_param [retard]\n");
  	exit(1);
  }
  carrega_parametres(ll_args[1]);

  if (n_args == 3) retard = atoi(ll_args[2]);
  else retard = 100;

  rc = win_ini(&n_fil1,&n_col,'+',INVERS);	/* intenta crear taulell */
  if (rc == 0)		/* si aconsegueix accedir a l'entorn CURSES */
  {
  
    inicialitza_joc();
    fi1 = 0; fi2 = 0;
   
  			/********** bucle principal del joc **********/
    
    pthread_create(&tid[0], NULL, mou_menjacocos, (void *)(intptr_t)0);

    for (int i = 0; i < numFantasmes; i++) {
      pthread_create(&tid[i+1], NULL, mou_fantasma, (void *)(intptr_t)i);
    }

    pthread_create(&tid[10], NULL, cronometre, (void *)(intptr_t)0);

    pthread_create(&tid[10], NULL, info, (void *)(intptr_t)0);
    //p++; if ((p%2)==0)		/* ralentitza fantasma a 2*retard */
    
    

   
    do
    {
      win_retard(100);  
    }while(!fi1 && !fi2);



      for(int j = 0; j < MAX_THREADS; j++)                                      /*finalizacion de threads*/
      {
        pthread_join(tid[j], NULL);
      }

      pthread_mutex_destroy(&fantasmamutex);
      win_fi();
      if (fi1 == -1) printf("S'ha aturat el joc amb tecla RETURN!\n");
      else { if (fi1) printf("Ha guanyat l'usuari!\n");
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