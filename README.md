# tpaServer
Servidor integrado com banco de dados SQL, comunicação com dispositivo embarcado ESP32 e aplicativo mobile. Em desenvolvimento utilizando Mongoose Web Server Library. Parte de projeto de iniciação científica e Trabalho de Conclusão de Curso. Engenharia de Computação - Universidade Federal de Santa Maria.



Código-fonte do servidor:  mg_http_RESTful_server.c
                           dbFunctions.c

Para Ubuntu:
1 - Incluir os arquivos mysql.h, cJSON.c, cJSON.h, mongoose.c e mongoose.h no mesmo diretório do código fonte.


2 - Instalar o XAMPP para utilizar o MySQL
    https://www.apachefriends.org/pt_br/index.html


3 - Instalar o MySQL Connector C no Ubuntu
    sudo apt update
    sudo apt upgrade
    sudo apt install libmysqlclient-dev

    ** se necessitar, descompacte tar.gz do mysql connector no diretório do servidor, depois inclua na linha de compilação
    exemplo (adequar caminhos de diretórios ao seu caso):
         compilar:
           gcc -o server mg_http_RESTful_server.c -I/usr/include/mysql -L/home/guilherme/proj/mysql-connector-c-6.1.11-linux-glibc2.12-x86_64 -lmysqlclient
         rodar:
           ./server

4 - Acessar pasta de instalação do XAMPP (meu caso: Computador/opt/lampp)


5 - Executar o XAMPP: 
      sudo ./manager-linux-x64.run

      
6 - Dar Start no Apache Web Server e MySQL Database


7 - Acessar o Web Server http://localhost/phpmyadmin/


8 - Importar o banco de dados disp_rastreador.sql


9 - compilar e rodar o servidor:
    gcc -o server mg_http_RESTful_server.c -I/usr/include/mysql -lmysqlclient
    ./server


10 - testar utilizando Postman / ESP / aplicativo

API conforme documentação

