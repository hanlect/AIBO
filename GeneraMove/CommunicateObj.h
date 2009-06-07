#ifndef COMMUNICATE_OBJ_H_DEFINED
#define COMMUNICATE_OBJ_H_DEFINED
#include <OPENR/OFbkImage.h>
#include "R_list.h"

#define PI 3.14159265
#define alt 160		//la y dell'immagine
#define lar 208		//la x dell'immagine
#define al alt-1	//per partire dal pixel non di bordo dell'immagine = 159
#define la lar-1	//sono le coordinate cui sopra -1 = 207

/*elemento della matrice matr*/
			typedef struct punto {
				 unsigned int val_col;
					unsigned int regione;
			};

			/*struttura per analisi dei pixel*/
			typedef struct colore {
			  unsigned int tipo;
	  		unsigned int occorrenza;
	  };

			/*-----------------------------------------------------------------*/
			/*struttura per elementi della coda di memorizzazione delle regioni*/
			/*-----------------------------------------------------------------*/

			typedef struct per_elem {			//elemento della coda per i pixel del perimetro della regione
				 unsigned int coord_x;				//coordinata x
					unsigned int coord_y;				//coordinata y
					struct per_elem* next;				//puntatore al prox elemento della coda 
   };

			typedef struct per_reg {			//per lista dei pixel del perimetro
				 struct per_elem* first;				//puntatore al primo elemento della coda perimetro
				 struct per_elem* last;				//puntatore all'ultimmo elemento della coda perimetro
			};
			
			typedef struct queue {				//Coda delle regioni
			  int cnt;						//contatore del numero di elementi
					struct region* front;			//puntatore al primo elemento
			};

			/*struttura per l'invio di messaggi tra AISIdentify e AtkAction*/
			typedef struct Snap {
			  region* o;	
					OFbkImageInfo* info ;
			};
			
			
namespace Com_Obj {
  /** Tipi di azioni fa far eseguire all'attaccante.*/
  enum Atk_Action{
				IDLE,			
    NOTHING,
    FINDBALL,
    TRACKINGBALL,
    GOBALL,
    FINDGOAL,
    SHOOT
  };
  
  /** Tipi di oggetti su cui eseguire azioni.*/
  enum ObjToDo{
    BALL,
    MYGOAL,
    OTHERGOAL,
    LINE
  };
  
  /** Comandi da far eseguire a Occipital.*/
  struct AtkCommand{
    Atk_Action _command; /**< Azione da compiere.*/
  };


  /** Informazioni sulla posizione dell'oggetto da AISIdentify a AISTracking.*/
  struct ObjLocation{
				ObjToDo _object; /**oggetto su cui stiamo tracciando la posizione*/ 					
    double _hour; /**< Posizione espressa in ore rispetto al
		     corpo. (p.es BALL a ore 12: la palla e' di fronte
		     alla testa.)*/
    double _distance; /**< Distanza dell'oggetto dal muso dell'Aibo
			 (in mm). Un valore pari a -1 significa che
			 non e' possibile definire la distanza
			 dell'oggetto.*/
  };
  
  /** Informazioni sulla posizione dell'oggetto da AISTracking a BallAction.*/
  struct ObjTracking{
				ObjToDo _object; /**oggetto su cui stiamo tracciando la posizione*/ 										
    double _hour; /**< Posizione espressa in ore rispetto al
		     corpo. (p.es BALL a ore 12: la palla e' di fronte
		     alla testa.)*/
    double _distance; /**< Distanza dell'oggetto dal muso dell'Aibo
			 (in mm). Un valore pari a -1 significa che
			 non e' possibile definire la distanza
			 dell'oggetto.*/
  };
  
} 
#endif
