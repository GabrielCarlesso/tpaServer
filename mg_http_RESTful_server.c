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
int dbWrite(MYSQL *connection, int lenght, cJSON *dataAddressArray[]);
int dbRead(MYSQL *connection, int rType, cJSON *dataAddressArray[]);
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
        const char *method = hm->method.ptr;
        char tokenStr[32];
        cJSON *root, *values[4], *nameItem, *emailItem, *passwordItem;


        printf("\n\t...message...\n%s\n", hm->message.ptr);

        //rota --- /api/user
        if(mg_http_match_uri(hm, "/api/user")){

            switch(method[0]){

                case 'G': // GET
                    printf("get user info (parâmetro user.id)\n");
                // Tratar requisição GET
                // ...
                    mg_http_reply(c, 200, "", "{\"result\": \"GETEI no switch \"}\n");
                    break;
                
                case 'P': // POST - Registrar usuário
                    root = cJSON_Parse(hm->body.ptr);

                    if (checkParse(root) == 0) {
                        return;
                    }
                    //printf("teste: %s\n", cJSON_GetObjectItem(root, "name")->valuestring);
                    values[0] = cJSON_GetObjectItem(root, "name");;

                    values[1] = cJSON_GetObjectItem(root, "email");

                    values[2] = cJSON_GetObjectItem(root, "password");
                    
                    // inicializa o cJSON como uma string vazia
                    values[3] = cJSON_CreateString(""); 
                    
                    // libera a memória alocada para o objeto cJSON
                    cJSON_Delete(root);

                    // registra usuário se não houver existente com o email informado
                    if (dbRead(dbConnection, 1, values) == 0) {
                        
                        // converte o token em uma string
                        snprintf(tokenStr, sizeof(tokenStr), "%ld", generateToken());

                        // insere os dados no banco
                        if(dbWrite(dbConnection, 4, values) == 0){
                            printf("Usuário registrado com sucesso!\n");
                            //sucesso, reply token
                        }
                    }
                    else{
                        printf("Já existe conta registrada com o email informado!\n");
                        //reply alguma coisa
                    }
                    break;
                    
                    //mg_http_reply(c, 200, "", "{\"result\": \"POSTEI o resource\"}\n");

                case 'D': //DELETE
                    printf("deletar usuario e todos os dados associados\n");
                    // ...
                    mg_http_reply(c, 200, "", "{\"result\": \"olá Kraemer\"}\n");
                    break;
            }
        }
        //rota --- /api/user/device
        else if(mg_http_match_uri(hm, "/api/user/device")){

            switch(method[0]){
                case 'G': //GET
                    printf("get all devices from user.id\n");
                    mg_http_reply(c, 200, "", "{\"result\": \"GETEI a data\"}\n");
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
    if (mysql_query(conexao, "show tables")) {
      fprintf(stderr, "%s\n", mysql_error(conexao));
      mysql_close(conexao);
      exit(1);
    }
    res = mysql_use_result(conexao);

    /* output table name */
    printf("MySQL Tables in mysql database:\n");
    while ((row = mysql_fetch_row(res)) != NULL)
      printf("%s \n", row[0]);

    /* close connection 
    mysql_free_result(res);
    mysql_close(conn); */
  }

  return conexao;
}

// cJSON *dataArray
int dbWrite(MYSQL *connection, int lenght, cJSON *dataAddressArray[]){

    char query[250];

    // lenght representa a quantidade de argumentos que serão inseridos no bando de dados
    switch (lenght){
        
        // device register (MAC, user.id)
        case 2:
            //int userId = getUserId;
            //sprintf(query, "INSERT INTO device(MAC, user.id) VALUES ('%s', '%s');", data[0], data[1]);
            break;

        // user register (name, email, password)
        case 4:
            printf("cheguei no dbWrite 4\n");
            //printf("teste: %s\n", dataAddressArray[0]->valuestring);
            sprintf(query, "INSERT INTO user(name, email, password, token) VALUES ('%s', '%s', '%s', '%s');", 
                dataAddressArray[0]->valuestring, dataAddressArray[1]->valuestring, dataAddressArray[2]->valuestring, dataAddressArray[3]->valuestring);
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
        return 1;
    }
    else {

        printf("\nDados inseridos com sucesso!\n");
        return 0;
    }   
}

int dbRead(MYSQL *connection, int rType, cJSON *dataAddressArray[]) {

    MYSQL_RES *answer;
    MYSQL_ROW row;
    char query[250];

    // switch utilizado para montagem da query
    switch (rType) {

        case 1:
            sprintf(query, "SELECT id FROM user WHERE email = '%s';", dataAddressArray[1]->valuestring);
            break;
            
    }

    if (mysql_query(connection, query)) {

        printf("\nErro ao consultar no banco de dados!\n");
        erro(connection);
        return 1;
    }
    else {

        printf("\nConsulta realizada com sucesso!\n");
    }

    /* if (mysql_query(connection, "SELECT * FROM disp")){
        printf("Erro na leitura!\n");
        erro(connection);
    } */

    answer = mysql_store_result(connection);
    
    if (answer == NULL) {

        printf("Retorno nulo!\n");
        erro(connection);
    }
    else {

    }


    /* row = mysql_fetch_row(answer);

    for (int i = 0; ) {

    } */
    
    mysql_free_result(answer);
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

