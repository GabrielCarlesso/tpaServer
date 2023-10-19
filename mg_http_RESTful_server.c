#include <stdio.h>
#include <stdlib.h>
#include <mysql.h>
#include <time.h>
#include "mongoose.h"
#include "mongoose.c"
#include "cJSON.h"
#include "cJSON.c"

/* Funções relacionadas ao banco de dados */
MYSQL * getConnection();
int dbWrite(MYSQL *connection, int lenght, cJSON *data);
int dbRead(MYSQL *connection, int rType, cJSON *data);
int checkParse (cJSON *msg);
long int generateToken();
void erro(MYSQL *connection);

MYSQL *dbConnection;

static const char *s_http_addr = "http://127.0.0.1:8000";    // HTTP port
static const char *s_https_addr = "https://127.0.0.1:443";  // HTTPS port
static const char *s_root_dir = "."; // diretório raiz onde os arquivos estáticos estão

// We use the same event handler function for HTTP and HTTPS connections
// The function is called every time an event happens
// fn_data is NULL for plain HTTP, and non-NULL for HTTPS

static void event_handler(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    
    if(ev == MG_EV_ACCEPT && fn_data != NULL) { // Identifies HTTPS connection
        
        // Defines communication security options (TLS/SSL Protocol)
        struct mg_tls_opts opts = {
            //.ca = "ca.pem",         // Uncomment to enable two-way SSL
            .cert = "server.pem",     // (Certificate PEM file) OR (Path to the PEM file containing the server's certificate)
            .certkey = "server.pem",  // (This PEM contains both cert and key) OR (Path to the PEM file containing both the server's certificate and private key.)
        };
        mg_tls_init(c, &opts); // Set the connection's configuration to use TLS/SSL 
    } 
    else if(ev == MG_EV_HTTP_MSG) { // Identifies HTTP connection
        
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;
        //struct mg_str *token = mg_http_get_header(hm, "Authorization");
        const char *method = hm->method.ptr;
        //char tokenStr[32];
        int id;
        cJSON *root = cJSON_Parse(hm->body.ptr);

        //root = cJSON_Parse(hm->body.ptr);


        printf("\n\t...message...\n%s\n", hm->message.ptr);

        //rota --- /api/user
        if(mg_http_match_uri(hm, "/api/user")) {

            switch(method[0]) {

                case 'G': // GET
                    printf("get user info (parâmetro user.id)\n");
                // Tratar requisição GET
                // ...
                    mg_http_reply(c, 200, "", "{\"result\": \"GETEI no switch \"}\n");
                    break;
                
                case 'P': // POST - Registrar usuário

                    if (checkParse(root) == 0) {
                        return;
                    } 
                     
                    // converte o token em uma string e adiciona ao objeto cJSON root
                    //snprintf(tokenStr, sizeof(tokenStr), "%ld", generateToken());
                    cJSON_AddNumberToObject(root, "token", generateToken());

                    // registra usuário se não houver existente com o email informado
                    if(dbWrite(dbConnection, 1, root) == 1){
                        printf("Usuário registrado com sucesso!\n");
                        mg_http_reply(c, 200, "", "{\"result\": \"Usuário registrado com sucesso!\"}\n");
                        //sucesso, reply token
                    }
                    else{
                        printf("Já existe conta registrada com o email informado!\n");
                        mg_http_reply(c, 200, "", "{\"result\": \"Já existe conta registrada com o email informado!\"}\n");
                        //reply alguma coisa
                    }
                    cJSON_Delete(root);
                    break;

                case 'D': //DELETE
                    printf("deletar usuario e todos os dados associados\n");
                    // ...
                    mg_http_reply(c, 200, "", "{\"result\": \"olá Kraemer\"}\n");
                    break;
            }
        }
        //rota --- /api/user/device
        else if(mg_http_match_uri(hm, "/api/user/device")){

            switch(method[0]) {
                case 'G': //GET
                    printf("get all devices from user.id\n");
                    mg_http_reply(c, 200, "", "{\"result\": \"GETEI a data\"}\n");
                    break;
            }
        }
        else if(mg_http_match_uri(hm, "/api/device")) {

            switch(method[0]) {
                
                case 'D':   // delete device
                    break;
                
                case 'G':   // get device info
                    break;
                
                case 'P':   // device register

                    if (checkParse(root) == 0) {
                        return;
                    } 
                    
                    if(cJSON_GetObjectItem(root, "token")->valuestring != NULL){
                        printf("dif de null");
                    
                        //cJSON_AddStringToObject(root, "token", token->ptr);
                        
                        // verifica qual usuário possui o token informado
                        id = dbRead(dbConnection, 2, root);
                    
                        if(id != 0){

                            printf("leu id: %i\n", id);
                        
                            cJSON_AddNumberToObject(root, "user.id", id);

                            // insere os dados no banco
                            if(dbWrite(dbConnection, 2, root) == 1){
                                printf("Dispositivo registrado com sucesso!\n");
                                mg_http_reply(c, 200, "", "{\"result\": \"Dispositivo registrado com sucesso!\"}\n");
                            }
                            else {
                                mg_http_reply(c, 200, "", "{\"result\": \"Já existe dispositivo registrado com o MAC informado!\"}\n");
                            }
                        }
                        else {
                            mg_http_reply(c, 200, "", "{\"result\": \"Não há usuário com o token informado!\"}\n");
                        }
                    }
                    else {
                        mg_http_reply(c, 200, "", "{\"result\": \"Token nulo!\"}\n");
                    }
                    
                    cJSON_Delete(root);
                    break;
            }
        }
        else if(mg_http_match_uri(hm, "/api/data")){

            switch(method[0]){
                
                case 'P':
                    printf("Recebido JSON: %.*s\n", (int)hm->body.len, hm->body.ptr); //debug
                
                    root = cJSON_Parse(hm->body.ptr);

                    if (checkParse(root) == 0) {
                        return;
                    }

                    cJSON *dtItem = cJSON_GetObjectItem(root, "date_time");
                    cJSON *macItem = cJSON_GetObjectItem(root, "MAC");
                    cJSON *longItem = cJSON_GetObjectItem(root, "longitude");
                    cJSON *latItem = cJSON_GetObjectItem(root, "latitude");
                    cJSON *acxItem = cJSON_GetObjectItem(root, "acx");
                    cJSON *acyItem = cJSON_GetObjectItem(root, "acy");
                    cJSON *aczItem = cJSON_GetObjectItem(root, "acz");

                    if (dtItem->type == cJSON_String) {
                        const char *dtValue = dtItem->valuestring;
                        printf("datetime: %s\n", dtValue);
                    }
                    if (macItem->type == cJSON_String){
                        const char *macValue = macItem->valuestring;
                    }
                    if(longItem->type == cJSON_Number){
                        float longValue = longItem->valuedouble;
                    }
                    if(latItem->type == cJSON_Number){
                        float latValue = latItem->valuedouble;
                    }
                    if(acxItem->type == cJSON_Number){
                        float acxValue = acxItem->valuedouble;
                    }
                    if(acyItem->type == cJSON_Number){
                        float acyValue = acxItem->valuedouble;
                    }
                    if(aczItem->type == cJSON_Number){
                        float aczValue = acxItem->valuedouble;
                    }

                    //libera a memória alocada para o objeto cJSON
                    cJSON_Delete(root);

                    //insere os dados no banco
                    //dbWrite(dbConnection, 1, 1, (time_t) 0, 88.0, 99.0, 0.0, 1.0, 2.0);

                    mg_http_reply(c, 200, "", "{\"result\": \"Dados JSON recebidos com sucesso\"}\n");
                    break;
            }
        }
    }
    //(void) fn_data;
}

// verifica erro no cJSON_Parse()
int checkParse (cJSON *msg) {
    if (msg == NULL) {
        // Erro de parsing
        printf("Erro no parsing do JSON.\n");
        return 0;
    }
    else return 1;
}

long int generateToken(){
    srand((unsigned)time(NULL));
    return rand();
}

void erro(MYSQL *connection){
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
        erro(conexao);
    } else {
    printf("Conexao realizada com sucesso!\n");
    
    /* send SQL query */
    /* if (mysql_query(conexao, "show tables")) {
      fprintf(stderr, "%s\n", mysql_error(conexao));
      mysql_close(conexao);
      exit(1);
    }
    res = mysql_use_result(conexao);

    /* output table name */
    /*printf("MySQL Tables in mysql database:\n");
    while ((row = mysql_fetch_row(res)) != NULL)
      printf("%s \n", row[0]); */

    /* close connection 
    mysql_free_result(res);
    mysql_close(conn); */
  }

  return conexao;
}

// cJSON *dataArray
int dbWrite(MYSQL *connection, int wType, cJSON *data){

    char query[250];

    // lenght representa a quantidade de argumentos que serão inseridos no bando de dados
    switch (wType){
        
        // user register (name, email, password, token)
        case 1:
            printf("cheguei no dbWrite userRegister\n");
            sprintf(query, "INSERT INTO user(name, email, password, token) VALUES ('%s', '%s', '%s', '%d');", 
                cJSON_GetObjectItem(data, "name")->valuestring, 
                cJSON_GetObjectItem(data, "email")->valuestring, 
                cJSON_GetObjectItem(data, "password")->valuestring,
                cJSON_GetObjectItem(data, "token")->valueint);
            break;
    
        // device register (MAC, user.id)
        case 2:
            printf("cheguei no dbWrite deviceRegister\n");
            sprintf(query, "INSERT INTO device(MAC, userID) VALUES ('%s', '%i');",
                cJSON_GetObjectItem(data, "MAC")->valuestring,
                cJSON_GetObjectItem(data, "user.id")->valueint);
            break;

        // data register (device.id, dateTime, longitude, latitude, acx, acy, acz);
        case 7:
            //sprintf(query, "INSERT INTO data(device.id, dateTime, longitude, latitude, acx, acy, acz) VALUES ('%i', '%s', '%f', '%f', '%f', '%f', '%f');");
            break;
    }

    printf("%s\n", query);
    if (mysql_query(connection, query)) {

        printf("\nErro ao inserir no banco de dados!\n");
        erro(connection);
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

    // switch utilizado para montagem da query
    switch (rType) {

        case 1:
            sprintf(query, "SELECT id FROM user WHERE email = '%s';", cJSON_GetObjectItem(data, "email")->valuestring);
            break;

        case 2:
            // Agora você pode usar tokenValue na sua consulta SQL
            sprintf(query, "SELECT id FROM user WHERE token = '%s';", cJSON_GetObjectItem(data, "token")->valuestring);
            break;
            
    }
    printf("query %s", query);

    if (mysql_query(connection, query)) {
        printf("\nErro ao consultar no banco de dados!\n");
        erro(connection);
        return 0;
    }

    //printf("%s\n", query);
    printf("\nConsulta realizada com sucesso!\n");
    
    res = mysql_store_result(connection);
    
    if (res == NULL) {
        printf("cheguei 2\t");
        printf("Retorno nulo!\n");
        erro(connection);
        mysql_free_result(res);
        return 0;
    }
    else {
        printf("cheguei 3\t");
        row = mysql_fetch_row(res);
        value = atoi(row[0]);
        mysql_free_result(res);
        return value;
        
    }

    //while ((row = mysql_fetch_row(res)) != NULL)
     // printf("%s \n", row[0]); */

    /* row = mysql_fetch_row(answer);

    for (int i = 0; ) {

    } */
    
    mysql_free_result(res);
    return (-1);
}

int main(void) {
    struct mg_mgr mgr;                            // Event manager
    mg_log_set(MG_LL_DEBUG);                      // Set log level

    dbConnection = getConnection();           // Connect to database
   
    mg_mgr_init(&mgr);                            // Initialise event manager
    
    mg_http_listen(&mgr, s_http_addr, event_handler, NULL);//(void *) &connection  // Create HTTP listener
    //mg_http_listen(&mgr, s_https_addr, event_handler, (void *) 1);  // HTTPS listener
    
    for (;;) mg_mgr_poll(&mgr, 1000);                    // Infinite event loop
    mg_mgr_free(&mgr);
    
    return 0;
}

