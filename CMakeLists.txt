cmake_minimum_required(VERSION 3.16)
project(OS-labs C)

set(CMAKE_C_STANDARD 11)

#ENV
add_executable(lab2 lab2/lab2.c)
add_executable(lab3 lab3/lab3.c)
add_executable(lab4 lab4/lab4.c)

##IO operations
add_executable(lab5 lab5/lab5.c)
add_executable(lab5FileGen lab5/fileGenerator.c)
add_executable(lab6 lab6/lab6.c)
add_executable(lab7 lab7/lab7.c)

##FORK EXEC
add_executable(lab8 lab8/lab8.c)
add_executable(lab9 lab9/lab9.c)
add_executable(lab10 lab10/lab10.c)
add_executable(lab11 lab11/lab11.c)

#TERMINALS
add_executable(lab16 lab16/lab16.c)
add_executable(lab17 lab17/lab17.c)

#FILES
add_executable(lab18 lab18/lab18.c)

#CATALOGS
add_executable(lab19 lab19/lab19.c)
add_executable(lab20 lab20/lab20.c)

#SIGNALS
add_executable(lab21 lab21/lab21.c)
add_executable(lab22 lab22/lab22.c)

##PIPES
add_executable(lab25 lab25/lab25.c)
add_executable(lab26 lab26/lab26.c)
add_executable(lab26-2 lab26/lab26-2.c)
add_executable(lab27 lab27/lab27.c)

#SOCKETS
add_executable(lab30-server lab30/lab30-server.c)
add_executable(lab30-client lab30/lab30-client.c)
add_executable(lab31-server lab31/lab31-server.c)
add_executable(lab31-client lab31/lab31-client.c)
add_executable(lab32-server lab32/lab32-server.c)
add_executable(lab32-client lab32/lab32-client.c)
add_executable(lab33-server lab33/lab33-server.c)
add_executable(lab33-client lab33/lab33-client.c)
add_executable(lab33-proxy lab33/lab33-proxy.c)
target_link_libraries(lab33-proxy m)