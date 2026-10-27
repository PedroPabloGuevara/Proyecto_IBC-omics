# Estado de la aplicacion

## Implementado en esta rama

- Estructuras de datos para clientes, pases diarios, reservas de libros, salas y tienda.
- Registro de clientes y almacenamiento de hasta 50 pases por cliente.
- Creacion, venta, consulta y administracion de pases diarios, con codigo individual,
  fecha, hora y recaudacion acumulada en `Tienda.recaudacion`.
- Modulo de pases separado entre administracion (`src/pases/pasesDelDia.c`) y
  flujo de venta (`src/pases/ventasPases.c`).
- Reservas de libros vinculadas a cada libro, con prioridad para reservas de tipo 4.
- Gestion de libros en arbol binario: busqueda, ventas por cantidad, despacho,
  descatalogacion y actualizacion de la recaudacion de la tienda.
- Modulo de libros separado entre operaciones del arbol (`operacionesArbol.c`), creacion y
  liberacion de libros (`libros.c`) y ventas/recaudacion (`ventasLibros.c`).
- Fila de ingreso de pases basada en `Fila` y `NodoPase`, separada de las reservas
  de libros; los pases tipo 4 tienen prioridad estable y los demas conservan FIFO.
- Validacion de fecha `DD/MM/AAAA`, hora `HH:MM`, horario del pase y horario de tienda.
- Al admitir un pase, su estado cambia a utilizado. El modelo compartido no registra
  ocupacion actual, salidas, reingresos ni asignacion de personas a salas.
- Modulo de entrada agrupado en `src/entrada/`: fila, gestion, control y validacion
  (`validacionEntrada.c`).

## Aclaracion sobre pases del dia

- En la implementacion actual, `Tienda.filaPasesDeldia` funciona como catalogo
  de pases modelo disponibles para vender: `ventasPases.c` busca ahi el tipo,
  y crea pases individuales que guarda en el arreglo `Cliente.pases`.
- Al procesar un ingreso, `gestionEntrada.c` cambia el estado del pase a
  utilizado y quita de la fila el nodo que esperaba. Actualmente no agrega ese
  pase a `filaPasesDeldia`, ni conserva un registro diario de ingresos unicos.
- La regla de funcionamiento solicitada es distinta: `pasesDelDia` debe
  representar los pases que ingresaron al menos una vez durante el dia. El
  primer ingreso registra el pase y cuenta una entrada; una salida no lo quita,
  y un reingreso con el mismo pase no debe registrarlo ni contarlo nuevamente.
- Esta regla todavia no esta implementada. Antes de cambiar el codigo hay que
  separar claramente el catalogo de tipos vendibles del registro de pases que
  ya ingresaron, y definir como validar reingresos con el modelo de structs
  compartido.

## Pendiente antes de considerar completa la aplicacion

- Conectar las operaciones de clientes, pases, libros, salas y fila desde `main.c`.
- Alinear el uso de `pasesDelDia` con la regla acordada para registrar una sola
  vez cada pase que entra, sin contar de nuevo los reingresos.
- Incorporar persistencia de datos e interfaz de uso; actualmente `main.c` solo
  imprime el mensaje de inicio.
- Evitar que eliminar un cliente o limpiar pases vencidos libere objetos que aun
  estan referenciados por reservas o solicitudes en espera.
- Corregir `eliminarPase`: actualmente quita el pase de la lista, pero no libera ni
  devuelve el pase separado.
- Agregar pruebas automatizadas permanentes para ventas, reservas, fila, pases,
  fechas y horarios.

## Verificacion realizada

- Compilacion sintactica de todos los archivos C con `-std=c99 -Wall -Wextra`.
- Prueba temporal de ingreso con `NodoPase`, prioridad/estado del pase y recaudacion
  compartida de ventas de libros y pases. La prueba aun no forma parte del repositorio.
