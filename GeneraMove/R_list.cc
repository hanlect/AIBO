#include <stdio.h>
#include "R_list.h"

/* crea una lista vuota e ne restituisce il puntatore radice */
R_list *createlist(void)
{
    return NULL;
}

/* visita una lista e esegue su ogni elemento la funzione op */
void traverse(R_list *p, void (*op)(R_list *))
{
	R_list *q;

	for(q = p; q; q = q->next)
		(*op)(q);
}

/* stampa l'elemento puntato */
void printelem(R_list *q)
{
	printf("ELEM:%d -|- color:%d -|- x:%f -|- y:%f\n",q->rgn.id, q->rgn.color, q->rgn.centroid_x, q->rgn.centroid_y);
	printf("ELEM:%d -|- minx:%d -|- maxx:%d -|- miny:%d -|- maxy:%d\n",
	       q->rgn.id, q->rgn.min_x, q->rgn.max_x, q->rgn.min_y, q->rgn.max_y );
 printf("ELEM:%d -|- fristX:%d -|- fristY:%d\n",q->rgn.id, q->rgn.fpixel_coord_x, q->rgn.fpixel_coord_y);      
 printf("\n");
}

/* ritorna il numero di elementi nella lista */
int countlist(R_list *p)
{
	int i;
	
	for(i = 0; p; p = p->next, i++);
	return i;
}

/* inserisce un elemento in testa alla lista */  
R_list *insertR(R_list *p,  int numid, unsigned int col, unsigned int frist_x, unsigned int frist_y, double	peri,
 double cent_x, double cent_y, double circo, unsigned int are, int ma_y,
	int mi_y,	int ma_x,	int mi_x,	bool gr,	bool wh)
{
	R_list *q =(R_list *) malloc(sizeof(R_list));

	if(!q) {
		fprintf(stderr,"Errore nell'allocazione del nuovo elemento\n");
		exit(-1);	
	};
	q->rgn.id= numid;
	q->rgn.color= col;
	q->rgn.fpixel_coord_x= frist_x;
	q->rgn.fpixel_coord_y= frist_y;
	q->rgn.area= are;
	q->rgn.max_y= ma_y;
 q->rgn.max_x= ma_x;
	q->rgn.min_y= mi_y;
	q->rgn.min_x= mi_x;
	q->rgn.centroid_x= cent_x;
	q->rgn.centroid_y= cent_y;
	q->rgn.circ= circo;
 q->rgn.perimeter= peri;
	q->rgn.green= gr;
 q->rgn.white= wh;
	q->next = p;
	return q; 
}

/* cancella l'elemento puntato da q dalla lista */
R_list *deleteR(R_list *p, int q)
{
	R_list *r;
	R_list *del;
	
 if(p == NULL)
   return NULL;
   
	if(p->rgn.id == q) {
		del=p;
		p = p->next;
		del->next= NULL;												
		free(del);
	} else {
		for(r = p; r && r->next->rgn.id != q; r = r->next);
		if(r && r->next->rgn.id == q) {
			del= r->next;			
			r->next = r->next->next;
			del->next= NULL;
			free(del);
		}
	}
	return p;
}

/* distrugge la lista */
R_list *destroylist(R_list *p)
{
	R_list *l;
	while(p){
	  l= p;
	  p=p->next;
	  free(l);
	}
	free(p);
	
	return p;
}

/* concatena la lista con radice q alla lista con radice p */
R_list *listcat(R_list *p, R_list *q)
{
	R_list *r;
	
	if(!p)
		return q;
	for(r = p; r->next; r = r->next);
	r->next = q;
	return p;
}

/* restituisce vero se la funzione check e' vera su almeno un elemento */
char traverseandcheck(R_list *p, char (*check)(R_list *, int), int a)
{
	R_list *q;

	for(q = p; q; q = q->next)
		if((*check)(q,a))
			return 1;
	return 0;
}

/* restituisce il primo elemento per cui check e' vera oppure NULL*/
R_list *getcheckelem(R_list *p, char (*check)(R_list *, int), int a)
{
	R_list *q;

	for(q = p; q; q = q->next)
		if((*check)(q,a))
			return q;
	return NULL;
}

/* restituisce vero se p->dato == elem */ 
char checkexist(R_list *p, int elem)
{
	return p->rgn.id == elem;
}

/* restituisce vero se l'elemento e' presente nella lista */
char existsinlist(R_list *p, int elem)
{
	return traverseandcheck(p,checkexist,elem);
}

/* restituisce il primo elemento q per cui q->dato == elem */
R_list *geteleminlist(R_list *p, int elem)
{
	return getcheckelem(p,checkexist,elem);
}

/* Versioni Ricorsive */

/* ritorna il numero di elementi nella lista: versione ricorsiva */
int rcountlist(R_list *p)
{
    return p ? rcountlist(p->next) + 1 : 0;
}

/* visita una lista e esegue su ogni elemento la funzione op: versione ricorsiva */
void rtraverse(R_list *p, void (*op)(R_list *))
{
    if(p) {
        (*op)(p);
        rtraverse(p->next,op);
    }
}

/* concatena la lista con radice q alla lista con radice p: versione ricorsiva */
R_list *rlistcat(R_list *p, R_list *q)
{
    if(p) {
        p->next = rlistcat(p->next,q);
    }
    return p ? p : q;
}
