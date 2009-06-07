#ifndef R_LIST_H
#define R_LIST_H
#include <stdlib.h>

typedef struct region {				//Elemento della coda di memorizzazione delle regioni
	    int id;					//identifictivo della regione
	    unsigned int color;					//tipo di colore presente
	    unsigned int fpixel_coord_x;		//coordinata x del primo pixel
	    unsigned int fpixel_coord_y;		//cordinata y del primo pixel
	    double	perimeter;					//lungh. del perimetro
	    double centroid_x;					//coordinata x del centroide
     double centroid_y;					//coordinata y del centroide
	    double circ;						//fattore di circolarità (distingue il cerchio dal quadrato
					unsigned int area;					//area della regione
					/*servono per il confronto con la riga dell'orizzonte*/
					unsigned int high_y;				//coordinata y del pixel + alto
					unsigned int low_y;					//coordinata y del pixel + basso
					int max_y;							//coordinate maggiori e minori dei pixel della regione
					int min_y;
					int max_x;
					int min_x;
					bool green;					//è a 1 se la regione confina con pixel verdi
					bool white;					//è a 1 se la regione confina con pixel bianchi  
			};
						
struct R_list {
    region rgn;
    struct R_list *next;
};

typedef struct R_list R_list;

/* crea una lista vuota e ne restituisce il puntatore radice */
R_list *createlist(void);

/* visita una lista e esegue su ogni elemento la funzione op */
void traverse(R_list *p, void (*op)(R_list *));

/* stampa l'elemento puntato */
void printelem(R_list *q);

/* ritorna il numero di elementi nella lista */
int countlist(R_list *p);

/* inserisce un elemento in testa alla lista */
R_list *insertR(R_list *p,  int numid, unsigned int col, unsigned int frist_x, unsigned int frist_y, double	peri,double cent_x, double cent_y, double circo, unsigned int are, int ma_y,int mi_y,	int ma_x,	int mi_x,	bool gr,	bool wh);

/* cancella l'elemento puntato da q dalla lista */
R_list *deleteR(R_list *p, int q);

/* distrugge la lista */
R_list *destroylist(R_list *p);

/* concatena la lista con radice q alla lista con radice p: non testato */
R_list *listcat(R_list *p, R_list *q);

/* restituisce vero se la funzione check e' vera su almeno un elemento:non testato */
char traverseandcheck(R_list *p, char (*check)(R_list *, int), int a);

/* restituisce il primo elemento per cui check e' vera oppure NULL: non testato*/
R_list *getcheckelem(R_list *p, char(*check)(R_list *, int), int a);

/* restituisce vero se p->dato == elem:non testato */ 
char checkexist(R_list *p, int elem);

/* restituisce vero se l'elemento e' presente nella lista: non testato */
char existsinlist(R_list *p, int elem);

/* restituisce il primo elemento q per cui q->dato == elem: non testato */
R_list *geteleminlist(R_list *p, int elem);

/* ritorna il numero di elementi nella lista: versione ricorsiva: non testato */
int rcountlist(R_list *p);

/* visita una lista e esegue su ogni elemento la funzione op: versione ricorsiva: non testato */
void rtraverse(R_list *p, void (*op)(R_list *));

/* concatena la lista con radice q alla lista con radice p: versione ricorsiva non testato*/
R_list *rlistcat(R_list *p, R_list *q);

#endif
