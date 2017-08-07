#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#define MAX 255
#define MAX_FIGLI 1024


typedef struct t
{
    bool tipo;           //tipo=0 dir; tipo=1 file
    char *dir;           //directory
    char *stringa;       //contenuto file
    struct t* fratello;
    struct t* figlio;
    short int altezza;
    short int figli;
} nodo;

typedef struct n     //utilizzata nella find
{
    nodo *corrente;
    struct n *next;
} nodol;




void init();                 //inizializza root e hash table
nodo* creael();              //alloca un elemento generico (tipo directory di default)
char *getinput(int size);    //ritorna un puntatore alla stringa acquisita da stdin
void create(char *curr, char* input); //crea file o directory
nodo* find(char *str);       //ritorna puntatore al nodo cercato se esiste nella hash, NULL altrimenti
void insert(nodo* nnew);      //inserisce new nella hashmap
void read(char *curr);        //legge il contenuto della stringa contenuta nel file
void write(char *curr);       //sovrascrive il contenuto della stringa nel file
int deleted(char *curr);      //elimina l' elemento curr, se esso non ha figli
void hashdel(nodo* old);      //cancella old dalla hashmap
void findn(char *curr);       //funzione find
void deleted_r(char *curr);   //funzione delete ricorsiva


uint32_t jenkins_one_at_a_time_hash(char* key, size_t length);    //funzione di hashing
int cstring_cmp(const void *a, const void *b);                    //funzione string compare per il qsort


nodo* root;    //radice albero
nodo** hash;   //puntatore alla hash table
const int DIM = 1048576;    //dimensione hash table

int main()
{
    char *input;
    char *curr;  //stringa corrente dopo strtok
    int i;       //contatore

    init();

    while(1)
    {
        input   = getinput(MAX);
        curr    = strtok (input, " ");

        if(!strcmp(curr, "create_dir") || !strcmp(curr, "create"))
        {
            curr = strtok (NULL, " "); //curr diventa la directory
            create(curr, input);
        }
        else if(!strcmp(curr, "read"))
        {
            curr = strtok (NULL, " ");
            read(curr);
        }
        else if(!strcmp(curr, "write"))
        {
            curr = strtok (NULL, " ");
            write(curr);
        }
        else if(!strcmp(curr, "delete"))
        {
            curr    = strtok (NULL, " ");
            i       = deleted(curr);
            if(i)
                printf("ok\n");
            else
                printf("no\n");
        }
        else if(!strcmp(curr, "delete_r"))
        {
            curr = strtok (NULL, " ");
            deleted_r(curr);
        }
        else if(!strcmp(curr, "find"))
        {
            curr = strtok (NULL, " ");
            findn(curr);
        }
        else if(!strcmp(curr, "exit"))
            return 0;
        free(input);
    }
}

void init()
{
    root = creael();
    hash = (nodo **) calloc(DIM, sizeof (nodo *));
}

nodo* creael()
{
    nodo *temp;

    temp=(nodo *) malloc(sizeof(nodo));
    temp->tipo      = 0;                  //directory di default
    temp->dir       = NULL;
    temp->stringa   = NULL;
    temp->fratello  = NULL;
    temp->figlio    = NULL;
    temp->altezza   = 0;
    temp->figli     = 0;

    return temp;

}

char *getinput(int size)
{
    char *str;
    int ch;
    int len = 0;
    str     = realloc(NULL, sizeof(char)*(size));
    while(EOF!=(ch=fgetc(stdin)) && ch != '\n')
    {
        str[len++] = ch;
        if(len==size)
        {
            str = realloc(str, sizeof(char)*(size+=MAX));
        }
    }
    str[len++]='\0';

    return realloc(str, sizeof(char)*len);
}

void create(char *curr, char *input)
{
    nodo *padre; //puntatore al padre
    nodo *son;   //puntatore al figlio
    char *temp;

    son     = find(curr);
    temp    = strrchr(curr, '/'); //posizione ultimo '/'
    temp[0] = '\0';
    padre   = find(curr);
    temp[0] = '/';

    if(!padre || son)     //controllo che il padre esista e l' elemento da creare non sia già presente
        printf("no\n");
    else
    {
        if(padre->tipo == 1 || padre->altezza >= MAX) //controllo che il padre sia una cartella e non venga superata l' altezza massima
            printf("no\n");
        else
        {
            if (padre->figlio == NULL)
            {
                son             = creael();
                padre->figlio   = son;
                son->dir        = (char *)malloc((strlen(curr)+1)*sizeof(char));
                strcpy(son->dir, curr);
                son->altezza    = (padre->altezza) + 1;
                if(!strcmp(input,"create"))
                    son->tipo   = 1;
                padre->figli++;
                insert(son);
                printf("ok\n");
            }
            else
            {
                if (padre->figli==MAX_FIGLI)
                    printf("no\n");
                else
                {
                    son             = creael();
                    son->fratello   = padre->figlio;
                    padre->figlio   = son;
                    son->dir        = (char *)malloc((strlen(curr)+1)*sizeof(char));
                    strcpy(son->dir, curr);
                    son->altezza    = (padre->altezza) + 1;
                    if(!strcmp(input,"create"))
                        son->tipo = 1;
                    padre->figli++;
                    insert(son);
                    printf("ok\n");
                }
            }
        }
    }
}

nodo* find(char *str)
{
    unsigned int temp;
    if (str[0]=='\0')
    {
        return root;
    }
    temp = jenkins_one_at_a_time_hash(str, strlen(str));
    while (hash[temp]!=NULL)
    {
        if (!strcmp(str,hash[temp]->dir))
            return hash[temp];
        temp=(temp+1)%DIM;
    }
    return NULL;
}

void insert (nodo *nnew)
{
    unsigned int temp;
    temp = jenkins_one_at_a_time_hash(nnew->dir, strlen(nnew->dir));
    while (hash[temp]!=NULL)
        temp = (temp+1)%DIM;
    hash[temp] = nnew;
}

void read(char *curr)
{
    nodo *temp;
    temp = find(curr);
    if (temp == NULL)
        printf("no\n");
    else if (temp->tipo == 0)
        printf("no\n");
    else if (temp->stringa == NULL)
        printf("contenuto \n");
    else
        printf("contenuto %s\n", temp->stringa);
}

void write(char *curr)
{
    int len;
    nodo *temp;
    temp = find(curr);
    if (temp == NULL)
        printf("no\n");
    else if(temp->tipo == 0)
        printf("no\n");
    else
    {
        if (temp->stringa != NULL)
            free(temp->stringa);
        curr            = strtok (NULL, " ");
        len             = strlen(curr);
        curr[len-1]     = '\0';
        curr            = &curr[1];
        temp->stringa   = (char *)malloc((len - 1) * sizeof(char));
        strcpy(temp->stringa, curr);
        printf("ok %d\n", len-2);
    }
}

int deleted(char *curr)
{
    int i = 0;
    char *temp;
    nodo *padre;
    nodo *son;

    son     = find(curr);
    temp    = strrchr(curr,'/'); //posizione ultimo '/'
    temp[0] = '\0';
    padre   = find(curr);
    temp[0] = '/';

    if(son && !(son->figli))  //controllo che l' elemento non abbia figli
    {
        padre->figli--;
        if (padre->figlio == son)
        {
            padre->figlio = son->fratello;
        }
        else
        {
            padre = padre->figlio;
            while(padre->fratello != son)
                padre = padre->fratello;
            padre->fratello = son->fratello;
        }
        hashdel(son);           //cancello l' elemento dalla hash table
        free(son->dir);
        if(son->stringa)
            free(son->stringa);
        free(son);
        i = 1;
    }
    return i;
}

void hashdel(nodo* old)
{
    unsigned int pos, hole, temporary;

    pos = jenkins_one_at_a_time_hash(old->dir, strlen(old->dir));
    while(hash[pos] != old)
        pos = (pos + 1) % DIM;
    hash[pos] = NULL;
    hole      = pos;
    pos       = (pos + 1) % DIM;

    while(hash[pos])             //rehashing degli elementi successivi fino al primo null
    {
        temporary = jenkins_one_at_a_time_hash(hash[pos]->dir, strlen(hash[pos]->dir));

        if (temporary <= hole){
            hash[hole]= hash[pos];
            hash[pos] = NULL;
            hole = pos;
        }
        pos       = (pos + 1) % DIM;
    }
}

void findn(char *curr)
{
    int dim = MAX;
    int t;
    int i = 0;
    char *str;
    char **lista;      //vettore dinamico di puntatori a stringhe
    nodol *testa;      //lista di elementi dell' albero da scorrere
    nodol *coda;
    nodol *temp;

    if (!(root->figlio))
        printf("no\n");
    else
    {
        testa           = (nodol*) malloc(sizeof(nodol));
        lista           = (char**) malloc(dim * sizeof(char *));
        testa->corrente = root->figlio;
        coda            = testa;
        testa->next     = NULL;
        lista[0]        = NULL;

        while(testa)
        {
            str = strrchr(testa->corrente->dir, '/');
            str = &str[1];
            if (!strcmp(str, curr))
            {
                if(i == dim)
                {
                    lista = realloc(lista, sizeof(char*) * (dim += MAX));
                }
                lista[i] = testa->corrente->dir;      //inserisco in lista la stringa trovata
                i++;
            }
            if (testa->corrente->fratello)           //inserisco in coda gli eventuali figli/fratelli
            {
                temp            = (nodol *)malloc(sizeof(nodol));
                temp->corrente  = testa->corrente->fratello;
                temp->next      = NULL;
                coda->next      = temp;
                coda=temp;
            }
            if (testa->corrente->figlio)
            {
                temp            = (nodol *)malloc(sizeof(nodol));
                temp->corrente  = testa->corrente->figlio;
                temp->next      = NULL;
                coda->next      = temp;
                coda            = temp;
            }
            temp    = testa;
            testa   = testa->next;
            free(temp);     //cancello la testa dalla lista
        }
        if(!i)
            printf("no\n");
        else
        {
            lista = realloc(lista, sizeof(char*) * (i));
            qsort(lista, i, sizeof(char *), cstring_cmp); //riordino le stringhe percorso
            for(t = 0; t < i; t++)
                printf("ok %s\n", lista[t]);
        }
        free(lista);
    }
}

void deleted_r(char *curr)
{
    nodo *padre;
    nodo **pila;     //inserisco in pila l' intero sottoalbero di curr e poi faccio la free di ogni elemento
    int i   = 0;
    int t   = 0;
    int dim = MAX;

    padre   = find(curr);
    if (padre == NULL)
        printf("no\n");
    else if (!(padre->figlio))
    {
        deleted(curr);
        printf("ok\n");
    }
    else
    {
        pila            = (nodo**)malloc(dim * sizeof(nodo *));
        pila[i]         = padre->figlio;
        padre->figli    = 0;      //altrimenti la deleted non andrebbe a buon fine

        while(pila[i]->figlio || pila[i]->fratello)
        {
            if (pila[i]->figlio)
            {
                t++;
                if(t == dim)
                    pila = realloc(pila, sizeof(nodo*) * (dim += MAX));
                pila[t] = pila[i]->figlio;    //inserisco in coda
            }
            if (pila[i]->fratello)
            {
                t++;
                if(t == dim)
                    pila = realloc(pila, sizeof(nodo*) * (dim += MAX));
                pila[t] = pila[i]->fratello;  //inserisco in coda
            }
            i++;
        }
        while(t >= 0)
        {
            hashdel(pila[t]);
            free(pila[t]->dir);
            if(pila[t]->stringa)
                free(pila[t]->stringa);
            free(pila[t]);
            t--;
        }
        deleted(curr);
        printf("ok\n");
        free(pila);
    }
}

uint32_t jenkins_one_at_a_time_hash(char* key, size_t length)
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

int cstring_cmp(const void *a, const void *b)
{
    const char **ia = (const char **)a;
    const char **ib = (const char **)b;
    return strcmp(*ia, *ib);
}
