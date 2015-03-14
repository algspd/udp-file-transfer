#Several ideas to do

  * Velocidades/tiempos
  * Al inicio probar varios tamaños de paquete o hacer que el cliente establezca el tamaño del frame
  * Implementar resume
  * Al lanzar encima de un ssh (8115):
# netstat -natup|grep 8115
tcp        0      0 0.0.0.0:8115            0.0.0.0:**LISTEN      1968/sshd
tcp6       0      0 :::8115                 :::**                    LISTEN      1968/sshd
# ./server 8115
Server started on port 8115