#include "mongoose.h"
#include "mongoose.c"
#include "cJSON.h"
#include "cJSON.c"
#include "dbFunctions.c"


int checkParse (cJSON *msg);
long int generateToken();


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
        char tokenStr[32];
        int id, deviceID;
        cJSON *root = cJSON_Parse(hm->body.ptr);

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
                        printf("Body da msg é nulo!\n");
                        break;
                    } 
                     
                    // converte o token em uma string e adiciona ao objeto cJSON root
                    snprintf(tokenStr, sizeof(tokenStr), "%ld", generateToken());
                    cJSON_AddStringToObject(root, "token", tokenStr);

                    // registra usuário se não houver existente com o email informado
                    if(dbWrite(dbConnection, 1, root) == 1){
                        mg_http_reply(c, 200, "", "{\"result\": \"Usuário registrado com sucesso!\"}\n");
                        //sucesso, reply token
                    }
                    else{
                        mg_http_reply(c, 200, "", "{\"result\": \"Já existe usuário registrado com o email informado!\"}\n");
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
                        printf("Body da msg é nulo!\n");
                        break;
                    }
                    
                    if (cJSON_GetObjectItem(root, "token")->valuestring != NULL) {
                        
                        // verifica qual usuário possui o token informado
                        id = dbRead(dbConnection, 2, root);
                    
                        if(id != 0){

                            printf("leu id: %i\n", id);
                        
                            cJSON_AddNumberToObject(root, "userID", id);

                            // insere os dados no banco
                            if(dbWrite(dbConnection, 2, root) == 1){
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

                // data register
                case 'P':

                    if (checkParse(root) == 0) {
                        printf("Body da msg é nulo!\n");
                        break;
                    }

                    if (cJSON_GetObjectItem(root, "token")->valuestring != NULL) {
                        
                        // verifica qual usuário possui o token informado
                        id = dbRead(dbConnection, 2, root);
                    
                        if(id != 0){

                            printf("leu id: %i\n", id);

                            cJSON_AddNumberToObject(root, "userID", id);

                            // verifica qual dispositivo possui o MAC informado
                            deviceID = dbRead(dbConnection, 3, root);

                            if (deviceID != 0) {
                                printf("leu deviceId: %i\n", deviceID);

                                cJSON_AddNumberToObject(root, "deviceID", deviceID);

                                // insere os dados no banco
                                if (dbWrite(dbConnection, 3, root) == 1){
                                    mg_http_reply(c, 200, "", "{\"result\": \"Dados registrados com sucesso!\"}\n");
                                }
                                else {
                                    mg_http_reply(c, 200, "", "{\"result\": \"Erro na inserção de dados.\"}\n");
                                }
                            }
                            else {
                                mg_http_reply(c, 200, "", "{\"result\": \"MAC incorreto/inexistente.\"}\n");
                            }
                        }
                        else {
                            mg_http_reply(c, 200, "", "{\"result\": \"Token incorreto.\"}\n");
                        }
                    }

                    //libera a memória alocada para o objeto cJSON
                    cJSON_Delete(root);

                    break;
            }
        }
    }
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

int main (void) {
    
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

