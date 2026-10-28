# Estado de la aplicacion

## Implementado en esta rama

- Estructuras de datos para clientes, pases diarios, reservas de libros, salas y tienda.
- Registro de clientes y almacenamiento de hasta 50 pases por cliente.
- Registro de pases externos asociados a clientes para validar su ingreso.
- `Tienda.recaudacion` registra las ventas de libros; la aplicacion no vende
  pases ni suma dinero por pases.
- Reservas solo para libros en preventa (disponibilidad 3), con prioridad para
  reservas de tipo 4. Al llegar despacho, el libro queda disponible y la venta
  atiende la cola: solo el primer reservador puede comprar su ejemplar reservado.
- Gestion de libros en arbol binario: busqueda, ventas por cantidad, despacho,
  descatalogacion y actualizacion de la recaudacion de la tienda.
- Modulo de libros separado entre operaciones del arbol (`operacionesArbol.c`), creacion y
  liberacion de libros (`libros.c`) y ventas/recaudacion (`ventasLibros.c`).
- Fila de ingreso de pases basada en `Fila` y `NodoPase`, separada de las reservas
  de libros; los pases tipo 4 tienen prioridad estable y los demas conservan FIFO.
- Validacion de fecha `DD/MM/AAAA`, hora actual `HH:MM` y horario de tienda.
  La hora guardada en el pase representa su compra y no afecta el ingreso.
- Al admitir por primera vez un pase, se registra en `Tienda.filaPasesDeldia` y su
  estado cambia a utilizado. Esto cuenta entradas unicas, pero no administra
  ocupacion actual, salidas ni asignacion de personas a salas.
- Modulo de entrada agrupado en `src/entrada/`: fila, gestion, control y validacion
  (`validacionEntrada.c`).

## Pases externos y registro diario de ingresos

- `registrarPaseCliente` asocia con un cliente un pase externo activo que cumple
  las condiciones basicas de fecha, codigo y horario. No realiza una venta.
- `Tienda.filaPasesDeldia` guarda una copia de cada ticket que ingresa por primera
  vez. La lista permanece durante la jornada y permite contar ingresos unicos.
- Los tickets se agregan en estado utilizado. Un reingreso se valida contra ese
  registro, no agrega una copia nueva y no altera la recaudacion por libros.
- Los pases activos se procesan por prioridad y orden de llegada, sin considerar
  la hora de compra. Los utilizados pueden reingresar durante el horario de
  tienda si ya constan en el registro diario; anulados o vencidos no se admiten.
- El registro diario contiene copias independientes: eliminar al cliente y sus
  credenciales no borra el conteo de ingresos unicos de la jornada.

## Cierre de jornada

- `cerrarTienda` libera la fila de ingreso, el registro diario y las credenciales
  de pase asociadas a clientes.
- Clientes, salas tematicas, libros y reservas se conservan al cierre para que
  puedan seguir utilizandose despues de terminar la jornada. Los clientes pueden
  registrar pases nuevos al comenzar otro dia.
- La recaudacion final se devuelve al llamador y permanece en
  `Tienda.recaudacion`. El cierre no la reinicia ni guarda datos en disco.
- La aplicacion actual solo ejecuta un dia por proceso. Al terminar el proceso
  no se persiste el estado; al abrirlo de nuevo se debe crear una tienda nueva
  e ingresar la fecha de esa jornada.
- `main.c` aun no tiene el flujo de uso que invoque este cierre; debe llamar a
  `cerrarTienda` al finalizar la jornada y mostrar la recaudacion devuelta.

## Pendiente antes de considerar completa la aplicacion

- Conectar las operaciones de clientes, pases, libros, salas y fila desde `main.c`.
- Invocar el cierre de jornada desde el flujo final de `main.c`.
- Incorporar el aviso/interfaz para comunicar a los clientes cuando una reserva
  pueda ser atendida; por ahora el orden se hace cumplir al registrar la venta.
- Incorporar persistencia de datos e interfaz de uso; actualmente `main.c` solo
  imprime el mensaje de inicio.

## Verificacion realizada

- Compilacion sintactica de todos los archivos C con `-std=c99 -Wall -Wextra`.
- Pruebas de regresion permanentes en `tests/pruebas_regresion.c`, ejecutables
  con `powershell -NoProfile -ExecutionPolicy Bypass -File tests/run_tests.ps1`.
- `tests/main_flujo.c` contiene un main exclusivo de integracion que recorre una
  jornada de prueba; no reemplaza ni conecta el `main.c` de la aplicacion.
- La suite cubre validacion de fechas/horas, limites y duplicados, prioridad y
  orden de fila, ingreso/reingreso, ventas y recaudacion, reservas solo en
  preventa, prioridad de venta tras despacho, datos invalidos de libros,
  eliminacion de referencias, despacho, descatalogacion y cierre que conserva
  clientes, salas, libros y reservas, pero limpia los pases diarios.
