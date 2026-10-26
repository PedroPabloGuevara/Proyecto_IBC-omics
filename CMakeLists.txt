cmake_minimum_required(VERSION 3.16)
project(Proyecto_IBC_omics C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(app
    main.c
    clientes.c
    CrearSalas.c
    structibros.c
    funcionesLibro.c
    cola.c
)
