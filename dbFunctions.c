/* Arquivo com funções relacionadas ao banco de dados */

#include <stdio.h>
#include <stdlib.h>
#include <mysql.h>
//#include <time.h>
#include <string.h>

void errorMsg(MYSQL *connection) {
    fprintf(stderr, "\n%s\n", mysql_error(connection));
    mysql_close(connection);
    exit(1);
}

MYSQL * getConnection(){  
    MYSQL *conexao;
    MYSQL_RES *res;
    MYSQL_ROW row;
    char *server = "127.0.0.1";
    char *user = "root";
    char *password = ""; /* set me first */
    char *database = "disp_rastreador";
  
    conexao = mysql_init(NULL);

    /* Connect to database */
    if (!mysql_real_connect(conexao, server, user, password, database, 0, NULL, 0)) {
        printf("Erro na conexao!\n");
        errorMsg(conexao);
    } 
    else {
        printf("Conexao realizada com sucesso!\n");
    }

    return conexao;
}

// cJSON *dataArray
int dbWrite(MYSQL *connection, int wType, cJSON *data){

    char query[250];

    // wType representa o tipo de escrita a ser feita
    switch (wType){
        
        // user register (name, email, password, token)
        case 1:
            sprintf(query, "INSERT INTO user(name, email, password, token) VALUES ('%s', '%s', '%s', '%d');", 
                cJSON_GetObjectItem(data, "name")->valuestring, 
                cJSON_GetObjectItem(data, "email")->valuestring, 
                cJSON_GetObjectItem(data, "password")->valuestring,
                cJSON_GetObjectItem(data, "token")->valueint);
            break;
    
        // device register (MAC, user.id)
        case 2:
            sprintf(query, "INSERT INTO device(MAC, userID) VALUES ('%s', '%i');",
                cJSON_GetObjectItem(data, "MAC")->valuestring,
                cJSON_GetObjectItem(data, "userID")->valueint);
            break;

        // data register (deviceID, dateTime, longitude, latitude, acx, acy, acz, gyx, gyy, gyz)
        case 3:
            sprintf(query, "INSERT INTO data(deviceID, dateTime, longitude, latitude, acx, acy, acz, gyx, gyy, gyz) VALUES ('%i', '%s', '%f', '%f', '%f', '%f', '%f', '%f', '%f', '%f');", 
                cJSON_GetObjectItem(data, "deviceID")->valueint,
                cJSON_GetObjectItem(data, "dateTime")->valuestring,
                cJSON_GetObjectItem(data, "longitude")->valuedouble,
                cJSON_GetObjectItem(data, "latitude")->valuedouble,
                cJSON_GetObjectItem(data, "acx")->valuedouble,
                cJSON_GetObjectItem(data, "acy")->valuedouble,
                cJSON_GetObjectItem(data, "acz")->valuedouble,
                cJSON_GetObjectItem(data, "gyx")->valuedouble,
                cJSON_GetObjectItem(data, "gyy")->valuedouble,
                cJSON_GetObjectItem(data, "gyz")->valuedouble);
            break;
    }

    printf("%s\n", query);
    if (mysql_query(connection, query)) {

        printf("\nErro ao inserir no banco de dados!\n");
        errorMsg(connection);
        return 0;
    }
    else {

        printf("\nDados inseridos com sucesso!\n");
        return 1;
    }   
}

int dbRead(MYSQL *connection, int rType, cJSON *data) {

    MYSQL_RES *res;
    MYSQL_ROW row;
    char query[250];
    int value;
    unsigned int num_fields, i;
    unsigned long *lengths;

    // switch utilizado para montagem da query
    switch (rType) {

        case 1:
            sprintf(query, "SELECT id FROM user WHERE email = '%s';", cJSON_GetObjectItem(data, "email")->valuestring);
            break;

        case 2:
            sprintf(query, "SELECT id FROM user WHERE token = '%s';", cJSON_GetObjectItem(data, "token")->valuestring);
            break;  

        case 3:
            sprintf(query, "SELECT id FROM device WHERE userID = %i AND MAC = '%s';", 
                cJSON_GetObjectItem(data, "userID")->valueint, 
                cJSON_GetObjectItem(data, "MAC")->valuestring);
            break;

        case 4:
            sprintf(query, "SELECT name, email FROM user WHERE id = %i;", cJSON_GetObjectItem(data, "userID")->valueint);
            break;  
    }
    printf("query %s", query);

    if (mysql_query(connection, query)) {
        printf("\nErro ao consultar no banco de dados!\n");
        errorMsg(connection);
        return 0;
    }

    printf("\nConsulta realizada com sucesso!\n");
    
    res = mysql_store_result(connection);
    
    if (res == NULL) {
        errorMsg(connection);
        mysql_free_result(res);
        return 0;
    }
    else {
        if (rType == 2 || rType == 3) {
            row = mysql_fetch_row(res);
            value = atoi(row[0]);
            mysql_free_result(res);
            return value;
        }
        else{
            num_fields = mysql_num_fields(res);
            
            while ((row = mysql_fetch_row(res))) {
                
                lengths = mysql_fetch_lengths(res);
                for(i = 0; i < num_fields; i++) {
                    printf("[%.*s] ", (int) lengths[i],
                        row[i] ? row[i] : "NULL");
                }
                printf("\n");
            }
        }
    }
    
    mysql_free_result(res);
    return (-1);
}