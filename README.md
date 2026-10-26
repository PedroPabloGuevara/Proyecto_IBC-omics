# Proyecto IBC-omics

Proyecto de consola escrito en C. El codigo modela algunas operaciones para una tienda con libros, salas tematicas, clientes y pases.

> **Estado actual:** el punto de entrada `main.c` solo imprime `Aplicacion iniciada`. Las estructuras y funciones descritas abajo son componentes del proyecto, pero todavia no estan conectadas en un menu o flujo completo de uso. La aplicacion no guarda datos en archivos ni en una base de datos.

## Requisitos

- Windows.
- [MSYS2](https://www.msys2.org/) con el entorno **UCRT64**.
- GCC para UCRT64.
- Git Bash o la terminal UCRT64 para ejecutar los comandos.

## Instalar GCC con MSYS2

1. Instala MSYS2 desde [msys2.org](https://www.msys2.org/).
2. Abre **MSYS2 UCRT64** desde el menu Inicio.
3. Actualiza los paquetes:

   ```bash
   pacman -Syu
   ```

   Si la terminal se cierra o solicita reiniciarla, vuelve a abrir **MSYS2 UCRT64** y ejecuta:

   ```bash
   pacman -Su
   ```

4. Instala GCC:

   ```bash
   pacman -S --needed mingw-w64-ucrt-x86_64-gcc
   ```

5. Comprueba que el compilador esta disponible:

   ```bash
   gcc --version
   ```

Si usas Git Bash y `gcc` no se reconoce, agrega GCC de MSYS2 al `PATH` de esa terminal (ubicacion predeterminada):

```bash
export PATH="/c/msys64/ucrt64/bin:$PATH"
gcc --version
```

Tambien puedes ejecutar los comandos desde la terminal **MSYS2 UCRT64**, donde normalmente GCC ya esta configurado.

## Compilar

Abre Git Bash o MSYS2 UCRT64 y cambia al directorio del proyecto:

```bash
cd /c/dev/Proyecto_IBC-omics
```

Para revisar errores de sintaxis sin generar un ejecutable:

```bash
gcc -std=c11 -Wall -Wextra -fsyntax-only main.c clientes.c CrearSalas.c structibros.c funcionesLibro.c
```

Para compilar y crear `app.exe`:

```bash
gcc -std=c11 -Wall -Wextra -o app.exe main.c clientes.c CrearSalas.c structibros.c funcionesLibro.c
```

Las opciones `-std=c11`, `-Wall` y `-Wextra` seleccionan el estandar C11 y habilitan advertencias utiles durante el desarrollo.

## Ejecutar

Desde la misma terminal y directorio:

```bash
./app.exe
```

Por ahora, la salida esperada es:

```text
Aplicacion iniciada
```

El programa aun no muestra un menu ni permite administrar libros o clientes desde la consola.

## Componentes y estructuras

Las estructuras compartidas se declaran en `estructuras.h`:

- **Libros:** `Libro` guarda codigo, nombre, precio, ejemplares, publicacion, genero, fecha, disponibilidad, clasificacion por edad y si pertenece a una coleccion.
- **Arbol de libros:** `NodoLibro` enlaza cada libro con sus hijos izquierdo y derecho. Los libros se organizan por codigo.
- **Clientes:** `Cliente` guarda nombre, RUT y un arreglo de hasta 50 punteros a pases. `NodoCliente` permite enlazarlos.
- **Salas tematicas:** `SalaTematica` guarda tematica, codigo, aforo aproximado y la raiz de un arbol de libros. `NodoSala` contiene enlaces anterior y siguiente.
- **Pases y reservas:** `Pase`, `NodoPase`, `Fila`, `pasesDelDia`, `Reserva` y `NodoReserva` representan datos y enlaces para pases, filas y reservas.
- **Tienda:** `Tienda` agrupa referencias a salas, clientes y filas, junto con la recaudacion y datos del horario y la fecha.

## Algoritmos y operaciones implementados

### Libros

`structibros.c` y `funcionesLibro.c` implementan la creacion de libros y nodos, y un **arbol binario de busqueda (ABB)** ordenado por codigo:

- Insercion recursiva; no acepta codigos duplicados.
- Busqueda por codigo, existencia y conteo de libros.
- Recorrido en preorden para mostrar codigo y nombre.
- Modificacion del precio, venta de ejemplares y recepcion de despachos.
- Marcado de libros como descatalogados.
- Eliminacion del arbol: contempla nodos hoja, nodos con un hijo y nodos con dos hijos. Para dos hijos utiliza el sucesor (el menor nodo del subarbol derecho).
- Liberacion de memoria dinamica del libro y sus textos al eliminarlo.

El ABB no esta balanceado, por lo que busquedas e inserciones pueden degradarse hasta tiempo lineal si los codigos se insertan en un orden desfavorable.

### Clientes y salas

- `clientes.c` implementa creacion, busqueda, alta, modificacion y listado de clientes en una **lista enlazada circular simple**. La eliminacion ajusta los enlaces para conservar el ciclo.
- `CrearSalas.c` implementa la creacion de salas y nodos, y la insercion al final de una **lista doblemente enlazada**.

Estas funciones todavia no se invocan desde `main.c`. Las estructuras de pases, filas y reservas estan declaradas, pero no hay un flujo de aplicacion completo para gestionarlas.

## Archivos principales

| Archivo | Responsabilidad |
| --- | --- |
| `main.c` | Punto de entrada actual; imprime un mensaje de inicio. |
| `estructuras.h` | Definiciones compartidas de las estructuras de datos. |
| `structibros.c` | Creacion, busqueda y obtencion del menor nodo del arbol de libros. |
| `funcionesLibro.c` | Operaciones sobre el arbol binario de busqueda de libros. |
| `funcionesLibro.h` | Declaraciones publicas de las funciones de libros. |
| `clientes.c` | Operaciones sobre clientes y su lista circular. |
| `CrearSalas.c` | Creacion e insercion de salas tematicas. |

## Problemas frecuentes

- **`gcc: command not found`:** abre MSYS2 UCRT64 o agrega `C:/msys64/ucrt64/bin` al `PATH` de Git Bash, como se indico arriba.
- **`undefined reference to WinMain` o `undefined reference to main`:** el enlazador no encontro una funcion `main`. Comprueba que `main.c` defina `int main(void)` y que este incluido en el comando de compilacion.
- **El comando de sintaxis no genera `app.exe`:** es el comportamiento esperado; para crear el ejecutable usa el comando de compilacion de la seccion anterior.
