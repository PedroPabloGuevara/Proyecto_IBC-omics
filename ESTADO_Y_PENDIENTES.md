# Estado de la aplicacion

## Implementado en esta rama

- Estructuras de datos para clientes, pases diarios, reservas de libros, salas y tienda.
- Registro de clientes y almacenamiento de hasta 50 pases por cliente.
- Creacion, venta, consulta y administracion de pases diarios, con codigo individual,
  fecha, hora y recaudacion.
- Reservas de libros vinculadas a cada libro, con prioridad para reservas de tipo 4.
- Gestion de libros en arbol binario: busqueda, ventas por cantidad, despacho,
  descatalogacion y recaudacion diaria.
- Fila de ingreso separada de las colas de reservas. Cada solicitud conserva la
  persona, el pase presentado y la sala preferida.
- Prioridad estable para los pases de tipo 4 y orden de llegada para los demas.
- Validacion de fecha `DD/MM/AAAA`, hora `HH:MM`, horario del pase y horario de tienda.
- Control de capacidad de la sala principal mas la suma de las capacidades tematicas,
  admision, espera cuando no hay espacio, registro de salida, reingreso con el mismo
  pase y cambio de sala sujeto a capacidad.
- Modulos de fila divididos por responsabilidad en `src/filas/`.

## Pendiente antes de considerar completa la aplicacion

- Conectar las operaciones de clientes, pases, libros, salas y fila desde `main.c`.
- Incorporar persistencia de datos e interfaz de uso; actualmente `main.c` solo
  imprime el mensaje de inicio.
- Definir como se conserva la ocupacion cuando se modifica o elimina una sala.
- Evitar que eliminar un cliente o limpiar pases vencidos libere objetos que aun
  estan referenciados por solicitudes en espera, ocupaciones o registros de uso.
- Sincronizar cambios de RUT con el historial de uso del pase, o impedir modificarlo
  mientras existan referencias activas.
- Corregir `eliminarPase`: actualmente quita el pase de la lista, pero no libera ni
  devuelve el pase separado.
- Validar que el aforo de las salas sea positivo al crearlas o modificarlas.
- Definir y mostrar explicitamente el estado de un pase ya utilizado; el control
  actual registra su uso internamente, pero no cambia el campo `estado` del pase.
- Evitar configuraciones repetidas del control de acceso que reemplacen estructuras
  existentes y pierdan la ocupacion o las solicitudes activas.
- Agregar pruebas automatizadas permanentes para ventas, reservas, fila, aforos,
  pases, fechas y horarios. La compilacion C89 por si sola no sustituye esas pruebas.

## Verificacion realizada

- Compilacion y enlace de todos los archivos C con `-std=c89 -Wall -Wextra -Werror
  -pedantic`.
- Pruebas puntuales ejecutadas durante el desarrollo para prioridad de fila,
  capacidades, fechas y horas, salidas, reingresos y cambios de sala. Estas pruebas
  se ejecutaron como programas temporales y aun no forman parte del repositorio.
