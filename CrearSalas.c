struct SalaTematica *crearSala(char *tematica,int codigo,int aforoAprox){

    struct SalaTematica *nuevo;

    nuevo = (struct SalaTematica*)malloc(sizeof(struct SalaTematica));
    nuevo->tematica = (char*)malloc(strlen(tematica)+1*sizeof(char));
    strcpy(nuevo->tematica,tematica);
    nuevo->codigo = codigo;
    nuevo->aforoAprox = aforoAprox;
    nuevo->raiz = NULL;

    return nuevo;
}


struct NodoSala *crearNodoSala(struct SalaTematica *nuevaSala){

    struct NodoSala *nuevoNodo;

    nuevoNodo = (struct NodoSala*) malloc(sizeof(struct NodoSala));
    nuevoNodo->dato = nuevaSala;
    nuevoNodo->sig = NULL;
    nuevoNodo->ant = NULL;

    return nuevoNodo;
}

void insertarSala(struct NodoSala**headSalas,struct NodoSala *nuevaSala) {

    struct NodoSala *rec;

    if (*headSalas == NULL) {
        *headSalas = nuevaSala;
    }else {
        rec = *headSalas;
        while (rec->sig != NULL) {
            rec = rec->sig;
        }
        rec->sig = nuevaSala;
        rec->sig->ant = rec;
    }
}
