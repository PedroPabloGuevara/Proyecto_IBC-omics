#define MAXPASES 50
#include <string.h>

struct Cliente *crearCliente()
{
    struct Cliente *nuevo;
    int i, size;
    char buffer[50]; /* espacio temporal */
    nuevo=((struct Cliente*)malloc(sizeof(struct Cliente)));

    scanf(" %49[^\n]", buffer); /* ingresar nombre. [^\n] lee hasta salto de linea */
    size=strlen(buffer);
    nuevo->nombre=((char *)malloc((size+1)*sizeof(char)));
    strcpy(nuevo->nombre, buffer);

    scanf("%d", &nuevo->rut); /* ingresar rut */

    nuevo->pases=((struct Pase **)malloc(MAXPASES*sizeof(struct Pase *))); /* arr pases */
    for(i=0; i<MAXPASES; i++)
    {
        nuevo->pases[i]=NULL;
    }
    nuevo->plibrepases=0;

    return nuevo;
}

struct NodoCliente *crearNodoCliente(struct Cliente *dato) /* para meter en lista */
{
    struct NodoCliente *nuevo;

    nuevo=((struct NodoCliente*)malloc(sizeof(struct NodoCliente)));
    nuevo->dato=dato;
    nuevo->sig=NULL;

    return nuevo;
}

struct NodoCliente *buscarCliente(struct NodoCliente *head, int rut) {
    struct NodoCliente *rec=head;
    if(rec!=NULL)
    {
        do
        {
            if(rec->dato->rut==rut)
            {
                return rec;
            }
            rec=rec->sig;
        }while(rec!=head);
    }
    return NULL;
}
void enlazarCliente(struct NodoCliente **head, struct NodoCliente *nuevo)
{
    struct NodoCliente *rec;

    if(*head==NULL)
    {
        *head=nuevo;
        nuevo->sig=*head;
    }
    else
    {
        rec=*head;
        while(rec->sig!=*head)
        {
            rec=rec->sig;
        }
        rec->sig=nuevo;
        nuevo->sig=*head;
    }
}

int agregarCliente(struct NodoCliente **head) /* retorna 1 se lo agrego, 0 si el rut ya existia */
{
    struct Cliente *dato;

    dato=crearCliente();
    if(buscarCliente(*head, dato->rut)!=NULL)
    {
        return 0;
    }
    enlazarCliente(head, crearNodoCliente(dato));
    return 1;
}

int eliminarCliente(struct NodoCliente **head, int rut)
{
    struct NodoCliente *act, *ant; /* actual y anterior */

    if(*head==NULL)
    {
        return 0;
    }

    ant=*head;
    while(ant->sig!=*head)
    {
        ant=ant->sig;
    }

    act=*head;
    do /* buscar el nodo a eliminar */
    {
        if(act->dato->rut==rut)
        {
            break;
        }
        ant=act;
        act=act->sig;
    }while(act!=*head);

    if(act->dato->rut!=rut)
    {
        return 0; /* no se encontro */
    }

    if(act->sig==act) /* por si era el unico nodo y lista queda vacia */
    {
        *head=NULL;
    }
    else
    {
        ant->sig=act->sig;

        if(act==*head) /* si eliminado era head, el head pasa al siguiente */
        {
            *head=act->sig;
        }
    }

    return 1;
}

int modificarCliente(struct NodoCliente *head, int rut) /* 1 : se modifico, 0: no existe, -1: rut ya existe */
{
    struct NodoCliente *nodo;
    int nuevoRut, size;
    char buffer[50];

    nodo=buscarCliente(head, rut);
    if(nodo==NULL)
    {
        return 0;
    }

    printf("nuevo nombre: ");
    scanf(" %49[^\n]", buffer);
    printf("nuevo rut: ");
    scanf("%d", &nuevoRut);

    if(nuevoRut!=rut && buscarCliente(head, nuevoRut)!=NULL) /* rut no puede pertenecer a otro cliente */
    {
        return -1;
    }

    /* reservar memoria para nombre y copiar */
    size=strlen(buffer);
    nodo->dato->nombre=((char *)malloc((size+1)*sizeof(char)));
    strcpy(nodo->dato->nombre, buffer);
    nodo->dato->rut=nuevoRut;

    return 1;
}

void listarClientes(struct NodoCliente *head) /* mostrar clientes */
{
    struct NodoCliente *rec=head;
    int cantidad=0;

    if(rec==NULL)
    {
        printf("No hay clientes registrados.\n");
    }
    else
    {
        do
        {
            cantidad++;
            printf("%d) Nombre: %s, RUT: %d, Pases: %d\n",
                   cantidad, rec->dato->nombre, rec->dato->rut,
                   rec->dato->plibrepases);
            rec=rec->sig;
        }while(rec!=head);
    }
}