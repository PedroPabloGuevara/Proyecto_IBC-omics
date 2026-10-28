int main() {
    int opcion, subopcion;
    int salaCodigo, salaAforoAprox, nuevoCodigo, nuevoAforo;
    int resultado, estadoTienda;
    char salaTematica[100];
    char nuevoNombre[100];
    struct NodoSala *headSalas = NULL;
    struct SalaTematica *salaQuitada;


    estadoTienda = 1;
    printf("Bienvenido al menu IBC-omincs\n");
    while (estadoTienda){
        printf("menu");
        scanf("%d",&opcion);
        switch (opcion){
            case 1:
                /* lista filaEntrada */
                printf("opciones de fila");
                scanf("%d",&subopcion);
                switch (subopcion)
                {
                    case 1:
                        /* code */
                        break;

                    default:
                        break;
                }


                break;
            case 2:
                /*cliente*/

                printf("1.Agregar 2.Buscar 3.Eliminar 4.Modificar 5.Listar 6.DarPase\n");
                scanf("%d", &subopcion);
                switch (subopcion) {
                    case 1:
                        printf("Nombre: ");
                        scanf("%99s", nombre);
                        printf("Rut: ");
                        scanf("%d", &rut);
                        resultado = agregarCliente(&headClientes, nombre, rut);
                        if (resultado == 1) printf("Cliente agregado\n");
                        else printf("El rut ya existe\n");
                        break;
                    case 2:
                        printf("Rut: ");
                        scanf("%d", &rut);
                        encontrado = buscarCliente(headClientes, rut);
                        if (encontrado != NULL)
                            printf("Cliente: %s rut: %d\n", encontrado->dato->nombre, encontrado->dato->rut);
                        else printf("No se encontro el cliente\n");
                        break;
                    case 3:
                        printf("Rut: ");
                        scanf("%d", &rut);
                        resultado = eliminarCliente(&headClientes, rut);
                        if (resultado == 1) printf("Cliente eliminado\n");
                        else printf("No se encontro el cliente\n");
                        break;
                    case 4:
                        printf("Rut actual: ");
                        scanf("%d", &rut);
                        printf("Nuevo rut: ");
                        scanf("%d", &nuevoRut);
                        limpiarBuffer();
                        printf("Nuevo nombre: ");
                        nuevoNombre = leerCadena();
                        if (nuevoNombre == NULL) {
                            printf("No hay memoria\n");
                            break;
                        }
                        resultado = modificarCliente(&headClientes, rut, nuevoNombre, nuevoRut);
                        free(nuevoNombre);               /* modificarCliente hizo su propia copia */
                        if (resultado == 1) printf("Cliente modificado\n");
                        else if (resultado == 0) printf("El cliente no existe\n");
                        else printf("El nuevo rut ya existe\n");
                        break;
                    case 5:
                        listarClientes(headClientes);
                        break;
                    case 6:
                        crearPase;
                        agregarPaseCliente(headClientes,rut,pase);
                        ventaPase += pase->precio;
                        contadorPases ++;
                        tienda->recaudacion += pase->precio;
                    default:
                        printf("Opcion no valida\n");
                        break;
                }
                break;


            case 3:
                /*salaTematica*/
                printf("1.Crear 2.Mostrar 3.Modificar 4.Eliminar 5.Quitar\n");
                scanf("%d", &subopcion);
                switch (subopcion) {
                    case 1:
                        printf("Tematica: ");
                        scanf("%99s", salaTematica);
                        printf("Codigo: ");
                        scanf("%d", &salaCodigo);
                        printf("Aforo aproximado: ");
                        scanf("%d", &salaAforoAprox);
                        insertarSala(&headSalas,crearNodoSala(crearSala(salaTematica, salaCodigo, salaAforoAprox)));
                        break;
                    case 2:
                        mostrarTodasSalas(headSalas);
                        break;
                    case 3:
                        printf("Codigo actual: ");
                        scanf("%d", &salaCodigo);
                        printf("Nuevo codigo: ");
                        scanf("%d", &nuevoCodigo);
                        printf("Nuevo aforo: ");
                        scanf("%d", &nuevoAforo);
                        printf("Nuevo nombre: ");
                        scanf("%99s", nuevoNombre);
                        resultado = modificarSala(&headSalas, salaCodigo, nuevoCodigo,nuevoAforo, nuevoNombre);
                        if (resultado == 1) printf("Sala modificada\n");
                        else printf("No se pudo modificar\n");
                        break;
                    case 4:
                        printf("Codigo: ");
                        scanf("%d", &salaCodigo);
                        resultado = EliminarSala(&headSalas, salaCodigo);
                        if (resultado == 1) printf("Sala eliminada\n");
                        else printf("No se encontro la sala\n");
                        break;
                    case 5:
                        printf("Codigo: ");
                        scanf("%d", &salaCodigo);
                        salaQuitada = QuitarSala(&headSalas, salaCodigo);
                        if (salaQuitada != NULL) printf("Sala quitada: %s\n", salaQuitada->tematica);
                        else printf("No se encontro la sala\n");
                        break;
                    default:
                        printf("Opcion no valida\n");
                        break;
                }
                break;

            case 4:
                /*reservaProducto*/
                printf("opciones de reservar producto");
                scanf("%d",&subopcion);
                switch (subopcion)
                {
                    case 1:
                        /* code */
                        break;

                    default:
                        break;
                }
                break;
            case 5:
                /*datos tienda*/
                printf("mostrar tienda");

                break;
           case 6:
                /*pasesDelDia*/
            case 0:
                estadoTienda = 0;
                break;

            default:
                printf("no se selecciono ninguna opcion Valida\n");
                break;
            }
    }
}