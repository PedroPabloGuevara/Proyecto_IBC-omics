# Guia detallada de funciones

Este documento describe las funciones implementadas en `src/` y las funciones
auxiliares de `tests/`. Para cada una se indican su objetivo, los datos que
recibe y los resultados o cambios que produce. Los nombres de estado usados por
libros son: `1` disponible, `2` agotado, `3` en preventa y `4` descatalogado.
Los estados de pase son: `1` activo, `2` anulado, `3` utilizado y `4` vencido.

## Inicio de la aplicacion

Archivo: [src/main.c](./src/main.c)

### `main`

Es el punto de entrada del ejecutable. Actualmente imprime un mensaje de inicio
y termina con codigo `0`. Todavia no conecta los menus de clientes, pases,
reservas, libros o salas ni ejecuta el cierre de una jornada.

## Libros y arbol binario

Archivos: [src/arbol-binario/libros.c](./src/arbol-binario/libros.c) y
[src/arbol-binario/operacionesArbol.c](./src/arbol-binario/operacionesArbol.c).

### `copiarTexto`

Crea una copia dinamica de una cadena. Si recibe `NULL`, retorna `NULL`; si
`malloc` no consigue memoria, tambien retorna `NULL`. Quien recibe una copia
correcta queda a cargo de liberarla.

### `crearLibro`

Recibe codigo, nombre, precio, cantidad de ejemplares, editorial, clasificacion
por edad, disponibilidad, genero, fecha de publicacion y un indicador de
coleccion. Rechaza codigos no positivos, textos nulos, precio o stock negativos,
estados fuera de `1..4`, libros disponibles sin stock y libros agotados o en
preventa con stock distinto de cero. Reserva el struct, copia sus textos y deja
la cola de reservas vacia. Retorna el libro o `NULL` si los datos o la memoria
no permiten crearlo. Un libro descatalogado puede conservar stock registrado.

### `crearNodoLibro`

Recibe un `Libro *` y crea el nodo que lo enlaza al arbol. Inicializa sus dos
hijos en `NULL`. No copia ni libera el libro. Retorna el nodo o `NULL` si el
libro no existe o falla la reserva de memoria.

### `buscarLibro`

Busca recursivamente un codigo en el arbol binario de busqueda. Compara con el
codigo del nodo actual y continua por la izquierda si el solicitado es menor o
por la derecha si es mayor. Retorna el nodo encontrado o `NULL`.

### `menorNodo`

Recibe la raiz no nula de un subarbol y sigue sus enlaces izquierdos hasta
encontrar el nodo con el menor codigo. Se usa al borrar un nodo que tiene dos
hijos, para localizar su sucesor en orden. Retorna ese nodo y no modifica el
arbol.

### `insertarLibro`

Recibe un puntero a la raiz y un nodo ya creado. Inserta el nodo conservando el
orden por codigo: menores a la izquierda y mayores a la derecha. Si el arbol esta
vacio, el nodo pasa a ser la raiz. Rechaza argumentos incompletos y codigos
duplicados. Retorna `1` si inserta y `0` si no lo hace. Si falla, la propiedad
del nodo sigue siendo del llamador.

### `existeLibro`

Usa `buscarLibro` para comprobar si hay un nodo con el codigo pedido. Retorna
`1` si existe y `0` si no existe.

### `contarLibros`

Cuenta recursivamente el nodo actual y los nodos de ambos subarboles. Retorna
`0` para una rama vacia y, en otro caso, el numero total de libros bajo la raiz.

### `mostrarLibrosPreorden`

Imprime codigo y nombre de cada libro en recorrido preorden: primero el nodo
actual, despues el subarbol izquierdo y por ultimo el derecho. Una raiz `NULL`
no produce salida.

### `modificarPrecio`

Busca el libro por codigo y cambia su precio si existe y el nuevo valor no es
negativo. No cambia el codigo ni el stock. Retorna `1` si modifica y `0` si el
libro no existe o el precio es invalido.

### `recibirDespacho`

Recibe la raiz, el codigo y una cantidad de ejemplares que llegan. Rechaza
libros inexistentes, cantidades no positivas, sumas que excederian `INT_MAX` y
libros descatalogados. Al aceptar, suma la cantidad al stock; si el libro estaba
agotado o en preventa, pasa a disponible. Retorna `1` si registra el despacho y
`0` en caso contrario.

### `descatalogarLibro`

Busca el libro por codigo. Si existe, cuenta sus reservas cuando se proporciona
`reservasCanceladas`, vacia la cola y marca el libro como descatalogado. El nodo
permanece en el arbol y el stock se conserva. Inicializa el contador opcional en
cero antes de buscar, por lo que tambien queda en cero si el codigo no existe.
Retorna `1` si encontro y descatalogo el libro, o `0` si no existe.

### `liberarLibro`

Libera la cola de reservas y los textos dinamicos del libro, y finalmente el
struct. No libera el nodo del arbol que pudiera contenerlo. Acepta `NULL` sin
hacer nada.

### `eliminarLibro`

Busca un codigo usando un puntero doble a la raiz, porque la raiz puede cambiar.
Si el nodo no existe retorna `0`. Si tiene cero o un hijo, reemplaza el enlace
del padre por el hijo restante y libera el libro y el nodo. Si tiene dos hijos,
intercambia el contenido del libro con el sucesor minimo del subarbol derecho y
elimina recursivamente el nodo sucesor. Retorna `1` al eliminar.

## Ventas y recaudacion

Archivo: [src/arbol-binario/ventasLibros.c](./src/arbol-binario/ventasLibros.c).

### `calcularRecaudacionDiaria`

Retorna `Tienda.recaudacion`. Retorna `-1` si la tienda es `NULL` o el total
almacenado es negativo. No calcula de nuevo las ventas recorriendo los libros.

### `reiniciarRecaudacionDiaria`

Pone en cero el acumulador de la tienda. Si recibe `NULL`, no hace nada. Debe
usarse solo cuando la operacion quiera reiniciar explicitamente ese total.

### `registrarVenta`

Funcion interna compartida por las dos funciones publicas de venta. Valida
tienda, libro, cantidad, estado disponible, stock, precio y recaudacion. Si hay
reservas, solo permite la venta al primer cliente de la cola y limita la venta a
una unidad. Si no hay reservas, limita las unidades al stock disponible.
Comprueba que el ingreso no desborde `int`; solo despues actualiza stock y
recaudacion. Si la llamada identifica comprador, elimina su reserva. Marca
agotado el libro cuando el stock llega a cero. Retorna unidades vendidas o `0`
si no puede completar la operacion.

### `venderEjemplar`

Busca el codigo en el arbol y solicita vender una unidad mediante
`registrarVenta`. No identifica al comprador; por ello no puede satisfacer una
reserva pendiente. Retorna `1` al vender o `0` si falta la tienda, el libro, el
stock o la venta no es permitida.

### `venderEjemplares`

Valida tienda y cantidad, busca el libro y delega en `registrarVenta`, indicando
el RUT del comprador. Puede vender hasta la cantidad solicitada y el stock, pero
si existe una cola de reservas solo vende una unidad al primer cliente
reservador. Retorna el numero vendido o `0` si no se vendio.

## Clientes y pases asociados

Archivo: [src/clientes/clientes.c](./src/clientes/clientes.c).

### `crearCliente`

Crea un cliente, copia su nombre, guarda el RUT y reserva espacio para
`MAXPASES` punteros a pases. Inicializa todas las posiciones a `NULL` y el
contador de pases a cero. Retorna el cliente o `NULL` si el nombre es nulo o
falla una asignacion. Quien crea el cliente debe liberar su memoria al eliminarlo.

### `crearNodoCliente`

Crea un nodo para una lista circular de clientes. Guarda el puntero al cliente y
deja el siguiente enlace en `NULL`, para que la insercion establezca la
circularidad. Retorna `NULL` si el dato no existe o falla `malloc`.

### `buscarCliente`

Busca un RUT desde la cabeza de una lista circular. Visita cada nodo una vez y
termina al volver a la cabeza. Retorna el nodo encontrado o `NULL` si no hay
coincidencia o la lista esta vacia.

### `enlazarCliente`

Inserta el nodo al final de la lista circular. En una lista vacia, el nodo se
apunta a si mismo y pasa a ser la cabeza; de lo contrario, busca el ultimo nodo,
lo enlaza al nuevo y hace que el nuevo apunte a la cabeza. Si los punteros de
entrada son invalidos, no cambia la lista.

### `agregarCliente`

Valida los punteros y comprueba que el RUT no exista. Crea el cliente y el nodo,
y luego los enlaza. Si falla una creacion, libera las asignaciones parciales.
Retorna `1` si agrega y `0` si rechaza los datos, el RUT duplicado o una
asignacion fallida.

### `registrarPaseCliente`

Asocia un pase externo activo al cliente identificado por RUT. Comprueba fecha,
codigo, tipo de pase, formato del horario de tienda, unicidad del codigo entre
clientes y el registro diario, y que haya una posicion libre entre los
`MAXPASES`. No realiza ventas ni altera la recaudacion. Retorna `1` si lo asocia;
en ese caso el cliente pasa a ser propietario del pase. Retorna `0` ante datos
invalidos o si el pase no se puede asociar; en ese caso el llamador conserva su
propiedad.

### `liberarPasesClientes`

Recorre la lista circular y libera todos los pases asociados a cada cliente.
Deja las posiciones en `NULL` y el contador de cada cliente en cero. Conserva los
clientes y sus otros datos. No hace nada si la lista esta vacia.

### `eliminarReservasClienteArbol`

Recorre recursivamente todos los libros del arbol y elimina de cada cola la
reserva cuyo cliente tenga el RUT indicado. Retorna la suma de reservas
eliminadas; una raiz nula aporta cero.

### `eliminarCliente`

Busca el cliente y evita dejar referencias colgantes al eliminarlo. Primero
retira de la fila los pases del cliente y borra sus reservas en los arboles de
libros de las salas. Luego lo desenlaza de la lista circular, libera sus pases,
nombre, arreglo de pases, struct y nodo. No borra las copias del registro diario.
Retorna `1` si elimina al cliente y `0` si no existe o la tienda no es valida.

### `modificarCliente`

Busca al cliente por su RUT actual, comprueba el nombre y evita asignar un RUT
que pertenezca a otra persona. Reserva y copia el nombre nuevo antes de liberar
el anterior; luego actualiza nombre y RUT. Retorna `1` si modifica, `0` si no
encuentra al cliente o el nombre es invalido, `-1` si el nuevo RUT esta ocupado y
`-2` si no puede reservar memoria.

### `listarClientes`

Imprime numero de orden, nombre, RUT y cantidad de pases de cada cliente en la
lista circular. Si no hay clientes, imprime un aviso. No modifica la lista.

## Fila y control de acceso

Archivos: [src/entrada/controlEntrada.c](./src/entrada/controlEntrada.c),
[src/entrada/filaEntrada.c](./src/entrada/filaEntrada.c),
[src/entrada/gestionEntrada.c](./src/entrada/gestionEntrada.c) y
[src/entrada/validacionEntrada.c](./src/entrada/validacionEntrada.c).

### `configurarControlAcceso`

Valida la fecha, la hora de apertura y la hora de cierre, y exige que el cierre
sea posterior a la apertura. Solo crea la fila si la tienda aun no tiene una.
Asigna la fila a la tienda despues de crearla correctamente. Retorna `1` si
configura y `0` si hay datos invalidos, ya estaba configurada o falla memoria.

### `liberarControlAcceso`

Libera la fila de entrada y sus nodos, sin liberar los pases referenciados.
Establece `tienda->filaEntrada` en `NULL`. Si la tienda es nula, no hace nada.

### `crearFila`

Reserva una estructura `Fila` e inicializa `head` y `tail` en `NULL`. Retorna la
fila vacia o `NULL` si falla la asignacion.

### `encolarFila`

Inserta un nodo de pase desligado. Rechaza punteros invalidos, tipo de pase
fuera de rango, codigo/referencia duplicada y extremos incoherentes. Los pases
tipo `4` se insertan delante de los normales, manteniendo FIFO dentro del grupo
prioritario; los otros se agregan al final. Retorna `1` si enlaza el nodo y `0`
si no lo hace. La fila adquiere el nodo, pero no el pase apuntado.

### `desencolarFila`

Retira el primer nodo y actualiza `head` y `tail`; ambos quedan en `NULL` al
retirar el ultimo. Desliga el nodo antes de retornarlo. Retorna `NULL` si la fila
no existe o esta vacia. El llamador libera el nodo, no el pase.

### `buscarEnFila`

Busca el nodo cuyo pase tenga el codigo indicado. No modifica la fila. Retorna
el nodo o `NULL` si no existe.

### `quitarDeFila`

Busca el pase por codigo, reconecta los enlaces y actualiza `head` o `tail` si
eliminara un extremo. Libera solo el nodo. Retorna `1` si lo quita y `0` si la
fila es nula o no encuentra el codigo.

### `vaciarFila`

Libera todos los nodos y deja los extremos en `NULL`, pero conserva la
estructura `Fila` y los pases referenciados. Acepta `NULL` sin cambios.

### `liberarFila`

Vacia primero los nodos con `vaciarFila` y luego libera la estructura de la
fila. No libera los pases. Acepta `NULL`.

### `cantidadEnFila`

Cuenta los nodos enlazados desde `head` hasta el final. Retorna cero si la fila
es nula o esta vacia; no altera el orden.

### `inicializarResultadoFila`

Pone `admitidos`, `rechazados` y `esperando` en cero. Si el puntero de resultado
es `NULL`, no hace nada.

### `solicitarIngreso`

Inicializa el resultado, valida la tienda, la fila y la relacion entre cliente y
pase, crea un nodo y lo encola. Luego llama a `procesarFila` con la hora actual.
Retorna `1` si la solicitud se registro y pudo procesarse, `0` si se rechazo la
solicitud antes de encolarla y `-1` si fallo el procesamiento tras encolarla.
Los conteos se escriben en el resultado cuando este no es nulo.

### `procesarFila`

Valida tienda, fecha, fila y horas. Antes de abrir conserva a todos en espera;
al llegar a la hora de cierre retira y cuenta como rechazados los nodos
pendientes. Durante el horario procesa desde la cabeza respetando prioridad y
llegada. Busca el titular, vuelve a validar el pase y descarta solicitudes
invalidas. Para un pase activo crea una copia independiente en el registro diario
y solo despues lo marca como utilizado; los utilizados ya registrados pueden
reingresar sin duplicar la entrada. Retorna `1` si la fila se pudo procesar,
`0` ante datos invalidos o falla al crear/agregar el registro diario.

### `soloDigitos`

Comprueba que los caracteres del rango inclusivo `[inicio, fin]` sean digitos.
Rechaza texto nulo, indices negativos, rango invertido o un final fuera de la
cadena. Retorna `1` si todos son digitos y `0` en otro caso.

### `numeroDe`

Convierte el rango inclusivo de caracteres numericos a entero acumulando cada
cifra en orden. Presupone que los limites son validos y que el rango contiene
solo digitos; no realiza validacion por si misma.

### `esBisiesto`

Aplica la regla gregoriana: un anio divisible por 400 es bisiesto; uno divisible
por 100 pero no por 400 no lo es; en los demas casos, basta ser divisible por 4.
Retorna `1` o `0`.

### `validarHoraFila`

Valida una cadena con formato exacto `HH:MM`, separador `:` y valores entre
`00:00` y `23:59`. Requiere un puntero de salida no nulo y, si es valida,
escribe los minutos desde medianoche. Retorna `1` si acepta y `0` si rechaza.

### `validarFechaFila`

Valida formato exacto `DD/MM/AAAA`, separadores y digitos; comprueba mes,
cantidad de dias y anios bisiestos. No acepta anio cero. Retorna `1` si la fecha
existe y `0` si es invalida.

### `horaDentroHorarioTienda`

Convierte la hora actual, apertura y cierre a minutos. Considera valido desde la
apertura inclusive hasta el cierre exclusivo. Rechaza horas invalidas y tiendas
cuyo cierre no sea posterior a la apertura. Retorna `1` dentro del intervalo y
`0` fuera.

### `buscarClientePorPaseFila`

Recorre la lista circular de clientes y las posiciones de pases de cada uno.
Compara la direccion del pase, no solo el codigo, para encontrar al titular.
Retorna el `Cliente *` o `NULL` si no existe.

### `validarSolicitudFila`

Comprueba tienda, cliente, pase, tipo, fecha de la tienda, hora actual,
intervalo horario y titularidad del pase. Un pase activo es valido solo si aun
no figura en el registro diario; uno utilizado es valido solo si ya figura.
Rechaza anulados, vencidos y estados desconocidos. Retorna `1` si se puede
procesar la solicitud y `0` si falla una condicion.

## Registro de pases diarios

Archivo: [src/pases/pasesDelDia.c](./src/pases/pasesDelDia.c).

### `crearPase`

Crea un pase y copia fecha y hora en memoria propia. Guarda tipo, codigo, valor
y estado tal como se reciben. Retorna el pase o `NULL` si falta fecha/hora o
falla alguna asignacion. La validacion de rangos del resto de los campos se
realiza en las operaciones que registran el pase.

### `liberarPase`

Libera fecha, hora y estructura del pase. Acepta `NULL` sin hacer nada.

### `crearNodoPase`

Crea un nodo desligado que referencia un pase existente; no copia ni libera el
pase. Retorna `NULL` si el pase es nulo o falla la memoria.

### `crearPasesDelDia`

Reserva la estructura de la lista diaria e inicializa su cabeza en `NULL`.
Retorna la lista vacia o `NULL` si falla memoria.

### `buscarPase`

Busca en la lista diaria un codigo de pase y retorna el nodo correspondiente.
Retorna `NULL` si la lista no existe o no encuentra el codigo.

### `agregarPase`

Agrega al final una referencia a un pase utilizado. Valida codigo positivo,
tipo entre `1..4`, estado utilizado, fecha y hora no nulas, y que no se repita
el codigo. Crea un nodo nuevo; retorna `1` si lo enlaza y `0` si falla. Ante
fallo, el pase sigue siendo propiedad del llamador.

### `listarPases`

Imprime cada pase del registro diario con codigo, tipo, estado, fecha y hora en
el orden de registro. Si no hay lista o esta vacia, imprime un aviso.

### `contarPasesPorEstado`

Recorre la lista y cuenta los pases cuyo estado coincide con el argumento.
Retorna cero para una lista nula o sin coincidencias.

### `paseValidoHoy`

Compara la fecha del pase con la fecha recibida como cadenas. Retorna `1` si
ambas existen y coinciden, o `0` si falta alguna o difieren.

### `liberarPasesDelDia`

Libera cada pase y nodo de la lista diaria, y finalmente la estructura de lista.
Esta lista es propietaria de las copias de pase que contiene. Acepta `NULL`.

## Reservas de libros

Archivo: [src/reservas/colaReservas.c](./src/reservas/colaReservas.c).

### `crearReserva`

Crea una reserva que guarda una referencia al cliente y el tipo de pase usado
para determinar prioridad. No copia ni posee al cliente. Retorna `NULL` si el
cliente es nulo o falla memoria.

### `crearNodoReserva`

Crea un nodo desligado que contiene una reserva. No copia la reserva. Retorna
`NULL` si la reserva es nula o falla memoria.

### `reservarLibro`

Valida que libro y cliente existan, que el libro este en preventa, que el pase
sea activo o utilizado y de tipo `1..4`, que el cliente tenga arreglo de pases y
que no tenga otra reserva del mismo libro. Exige que el pase sea exactamente uno
de los punteros asociados al cliente. Crea reserva y nodo y llama a `encolar`;
si falla cualquiera de esos pasos, libera lo que alcanzo a crear. Retorna `1` al
reservar y `0` si rechaza la solicitud.

### `encolar`

Inserta un nodo de reserva desligado en el libro en preventa. Rechaza estructuras
incompletas, tipo invalido, otro estado de libro, nodo ya enlazado, el mismo nodo
o un cliente que ya aparezca en la cola. Las reservas tipo `4` se colocan antes
de las normales y detras de las prioritarias que ya estaban; los demas tipos van
al final. Retorna `1` si enlaza. Si retorna `0`, el llamador conserva la
propiedad del nodo y de la reserva.

### `desencolar`

Retira y desliga la primera reserva del libro. Retorna el nodo para que quien
llama lo procese y libere; no libera ni nodo ni reserva. Retorna `NULL` si el
libro no existe o la cola esta vacia.

### `cantidadEnEspera`

Cuenta los nodos de reserva. Retorna cero si el libro es nulo o no tiene cola.

### `mostrarCola`

Imprime las reservas en orden de atencion, indicando posicion, nombre del
cliente y marca prioritaria para tipo de pase `4`. Si la cola no existe o esta
vacia, imprime un aviso.

### `buscarReserva`

Busca en la cola la reserva cuyo cliente tenga el RUT solicitado. Retorna el
nodo encontrado o `NULL` si el libro es nulo o no hay coincidencia. Ignora nodos
que tengan datos incompletos.

### `eliminarReserva`

Busca una reserva por RUT y la desenlaza desde la cabeza o desde su nodo
anterior. Libera el nodo y el struct `Reserva`, pero no libera al cliente.
Retorna `1` si elimina y `0` si el libro es nulo o no hay coincidencia.

### `vaciarCola`

Libera todas las reservas y nodos de un libro y deja `headReserva` en `NULL`.
No libera a los clientes referenciados. Acepta `NULL`.

## Salas tematicas

Archivos: [src/salas/CrearSalas.c](./src/salas/CrearSalas.c),
[src/salas/EliminarSala.c](./src/salas/EliminarSala.c) y
[src/salas/SalaModificar.c](./src/salas/SalaModificar.c).

### `crearSala`

Crea una sala con tematica, codigo y aforo aproximado. Copia la tematica y deja
el arbol de libros vacio. Retorna la sala o `NULL` si la tematica es nula o falla
memoria. La funcion no valida rangos de codigo o aforo.

### `crearNodoSala`

Crea un nodo doblemente enlazado que referencia una sala. Inicializa `sig` y
`ant` en `NULL`; no copia ni libera la sala. Retorna `NULL` si falta la sala o
memoria.

### `insertarSala`

Agrega el nodo al final de la lista doblemente enlazada de salas. Reinicia sus
enlaces antes de insertarlo y actualiza el enlace anterior del nuevo nodo. Si
alguno de los punteros principales es nulo, no hace nada. No devuelve resultado
de exito.

### `buscarSala`

Busca linealmente por codigo desde la cabeza de la lista. Retorna el nodo
encontrado o `NULL`.

### `desenlazarSala`

Funcion auxiliar que busca una sala y la retira de la lista doblemente
enlazada. Actualiza la cabeza si retira el primer elemento, repara los enlaces
vecinos y deja ambos enlaces del nodo retirado en `NULL`. Retorna el nodo
desligado, sin liberar ni nodo ni sala; retorna `NULL` si no se encuentra.

### `EliminarSala`

Desenlaza una sala y libera el nodo de lista, pero deja viva la estructura de
sala para que el codigo llamador pueda conservarla o liberarla. Retorna `-1` si
la lista o su puntero son invalidos, `0` si no existe el codigo y `1` si elimina
el nodo.

### `QuitarSala`

Desenlaza una sala, libera el nodo de lista y devuelve el puntero a la sala. El
llamador pasa a ser responsable de liberar la tematica, los libros y la propia
sala cuando corresponda. Retorna `NULL` si no existe.

### `mostrarTodasSalas`

Imprime tematica, aforo aproximado y codigo de cada sala. Recorre la lista sin
modificarla.

### `modificarSala`

Busca la sala por codigo, valida que el nuevo codigo no pertenezca a otra sala,
reserva y copia la nueva tematica, y actualiza tematica, codigo y aforo. Reserva
la copia antes de liberar el texto anterior, para mantener los datos si falla
memoria. Retorna `1` si modifica y `0` si faltan argumentos, la sala no existe,
el codigo esta ocupado o falla memoria.

## Cierre de jornada

Archivo: [src/tienda/cierreTienda.c](./src/tienda/cierreTienda.c).

### `cerrarTienda`

Valida tienda y puntero de salida, guarda la recaudacion, libera la fila de
ingreso, las credenciales de pases asociadas a clientes y el registro diario
independiente. Conserva clientes, salas, libros, reservas y recaudacion en la
tienda. Copia el total al parametro de salida. Retorna `1` si completa el cierre
y `0` si algun argumento es nulo.

## Funciones de prueba

Estas funciones estan en `tests/`; construyen escenarios o ejecutan aserciones y
no forman parte del ejecutable de produccion.

Archivo: [tests/main_flujo.c](./tests/main_flujo.c)

### `crearPasePrueba`

Crea un pase de prueba con valor cero usando la fecha y hora entregadas. La
funcion `crearPase` copia las cadenas. Retorna el pase dinamico o `NULL` si falla
la creacion.

### `agregarLibroPrueba`

Crea un libro con valores constantes para editorial, genero y fecha de
publicacion; crea su nodo, lo inserta en la raiz del arbol de la sala y devuelve
el libro. Las aserciones fallan inmediatamente si no se puede crear o insertar.

### `probarFechasHorasYConfiguracion`

Verifica formatos validos e invalidos de fechas y horas, conversion a minutos,
limites del horario y que el control de acceso solo se configure una vez.
Tambien comprueba el comportamiento de liberar la configuracion.

### `probarLimitePasesPorCliente`

Crea clientes, comprueba modificar RUT/nombre y registra exactamente
`MAXPASES` pases. Verifica que el pase adicional se rechace y elimina los
clientes para limpiar el escenario.

### `probarFlujoCompletoDeJornada`

Simula una jornada completa: clientes y pases validos/invalidos, prioridad de
fila, admision y reingreso, creacion y modificacion de sala, libros y estados,
reservas y su prioridad, despachos, ventas y recaudacion, descatalogacion,
eliminacion de referencias y cierre. Comprueba que el cierre conserva los datos
permanentes y que se pueden registrar pases en una jornada siguiente.

### `main`

Ejecuta las pruebas puntuales de fechas y capacidad, y luego el escenario de
jornada integrada. Si una asercion falla, el proceso de prueba termina con error;
si todas pasan, imprime un mensaje de exito y retorna `0`.

Archivo: [tests/pruebas_regresion.c](./tests/pruebas_regresion.c)

### `probarIngresosUnicosYReingresos`

Verifica que los pases prioritarios se atiendan primero, que un pase activo se
registre como utilizado al ingresar y que pueda reingresar sin duplicar el
registro diario ni sumar recaudacion. Comprueba tambien el rechazo de pases
anulados/vencidos y el cierre.

### `probarEliminacionDeReferenciasYDespachos`

Comprueba que al eliminar a un cliente se retiren sus pases de la fila y sus
reservas, sin borrar copias del registro diario. Tambien valida despacho,
reservas de otro cliente, venta de un libro, cierre y conservacion de los datos
permanentes.

### `probarCasosLimiteDeReservas`

Prueba los argumentos nulos, pase ajeno, pases anulados/vencidos o con tipo
invalido, libro en estado no reservable y duplicado de cliente/libro. Verifica
prioridad tipo `4` con FIFO estable, rechazo de nodos incompletos o enlazados,
desencolado, eliminacion por RUT, vaciado repetido y estado final de la cola.

### `main`

Ejecuta los escenarios regresivos de ingreso, eliminacion de referencias y
limites de reservas. Imprime el mensaje final y retorna `0` si todas las
aserciones se cumplen.
