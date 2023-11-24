/* Arquivo com funções relacionadas ao banco de dados */

#include <stdio.h>
#include <stdlib.h>
#include <mysql.h>
#include <string.h>
#include "mongoose.h"
#include "mongoose.c"
#include "cJSON.h"
#include "cJSON.c"

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

int dbWrite(MYSQL *connection, int wType, cJSON *data){

    char query[250];

    // wType representa o tipo de escrita a ser feita
    switch (wType){
        
        // registro de usuário (name, email, password, token)
        case 1:
            sprintf(query, "INSERT INTO user(name, email, password, token) VALUES ('%s', '%s', '%s', '%s');", 
                cJSON_GetObjectItem(data, "name")->valuestring, 
                cJSON_GetObjectItem(data, "email")->valuestring, 
                cJSON_GetObjectItem(data, "password")->valuestring,
                cJSON_GetObjectItem(data, "token")->valuestring);
            break;
    
        // registro de dispositivo (MAC, user.id)
        case 2:
            sprintf(query, "INSERT INTO device(MAC, userID) VALUES ('%s', '%i');",
                cJSON_GetObjectItem(data, "MAC")->valuestring,
                cJSON_GetObjectItem(data, "userID")->valueint);
            break;

        // registro de dados (deviceID, dateTime, longitude, latitude, acx, acy, acz, gyx, gyy, gyz)
        case 3:
            sprintf(query, "INSERT INTO data(deviceID, dateTime, longitude, latitude, acx, acy, acz, gyx, gyy, gyz) VALUES ('%i', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s');", 
                cJSON_GetObjectItem(data, "deviceID")->valueint,
                cJSON_GetObjectItem(data, "dateTime")->valuestring,
                cJSON_GetObjectItem(data, "longitude")->valuestring,
                cJSON_GetObjectItem(data, "latitude")->valuestring,
                cJSON_GetObjectItem(data, "acx")->valuestring,
                cJSON_GetObjectItem(data, "acy")->valuestring,
                cJSON_GetObjectItem(data, "acz")->valuestring,
                cJSON_GetObjectItem(data, "gyx")->valuestring,
                cJSON_GetObjectItem(data, "gyy")->valuestring,
                cJSON_GetObjectItem(data, "gyz")->valuestring);
            break;

        // registro de coordenadas para cercamento virtual
        case 4:
            sprintf(query, "UPDATE user SET vfCoordinates = '%s' WHERE token = '%s';", cJSON_GetObjectItem(data, "coordinates")->valuestring,
                cJSON_GetObjectItem(data, "token")->valuestring);
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
    MYSQL_FIELD *current_field, *fields;
    char query[250];
    char *field_name, *field_value;
    int value;
    unsigned int num_fields, i;
    unsigned long *lengths;
    cJSON *dataArray;
    cJSON *rowObject;
    

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
            sprintf(query, "SELECT name, email, vfCoordinates FROM user WHERE id = %i;", cJSON_GetObjectItem(data, "userID")->valueint);
            break;  

        case 5:
            sprintf(query, "SELECT MAC FROM device WHERE userID = %i;", cJSON_GetObjectItem(data, "userID")->valueint);
            break;

        case 6:
            sprintf(query, "SELECT id FROM user WHERE email = '%s' and password = '%s';", 
                cJSON_GetObjectItem(data, "email")->valuestring,
                cJSON_GetObjectItem(data, "password")->valuestring);
            break;
        
        case 7:
            sprintf(query, "SELECT dateTime, longitude, latitude, acx, acy, acz, gyx, gyy, gyz FROM data WHERE deviceID = %i;", cJSON_GetObjectItem(data, "deviceID")->valueint);
            break;

    }
    printf("query %s", query);

    if (mysql_query(connection, query)) {
        printf("\nErro ao consultar no banco de dados!\n");
        errorMsg(connection);
        return 0;
    }

    printf("\nConsulta realizada com sucesso!\n");
    
    res = mysql_use_result(connection);
    
    if (res == NULL) {
        printf("res = NULL\n");
        errorMsg(connection);
        mysql_free_result(res);
        return 0;
    }
    else {
        if ((rType == 2) || (rType == 3) || (rType == 6)) {
            row = mysql_fetch_row(res);
            value = atoi(row[0]);
            mysql_free_result(res);
            return value;
        }
        else {
            num_fields = mysql_num_fields(res);
            
            fields = mysql_fetch_fields(res);

            dataArray = cJSON_CreateArray();
            
            while ((row = mysql_fetch_row(res))) {
                //lengths = mysql_fetch_lengths(res);
                rowObject = cJSON_CreateObject();

                for (i = 0; i < num_fields; i++) {

                    field_name = fields[i].name;
                    field_value = row[i] ? row[i] : "NULL";
                    
                    //printf("[%s : %s] ", field_name, field_value);
                    
                    if(cJSON_AddStringToObject(rowObject, field_name, field_value) == NULL){
                        printf("Erro ao adicionar ao objeto cJSON rowObject.\n");
                    }
                }
                cJSON_AddItemToArray(dataArray, rowObject);
                printf("\n");
                
            }
            cJSON_AddItemToObject(data, "rows", dataArray);
        }
    }
    
    mysql_free_result(res);
    return (-1);
}

int dbDelete(MYSQL *connection, int dType, cJSON *data) {

    char query[250];

    switch (dType) {

        case 1:
            sprintf(query, "DELETE FROM user WHERE id = %d;", cJSON_GetObjectItem(data, "userID")->valueint);
            break;

        case 2:
            sprintf(query, "DELETE FROM device WHERE MAC = '%s' AND userID = %d;", cJSON_GetObjectItem(data, "MAC")->valuestring, cJSON_GetObjectItem(data, "userID")->valueint);
            break;
        
        case 3:
            sprintf(query, "DELETE FROM data WHERE deviceID = %d;", cJSON_GetObjectItem(data, "deviceID")->valueint);
            break;
    }

    printf("%s\n", query);

    if (mysql_query(connection, query)) {

        printf("\nErro ao excluir do banco de dados!\n");
        errorMsg(connection);
        return 0;
    }
    else {

        printf("\nDados excluídos com sucesso!\n");
        return 1;
    }   
}