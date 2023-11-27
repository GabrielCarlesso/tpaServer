#include "dbFunctions.c"


int checkParse (cJSON *msg);
long int generateToken();


MYSQL *dbConnection;

static const char *s_http_addr = "http://127.0.0.1:8000";    // HTTP port


// The function is called every time an event happens

static void event_handler(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    
    if(ev == MG_EV_HTTP_MSG) { // Identifies HTTP connection
        
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;
        const char *method = hm->method.ptr;
        char tokenStr[32];
        int i, id, deviceID;
        cJSON *root = cJSON_Parse(hm->body.ptr);
        cJSON *aux, *arrayItem;
        

        printf("\n\t...message...\n%s\n", hm->message.ptr);

        if (checkParse(root) == 0) {
            mg_http_reply(c, 200, "", "{\"result\": \"Body da mensagem é nulo!\"}\n");
            cJSON_Delete(root);
        } 
        else {

            if (method[1] == 'U' || method[1] == 'A') { // PUT e PATCH indisponíveis
                mg_http_reply(c, 200, "", "{\"result\": \"Método indisponível.\"}\n");
            }

            //rota --- /api/user
            else if(mg_http_match_uri(hm, "/api/user")) {

                switch(method[0]) {

                    case 'G': // GET - Obtém informações do usuário 

                        if (cJSON_GetObjectItem(root, "token")->valuestring != NULL) {

                            // verifica qual usuário possui o token informado
                            id = dbRead(dbConnection, 2, root);
                        
                            if (id != 0) {

                                cJSON_AddNumberToObject(root, "userID", id);

                                // consulta os dados do usuário
                                if (dbRead(dbConnection, 4, root) != 0) {
                                    cJSON_DeleteItemFromObjectCaseSensitive(root, "token");
                                    cJSON_DeleteItemFromObjectCaseSensitive(root, "userID");
                                    char *jsonString = cJSON_Print(cJSON_GetObjectItem(root, "rows"));
                                    mg_http_reply(c, 200, "", jsonString);
                                    free(jsonString);
                                }
                                else {
                                    mg_http_reply(c, 200, "", "{\"result\": \"Usuário não encontrado!\"}\n");
                                }
                            }
                            else {
                                mg_http_reply(c, 200, "", "{\"result\": \"Não há usuário com o token informado!\"}\n");
                            }
                        }
                        break;
                    
                    case 'P': // POST - Registrar usuário

                        // converte o token em uma string e adiciona ao objeto cJSON root
                        snprintf(tokenStr, sizeof(tokenStr), "%ld", generateToken());
                        cJSON_AddStringToObject(root, "token", tokenStr);

                        // registra usuário se não houver existente com o email informado
                        if(dbWrite(dbConnection, 1, root) == 1){
                            char jsonString[50] = "";
                            sprintf(jsonString, "{\"token\": \"%s\"}", tokenStr);
                            printf("%s\n", jsonString);
                            mg_http_reply(c, 200, "", jsonString);
                        }
                        else{
                            mg_http_reply(c, 200, "", "{\"result\": \"Já existe usuário registrado com o email informado!\"}\n");
                        }
                        break;

                    case 'D':  // DELETE - Excluir usuário, dispositivos e dados associados
                        
                        if (cJSON_GetObjectItem(root, "token")->valuestring != NULL) {
                            
                            // verifica qual usuário possui o token informado
                            id = dbRead(dbConnection, 2, root);
                        
                            if (id != 0) {

                                cJSON_AddNumberToObject(root, "userID", id);

                                // consulta os dados do usuário
                                if (dbDelete(dbConnection, 1, root) != 0) {
                                    
                                    mg_http_reply(c, 200, "", "{\"result\": \"Usuário excluído com sucesso.\"}\n");
                                }
                                else {
                                    mg_http_reply(c, 200, "", "{\"result\": \"Falha ao excluir usuário.\"}\n");
                                }
                            }
                            else {
                                mg_http_reply(c, 200, "", "{\"result\": \"Não há usuário com o token informado.\"}\n");
                            }
                        }
                        break;

                    default:
                        mg_http_reply(c, 200, "", "{\"result\": \"Método indisponível.\"}\n");
                        break;
                }
            }
            //rota --- /api/user/device
            else if(mg_http_match_uri(hm, "/api/user/device")){

                switch(method[0]) {

                    case 'G': //GET obtém o MAC de todos os dispositivos do usuário
                        
                        if (cJSON_GetObjectItem(root, "token")->valuestring != NULL) {
                            
                            // verifica qual usuário possui o token informado
                            id = dbRead(dbConnection, 2, root);
                        
                            if (id != 0) {

                                cJSON_AddNumberToObject(root, "userID", id);

                                // consulta os dados do usuário
                                if (dbRead(dbConnection, 5, root) != 0) {
                                    cJSON_DeleteItemFromObjectCaseSensitive(root, "token");
                                    cJSON_DeleteItemFromObjectCaseSensitive(root, "userID");
                                    char *jsonString = cJSON_Print(root);
                                    printf("%s\n", jsonString);
                                    mg_http_reply(c, 200, "", jsonString);
                                    free(jsonString);
                                }
                                else {
                                    mg_http_reply(c, 200, "", "{\"result\": \"Usuário não encontrado!\"}\n");
                                }
                            }
                            else {
                                mg_http_reply(c, 200, "", "{\"result\": \"Não há usuário com o token informado!\"}\n");
                            }
                        }
                        break;
                    
                    default:
                        mg_http_reply(c, 200, "", "{\"result\": \"Método indisponível.\"}\n");
                        break;
                }
            }
            // rota --- /api/user/login
            else if (mg_http_match_uri(hm, "/api/user/login")) {

                switch(method[0]) {

                    case 'G': //GET verifica se o email e senha estão corretos
                        
                        if (cJSON_GetObjectItem(root, "token")->valuestring != NULL) {
                            
                            // verifica qual usuário possui o token informado
                            id = dbRead(dbConnection, 2, root);
                        
                            if (id != 0) {

                                // consulta os dados do usuário
                                if (dbRead(dbConnection, 6, root) == id) {
                                    mg_http_reply(c, 200, "", "{\"result\": \"Login efetuado com sucesso!\"}\n");
                                }
                                else {
                                    mg_http_reply(c, 200, "", "{\"result\": \"Dados incorretos!\"}\n");
                                }
                            }
                            else {
                                mg_http_reply(c, 200, "", "{\"result\": \"Não há usuário com o token informado!\"}\n");
                            }
                        }
                        break;
                    
                    default:
                        mg_http_reply(c, 200, "", "{\"result\": \"Método indisponível.\"}\n");
                        break;
                }

            }
            //rota --- /api/user/coordinates
            else if(mg_http_match_uri(hm, "/api/user/coordinates")){

                switch(method[0]) {
                    
                    case 'P': // post de coordenadas para cercamento virtual

                        if (cJSON_GetObjectItem(root, "token")->valuestring != NULL) {
                            
                            // verifica qual usuário possui o token informado
                            id = dbRead(dbConnection, 2, root);
                        
                            if (id != 0) {

                                if (cJSON_GetObjectItem(root, "coordinates")->valuestring != NULL){

                                    // insere os dados no banco
                                    if(dbWrite(dbConnection, 4, root) == 1){
                                        mg_http_reply(c, 200, "", "{\"result\": \"Coordenadas registradas com sucesso!\"}\n");
                                    }
                                    else {
                                        mg_http_reply(c, 200, "", "{\"result\": \"Coordenadas já armazenadas anteriormente!\"}\n");
                                    }

                                }
                            }
                            else {
                                mg_http_reply(c, 200, "", "{\"result\": \"Não há usuário com o token informado!\"}\n");
                            }
                        }
                        break;
                    
                    default:
                        mg_http_reply(c, 200, "", "{\"result\": \"Método indisponível.\"}\n");
                        break;
                }

            }
            //rota --- /api/device
            else if (mg_http_match_uri(hm, "/api/device")) {

                switch(method[0]) {
                    
                    case 'D':   // delete device

                        if (cJSON_GetObjectItem(root, "token")->valuestring != NULL) {
                            
                            // verifica qual usuário possui o token informado
                            id = dbRead(dbConnection, 2, root);
                        
                            if (id != 0) {

                                cJSON_AddNumberToObject(root, "userID", id);

                                // consulta os dados do usuário
                                if (dbDelete(dbConnection, 2, root) != 0) {
                                    
                                    mg_http_reply(c, 200, "", "{\"result\": \"Dispositivo excluído com sucesso.\"}\n");
                                }
                                else {
                                    mg_http_reply(c, 200, "", "{\"result\": \"Falha ao excluir dispositivo.\"}\n");
                                }
                            }
                            else {
                                mg_http_reply(c, 200, "", "{\"result\": \"Não há usuário com o token informado.\"}\n");
                            }
                        }
                        break;
                    
                    case 'P':   // device register
                        
                        if (cJSON_GetObjectItem(root, "token")->valuestring != NULL) {
                            
                            // verifica qual usuário possui o token informado
                            id = dbRead(dbConnection, 2, root);
                        
                            if(id != 0){
                            
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
                        break;
                    
                    default:
                        mg_http_reply(c, 200, "", "{\"result\": \"Método indisponível.\"}\n");
                        break;
                }
            }
            else if(mg_http_match_uri(hm, "/api/data")){

                switch(method[0]){

                    //
                    case 'G': // obtém os dados do dispositivo informado (no período informado ou o registro completo)

                        if (cJSON_GetObjectItem(root, "token")->valuestring != NULL) {
                            
                            // verifica qual usuário possui o token informado
                            id = dbRead(dbConnection, 2, root);
                        
                            if (id != 0) {

                                cJSON_AddNumberToObject(root, "userID", id);

                                // verifica qual dispositivo possui o MAC informado
                                deviceID = dbRead(dbConnection, 3, root);

                                printf("device ID: %i\n", deviceID);
                                if (deviceID != 0) {

                                    cJSON_AddNumberToObject(root, "deviceID", deviceID);

                                    // todos os dados do dispositivo
                                    if (dbRead(dbConnection, 7, root) != 0) {
                                        cJSON_DeleteItemFromObjectCaseSensitive(root, "token");
                                        cJSON_DeleteItemFromObjectCaseSensitive(root, "MAC");
                                        cJSON_DeleteItemFromObjectCaseSensitive(root, "deviceID");
                                        cJSON_DeleteItemFromObjectCaseSensitive(root, "userID");

                                        char *jsonString = cJSON_Print(root);
                                        mg_http_reply(c, 200, "", jsonString);
                                        free(jsonString);
                                    }  
                                }
                            }
                            else {
                                mg_http_reply(c, 200, "", "{\"result\": \"Não há usuário com o token informado!\"}\n");
                            }
                        }
                        break;
                    
                    // Deleta todos os dados referentes ao dispositivo
                    case 'D':

                        if (cJSON_GetObjectItem(root, "token")->valuestring != NULL) {
                            
                            // verifica qual usuário possui o token informado
                            id = dbRead(dbConnection, 2, root);
                        
                            if (id != 0) {

                                cJSON_AddNumberToObject(root, "userID", id);

                                // verifica qual dispositivo possui o MAC informado
                                deviceID = dbRead(dbConnection, 3, root);

                                printf("device ID: %i\n", deviceID);
                                if (deviceID != 0) {

                                    cJSON_AddNumberToObject(root, "deviceID", deviceID);

                                    // exclui todos os dados do dispositivo
                                    
                                    if(dbDelete(dbConnection, 3, root) == 1){
                                        mg_http_reply(c, 200, "", "{\"result\": \"Dados excluídos com sucesso!\"}\n");
                                    }
                                    else{
                                        mg_http_reply(c, 200, "", "{\"result\": \"Falha ao excluir os dados!\"}\n");
                                    }
                                }
                            }
                            else {
                                mg_http_reply(c, 200, "", "{\"result\": \"Não há usuário com o token informado!\"}\n");
                            }
                        }
                        break;

                    // data register
                    case 'P':

                        if (cJSON_GetObjectItem(root, "token")->valuestring != NULL) {
                            
                            // verifica qual usuário possui o token informado
                            id = dbRead(dbConnection, 2, root);
                        
                            if(id != 0){

                                cJSON_AddNumberToObject(root, "userID", id);

                                // verifica qual dispositivo possui o MAC informado
                                deviceID = dbRead(dbConnection, 3, root);

                                if (deviceID != 0) {

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
                        break;
                    
                    default:
                        mg_http_reply(c, 200, "", "{\"result\": \"Método indisponível.\"}\n");
                        break;
                }
            }
            else {
                mg_http_reply(c, 200, "", "{\"result\": \"Rota não encontrada.\"}\n");
            }
            cJSON_Delete(root);
        }
    }
    /* else {
        mg_http_reply(c, 200, "", "{\"result\": \"Evento desconhecido.\"}\n");
    } */
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
    
    for (;;) mg_mgr_poll(&mgr, 1000);                    // Infinite event loop
    mg_mgr_free(&mgr);
    
    return 0;
}