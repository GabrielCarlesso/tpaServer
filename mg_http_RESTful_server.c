#include <stdio.h>
#include <stdlib.h>
#include <mysql.h>
#include <time.h>
#include "mongoose.h"
#include "mongoose.c"
#include "cJSON.h"
#include "cJSON.c"

/* #include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h> */

MYSQL * obterConexao();
//void inserir(MYSQL *conexao, int id, int deviceId, time_t horario, float longitude, float latitude, float acx, float acy, float acz);
//int inserir();
void erro(MYSQL *conexao);
void dbRead(MYSQL *conexao);
MYSQL *dbConnection = obterConexao();

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
        //MYSQL *dbConnection = (MYSQL *) fn_data; 
        const char *method = hm->method.ptr;

        printf("\n\t...message...\n%s\n", hm->message.ptr);

        //printf("\n\t.. value headers[1].ptr %s\n", hm->headers[1].value.ptr);
        /* printf("\n\t..method.ptr %s\n", hm->method.ptr);
        printf("\t..uri.ptr %s\n", hm->uri.ptr); */

        /* printf("\t..query.ptr %s\n", hm->query.ptr);
        printf("\t..proto.ptr %s\n", hm->proto.ptr);
        printf("\t..body.ptr %s\n", hm->body.ptr);
        printf("\t..message.ptr %s\n", hm->message.ptr); */

        //rota --- /api/user
        if(mg_http_match_uri(hm, "/api/user")){

            switch(method[0]){

                case 'G': // GET
                    printf("get user info (parâmetro user.id)\n");
                // Tratar requisição GET
                // ...
                    mg_http_reply(c, 200, "", "{\"result\": \"GETEI no switch \"}\n");
                    break;
                
                case 'P': // POST
                    //printf("registrar user (name, email, password\n");
                    cJSON *root = cJSON_Parse(hm->body.ptr);
                    cJSON *values[4];

                    if (checkParse(root) == 0) {
                        return;
                    }

                    cJSON *values[0] = cJSON_GetObjectItem(root, "name");
                    //cJSON *nameItem = cJSON_GetObjectItem(root, "name");
                    cJSON *values[1] = cJSON_GetObjectItem(root, "email");
                    //cJSON *emailItem = cJSON_GetObjectItem(root, "email");
                    cJSON *values[2] = cJSON_GetObjectItem(root, "password");
                    //cJSON *passwordItem = cJSON_GetObjectItem(root, "password");

                   /*  if (nameItem->type == cJSON_String) {
                        //values[0] = &nameItem;
                        //values[0] = &(nameItem->valuestring);
                        values[0] = nameItem->valuestring;
                    }
                    if (emailItem->type == cJSON_String) {
                        values[1] = emailItem->valuestring;
                    }
                    if (passwordItem->type == cJSON_String) {
                        values[2] = passwordItem->valuestring;
                    } */

                    // libera a memória alocada para o objeto cJSON
                    cJSON_Delete(root);

                    //consultar banco de dados para ver se já existe conta com o email informado
                    //se não existe conta criada com o email passado, prossegue com o registro
                    if () {

                    }
                    else{
                        values[3] = itoa(generateToken());
                    }

                    // insere os dados no banco
                    if(inserir(dbConnection, 4, values) == 0){
                        //sucesso, reply token
                    }
                    
                    //mg_http_reply(c, 200, "", "{\"result\": \"POSTEI o resource\"}\n");

                case 'D': //DELETE
                    printf("deletar usuario e todos os dados associados\n");
                    // ...
                    mg_http_reply(c, 200, "", "{\"result\": \"olá Kraemer\"}\n");
            }
        }
        //rota --- /api/user/device
        else if(mg_http_match_uri(hm, "/api/user/device")){

            switch(method[0]){
                case 'G': //GET
                    printf("get all devices from user.id\n");
                    mg_http_reply(c, 200, "", "{\"result\": \"GETEI a data\"}\n");
            }
        }
        else if(mg_http_match_uri(hm, "/api/data")){

            switch(method[0]){
                
                case 'P':
                    printf("Recebido JSON: %.*s\n", (int)hm->body.len, hm->body.ptr); //debug
                
                    cJSON *root = cJSON_Parse(hm->body.ptr);

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
                    inserir(dbConnection, 1, 1, (time_t) 0, 88.0, 99.0, 0.0, 1.0, 2.0);

                    mg_http_reply(c, 200, "", "{\"result\": \"Dados JSON recebidos com sucesso\"}\n");
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

void erro(MYSQL *conexao){
    fprintf(stderr, "\n%s\n", mysql_error(conexao));
    mysql_close(conexao);
    exit(1);
}

MYSQL * obterConexao(){  
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
int inserir(MYSQL *connection, int lenght, cJSON *dataAddressArray){

    char query[250];

    // lenght representa a quantidade de argumentos que serão inseridos no bando de dados
    switch (lenght){
        
        // device register (MAC, user.id)
        case 2:
            //int userId = getUserId;
            //sprintf(query, "INSERT INTO device(MAC, user.id) VALUES ('%s', '%s');", data[0], data[1]);

        // user register (name, email, password)
        case 3:
            //sprintf(query, "INSERT INTO user(name, email, password) VALUES ('%s', '%s', '%s');", data[0], data[1], data[2]);
    
        // data register (device.id, dateTime, longitude, latitude, acx, acy, acz);
        case 7:
            //sprintf(query, "INSERT INTO data(device.id, dateTime, longitude, latitude, acx, acy, acz) VALUES ('%i', '%s', '%f', '%f', '%f', '%f', '%f');");
    }

    printf("%s\n", query);
    if (mysql_query(conexao, query))
    {
        printf("\nErro ao inserir no banco de dados!\n");
        erro(conexao);
        return 1;
    }
    else
    {
        printf("\nDados inseridos com sucesso!\n");
        return 0;
    }   
}

/* void inserir(MYSQL *conexao, int id, int deviceId, char* horario, float longitude, float latitude, float acx, float acy, float acz) {
    
    struct tm *p;
    char query[250], datahora[50];
    time_t teste;

    time(&teste);

    p = localtime(&teste);

    //mySQL datetime format: YYYY-MM-DD hh:mm:ss
    sprintf(datahora, "%d-%d-%d %d:%d:%d", p->tm_year + 1900, p->tm_mon + 1, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);

    
    sprintf(query, "INSERT INTO data(id, device.id, dateTime, longitude, latitude, acx, acy, acz) VALUES('%d', '%d', '%s', '%f', '%f', '%f', '%f', '%f');", id, deviceId, datahora, longitude, latitude, acx, acy, acz);
    printf("%s\n", query);
    if (mysql_query(conexao, query))
    {
        printf("\nErro ao inserir no banco de dados!\n");
        erro(conexao);
    }
    else
    {
        printf("\nDados inseridos com sucesso!\n");
    }
} */

void dbRead(MYSQL *conexao, int rType, cJSON *dataAddressArray) {

    MYSQL_RES *resultado;
    MYSQL_ROW row;

    switch (rType){

        case 1:
            //sprintf(query, "SELECT id FROM user WHERE email = '%s'", emailUserRegister);
            if (mysql_query(conexao, "SELECT id FROM user WHERE email = ")) {

            }
    }
    if (mysql_query(conexao, "SELECT * FROM disp")){
        printf("Erro na leitura!\n");
        erro(conexao);
    }
    resultado = mysql_store_result(conexao);
    
    if (resultado == NULL){
        printf("Retorno nulo!\n");
        erro(conexao);
    }

    while ((row = mysql_fetch_row(resultado)) != NULL){
        printf("\nId: %s\n", row[0]);
        printf("Data hora: %s\n", row[1]);
        printf("Longitude: %s\n", row[2]);
        printf("Latitude: %s\n", row[3]);
        printf("Acx: %s, Acy: %s, Acz: %s\n", row[4], row[5], row[6]);
    }
    mysql_free_result(resultado);
}

int main(void) {
    struct mg_mgr mgr;                            // Event manager
    mg_log_set(MG_LL_DEBUG);                      // Set log level

    //MYSQL* connection = obterConexao();           // Connect to database
   
    mg_mgr_init(&mgr);                            // Initialise event manager
    
    mg_http_listen(&mgr, s_http_addr, event_handler, NULL);//(void *) &connection  // Create HTTP listener
    //mg_http_listen(&mgr, s_https_addr, event_handler, (void *) 1);  // HTTPS listener
    
    for (;;) mg_mgr_poll(&mgr, 1000);                    // Infinite event loop
    mg_mgr_free(&mgr);
    
    return 0;
}

