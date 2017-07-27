#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include<stdint.h>
#define MAX 255
#define MAX_FIGLI 1024
#define DIM 1048576

typedef struct t
{
    bool tipo;     //tipo=0 dir; tipo=1 file
    char *dir;     //directory
    char *stringa; //contenuto file
    struct t* fratello;
    struct t* figlio;
    short int altezza;
} nodo;

typedef struct n
{
    nodo *corrente;
    struct n *next;
} nodol;




void init();                 //inizializza root e hash table
nodo* creael();              //alloca un elemento generico (tipo directory di default)
char *getinput(int size);    //ritorna un puntatore alla stringa acquisita da stdin
uint32_t jenkins_one_at_a_time_hash(const uint8_t* key, size_t length);
nodo* find(char *str);       //ritorna puntatore al nodo cercato se esiste nella hash, NULL altrimenti
void create(char *curr, char* input); //crea file o directory
void insert(nodo* nnew);      //inserisce new nella hashmap
void read(char *curr);        //legge il contenuto della stringa contenuta nel file
void write(char *curr);       //sovrascrive il contenuto della stringa nel file
int deleted(char *curr);      //elimina il nodo
void hashdel(nodo* old);      //cancella old dalla hashmap
void deleted_r(char *curr);
void findn(char *curr);
int cstring_cmp(const void *a, const void *b);


nodo* root;
nodo** hash;                                  //NON RIALLOCO MAI L HASH TABLE

int main()                                    //NON CONTROLLO CHE I NOMI SIANO ALFANUMERICI DI MAX 255 CARATTERI
{
    char *input;
    char *curr;  //stringa corrente dopo strtok
    int i;

    init();

    while(1)
    {
        input=getinput(MAX);
        curr=strtok (input," ");

        if(!strcmp(curr,"create_dir")||!strcmp(curr,"create"))
        {
            curr=strtok (NULL," "); //curr diventa la directory
            create(curr, input);
        }
        else if(!strcmp(curr,"read"))
        {
            curr=strtok (NULL," ");
            read(curr);
        }
        else if(!strcmp(curr,"write"))
        {
            curr=strtok (NULL," ");
            write(curr);
        }
        else if(!strcmp(curr,"delete"))
        {
            curr=strtok (NULL," ");
            i=deleted(curr);
            if (i) printf("ok\n");
            else printf("no\n");
        }
        else if(!strcmp(curr,"delete_r"))
        {
            curr=strtok (NULL," ");
            deleted_r(curr);
        }
        else if(!strcmp(curr,"find"))
        {
            curr=strtok (NULL," ");
            findn(curr);
        }
        else if(!strcmp(curr,"exit")) return 0;
        free(input);
    }
}

void hashdel(nodo* old)
{
    unsigned int temp;
    nodo *temp2;
    temp=jenkins_one_at_a_time_hash(old->dir, strlen(old->dir));
    hash[temp]=NULL;
    temp++;
    while(hash[temp])
    {
        temp2=hash[temp];
        hash[temp]=NULL;
        insert(temp2);
        temp=(temp+1)%DIM;
    }
}

void findn(char *curr)
{
    int dim=MAX;
    int t;
    int i=0;
    char *str;
    char **lista;
    nodol *testa;
    nodol *coda;
    nodol *temp;

    if (!root->figlio) printf("no\n");
    else
    {
        testa=(nodol*)malloc(sizeof(nodol));
        lista=(char**)malloc(dim*sizeof(char *));
        testa->corrente=root->figlio;
        coda=testa;
        testa->next=NULL;
        lista[0]=NULL;
        while(testa)
        {
            str=strrchr(testa->corrente->dir,'/');
            str=&str[1];
            if (!strcmp(str,curr))
            {
                if(i==dim){
                   lista=realloc(lista, sizeof(char*)*(dim+=MAX));
                }
                lista[i]=testa->corrente->dir;
                i++;
            }
            if (testa->corrente->fratello)
            {
                temp=(nodol *)malloc(sizeof(nodol));
                temp->corrente=testa->corrente->fratello;
                temp->next=NULL;
                coda->next=temp;
                coda=temp;
            }
            if (testa->corrente->figlio)
            {
                temp=(nodol *)malloc(sizeof(nodol));
                temp->corrente=testa->corrente->figlio;
                temp->next=NULL;
                coda->next=temp;
                coda=temp;
            }
            temp=testa;
            testa=testa->next;
            free(temp);
        }
        if(!i) printf("no\n");
        else{
        lista=realloc(lista, sizeof(char*)*(i));
        qsort(lista, i, sizeof(char *), cstring_cmp);
        for(t=0;t<i;t++) printf("ok %s\n",lista[t]);
        }
        free(lista);
    }
}

int cstring_cmp(const void *a, const void *b){
    const char **ia = (const char **)a;
    const char **ib = (const char **)b;
    return strcmp(*ia, *ib);
}

void deleted_r(char *curr)
{
    nodo *padre;
    nodo **pila;
    int i=0;
    int t=0;
    int dim=MAX;

    padre=find(curr);
    if (padre==NULL) printf("no\n");
    else if (!(padre->figlio))
    {
        i=deleted(curr);
        if (i) printf("ok\n");
        else printf ("Qualcosa non va");
    }
    else
    {
        pila=(nodo**)malloc(dim*sizeof(nodo *));
        pila[i]=padre->figlio;
        padre->figlio=NULL;
        while(pila[i]->figlio||pila[i]->fratello)
        {
            if (pila[i]->figlio)
            {
                t++;
                if(t==dim) pila=realloc(pila, sizeof(nodo*)*(dim+=MAX));
                pila[t]=pila[i]->figlio;
            }
            if (pila[i]->fratello)
            {
                t++;
                if(t==dim) pila=realloc(pila, sizeof(nodo*)*(dim+=MAX));
                pila[t]=pila[i]->fratello;
            }
            i++;
        }
        while(t>=0)
        {
            hashdel(pila[t]);
            free(pila[t]->dir);
            if(pila[t]->stringa) free(pila[t]->stringa);
            free(pila[t]);
            t--;
        }
        i=deleted(curr);
        if (i) printf("ok\n");
        else printf ("Qualcosa non va");
        free(pila);
    }
}


int deleted(char *curr)
{
    int i=0;
    char *temp;
    nodo *padre;
    nodo *son;

    son=find(curr);
    temp=strrchr(curr,'/'); //posizione ultimo '/'
    temp[0]='\0';
    padre=find(curr);
    temp[0]='/';

    if(son&&padre&&!(son->figlio))
    {
        if (padre->figlio==son)
        {
            padre->figlio=son->fratello;
        }
        else
        {
            padre=padre->figlio;
            while(padre->fratello!=son) padre=padre->fratello;
            padre->fratello=son->fratello;
        }
        hashdel(son);
        free(son->dir);
        if(son->stringa) free(son->stringa);
        free(son);
        i=1;
    }
    return i;
}





void write(char *curr)                        //NON CONTROLLO CHE LE STRINGHE SIANO COMPRESE FRA ""
{
    int temp1;
    nodo *temp;
    temp=find(curr);
    if (temp==NULL) printf("no\n");
    else if(temp->tipo==0) printf("no\n");
    else
    {
        if (temp->stringa!=NULL) free(temp->stringa);
        curr=strtok (NULL," ");
        temp1=strlen(curr);
        curr[temp1-1]='\0';
        curr=&curr[1];
        temp->stringa=(char *)malloc((strlen(curr)+1)*sizeof(char));
        strcpy(temp->stringa, curr);
        printf("ok %d\n",temp1-2);
    }
}

void read(char *curr)
{
    nodo *temp;
    temp=find(curr);
    if (temp==NULL) printf("no\n");
    else if (temp->tipo==0) printf("no\n");
    else if (temp->stringa==NULL) printf("contenuto \n");
    else printf("contenuto %s\n",temp->stringa);
}

void create(char *curr, char *input)        // NON CONTROLLO CHE LA DIRECTORY SIA SCRITTA BENE (CON I GIUSTI /)
{
    nodo *padre; //puntatore al padre
    nodo *son;   //puntatore al figlio
    char *temp;
    int i;       //var contatore

    son=find(curr);
    temp=strrchr(curr,'/'); //posizione ultimo '/'
    temp[0]='\0';
    padre=find(curr);
    temp[0]='/';
    if(!padre||son) printf("no\n");
    else
    {
        if(padre->tipo==1) printf("no\n");
        else
        {
            if (padre->altezza>=MAX) printf("no\n");
            else
            {
                if (padre->figlio==NULL)
                {
                    son=creael();
                    padre->figlio=son;
                    son->dir=(char *)malloc((strlen(curr)+1)*sizeof(char));
                    strcpy(son->dir, curr);
                    son->altezza=(padre->altezza)+1;
                    if(!strcmp(input,"create")) son->tipo=1;
                    insert(son);
                    printf("ok\n");
                }
                else
                {
                    padre=padre->figlio;
                    i=1;
                    while(padre->fratello!=NULL)
                    {
                        padre=padre->fratello;
                        i++;
                    }
                    if(i>=MAX_FIGLI) printf("no\n");
                    else
                    {
                        son=creael();
                        padre->fratello=son;
                        son->dir=(char *)malloc((strlen(curr)+1)*sizeof(char));
                        strcpy(son->dir, curr);
                        son->altezza=padre->altezza;
                        if(!strcmp(input,"create")) son->tipo=1;
                        insert(son);
                        printf("ok\n");
                    }
                }
            }
        }
    }
}

void insert (nodo *nnew)               //NON CONSIDERO I DELETED
{
    unsigned int temp;
    temp=jenkins_one_at_a_time_hash(nnew->dir, strlen(nnew->dir));
    while (hash[temp]!=NULL) temp=(temp+1)%DIM;
    hash[temp]=nnew;
}

nodo* find(char *str)
{
    unsigned int temp;
    if (str[0]=='\0')
    {
        return root;
    }
    temp=jenkins_one_at_a_time_hash(str, strlen(str));
    while (hash[temp]!=NULL)
    {
        if (!strcmp(str,hash[temp]->dir)) return hash[temp];
        temp=(temp+1)%DIM;
    }
    return NULL;
}



void init()
{
    root=NULL;
    hash=NULL;
    root=creael();
    if (root==NULL) printf("Errore nella creazione della root");
    hash = (nodo **) calloc(DIM, sizeof (nodo *));
    if (hash==NULL) printf ("Errore nella creazione della hash");
}

nodo* creael()
{
    nodo *temp;

    temp=(nodo *) malloc(sizeof(nodo));
    temp->tipo=0;
    temp->dir=NULL;
    temp->stringa=NULL;
    temp->fratello=NULL;
    temp->figlio=NULL;
    temp->altezza=0;

    return temp;

}



uint32_t jenkins_one_at_a_time_hash(const uint8_t* key, size_t length)
{
    size_t i = 0;
    uint32_t hash = 0;
    while (i != length)
    {
        hash += key[i++];
        hash += hash << 10;
        hash ^= hash >> 6;
    }
    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;
    return (hash%DIM);
}

char *getinput(int size)
{
    char *str;
    int ch;
    int len = 0;
    str = realloc(NULL, sizeof(char)*size+1);
    while(EOF!=(ch=fgetc(stdin)) && ch != '\n')
    {
        str[len++]=ch;
        if(len==size)
        {
            str = realloc(str, 1+sizeof(char)*(size+=MAX));
            if(!str)return str;
        }
    }
    str[len++]='\0';

    return realloc(str, sizeof(char)*len);
}
