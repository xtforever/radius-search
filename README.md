# radius-search
A very simple example for radius search. 
This application reads postal codes and geographic coordinats from a text file.
it can calculate the distance beetween two geographic coordinats and it
can do a fast search on nearby coordinates to do a radius search.
The program reads commands from stdin. the commands are:

- u <postal-code> <radius-km>
- p <postal-code>
- r <lon> <lat> <radius-km>


example;
  ./geo1
   u 26603 10


Quickstart:
 
    sh ./build.sh

    type:
          u 26603 10
          p 26603
          r 10.1 53.01 20


Use websocketd to create a microservice for your web application

    websocketd --port=8080 --devconsole ./geo1

