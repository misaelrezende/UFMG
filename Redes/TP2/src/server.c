#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdbool.h> // bool

#define MAX_SIZE 1024
#define DEBUG false
#define MAXCONNECTED 10 // Quantidade máxima de conexões
static const int MAXPENDING = 10; // Pedidos de conexão pendentes máximos

void informa_erro_e_termina_programa(char *mensagem){
    printf("%s", mensagem);
    exit(1);
}


typedef struct{
	int id;
	float temperatura; // Entre 0 e 10, com duas casas decimais
	int socket_id;
	struct sockaddr_in endereco_equipamento;
	int equipamentos_conectados[MAXCONNECTED];
	// Work around: posicao MAXCONNECTED indica a quantidade de equipamentos atualmente conectados no servidor
	int total_equipamentos_conectados;
} Equipamento;

Equipamento vetor_de_equipamentos[MAXCONNECTED+1];
pthread_t vetor_de_threads[MAXCONNECTED];

/* Inicializa os equipamentos com valores padrão.
Inicializa lista de equipamentos conectados com valor padrão.
Inicializa id e socket_id com valor '-1' */
void inicializar_equipamentos(Equipamento *equipamentos){
	float limite_superior = 10, numero_randomico;

	for(int i = 0; i < MAXCONNECTED; i++){
		equipamentos[i].id = -1;
		equipamentos[i].socket_id = -1;

		numero_randomico = (float) rand() / (float) (RAND_MAX / limite_superior); // BUG: Ainda retorna o mesmo valor
		equipamentos[i].temperatura = numero_randomico;

		for(int j = 0; j < MAXCONNECTED; j++)
			equipamentos[i].equipamentos_conectados[j] = -1;
	}

	equipamentos[10].total_equipamentos_conectados = 0;
}

// Cria conexão TCP
int criar_conexao_tcp(){
	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sock < 0)
        informa_erro_e_termina_programa("Falha em iniciar conexao socket do servidor.\n");

	return sock;
}

// Abre conexão TCP
void abrir_conexao_tcp(int socket_do_servidor, int numero_de_porta){
	struct sockaddr_in endereco_servidor;						// Local address
	memset(&endereco_servidor, 0, sizeof(endereco_servidor));	// Zero out structure
	endereco_servidor.sin_family = AF_INET;						// IPv4 address family
	endereco_servidor.sin_addr.s_addr = htons(INADDR_ANY);		// Any incoming interface
	endereco_servidor.sin_port = htons(numero_de_porta);		// Local port

	// Bind ao endereço local
	if(bind(socket_do_servidor, (struct sockaddr *) &endereco_servidor, sizeof(endereco_servidor)) < 0){
		// Fecha a conexão devido a erro
		close(socket_do_servidor);
		informa_erro_e_termina_programa("Falha no bind().\nEncerrando servidor.\n");
	}

	// Abrir escuta para conexão de cliente
	if(listen(socket_do_servidor, MAXPENDING) < 0){
		// Fecha a conexão devido a erro
        close(socket_do_servidor);
        informa_erro_e_termina_programa("Falha no listen().\nEncerrando servidor.\n");
	}

	if(DEBUG == true)
        printf(">> Servidor escutando na porta %i\n", numero_de_porta);
}

// Verifica se houve erro no envio de mensagem.
// Termina o programa em caso de erro.
void verificar_erro_envio_de_mensagem(ssize_t numero_bytes_enviados, int tamanho_mensagem){
	if(numero_bytes_enviados < 0)
		informa_erro_e_termina_programa("Falha no send() ao enviar mensagem para equipamento.\n");
	else if(numero_bytes_enviados != tamanho_mensagem)
		informa_erro_e_termina_programa("Falha no send().\nEnviado numero inesperado de bytes.\n");
}

// Envia mensagem para um equipamento
void enviar_mensagem_unicast(int socket, char* mensagem){
	ssize_t num_bytes_enviados = send(socket, mensagem, strlen(mensagem), 0);
	verificar_erro_envio_de_mensagem(num_bytes_enviados, strlen(mensagem));
}

// Envia mensagem a todos equipamentos conectados
void enviar_mensagem_broadcast(Equipamento* equipamentos, int id_atual, char* mensagem){
	int socket_conectado, id_equipamento_conectado;
	for(int j = 0; j < MAXCONNECTED; j++){
		id_equipamento_conectado = equipamentos[id_atual].equipamentos_conectados[j];

		if(id_equipamento_conectado != -1){
			socket_conectado = equipamentos[id_equipamento_conectado].socket_id;
			enviar_mensagem_unicast(socket_conectado, mensagem);
		}
	}
}

// Encontra uma posição livre no vetor de equipamentos
int encontrar_posicao_livre(Equipamento *equipamentos){
	int posicao;
	for(int i = 0; i < MAXCONNECTED; i++){
		if(equipamentos[i].id == -1){
			posicao = i;
			break;
		}
	}
	return posicao;
}

// Remove equipamento do vetor de equipamentos
void remover_equipamento(Equipamento *equipamento_atual){
	// Limpa lista de equipamentos conectados desse equipamento
	for(int j = 0; j < MAXCONNECTED; j++){
		if(equipamento_atual->equipamentos_conectados[j] != -1){
			equipamento_atual->equipamentos_conectados[j] = -1;
		}
	}
	equipamento_atual->id = -1;
	equipamento_atual->socket_id = -1;
}

// Recebe e consulta a temperatura do equipamento requerido
float requisitar_temperatura(Equipamento *equipamento_atual, int equipamento_solicitado){
	float temperatura = -1;

	for(int j = 0; j < MAXCONNECTED; j++){
		if(equipamento_atual->equipamentos_conectados[j] == equipamento_solicitado){
			temperatura = equipamento_atual[equipamento_solicitado].temperatura;
			break;
		}
	}
	return temperatura;
}

// Processa o comando recebido pelo equipamento atual
char* processar_comando(char *mensagem, Equipamento *equipamentos, int id_atual){
	if(DEBUG == true)
		printf("Processando comando: %s", mensagem);

	if(strcmp(mensagem, "close connection\n") == 0){
		return "close connection";
	}else if(strcmp(mensagem, "list equipment\n") == 0){
		char lista_de_equipamentos[20] = "", id[3], *ptr_msg_retorno;
		int equipamento;
		// Obtém lista de equipamentos conectados
		for(int j = 0; j < MAXCONNECTED; j++){
			equipamento = equipamentos[id_atual].equipamentos_conectados[j];
			if(equipamento != -1){
				sprintf(id, "%d ", equipamento);
				strcat(lista_de_equipamentos, id);
			}
		}

		ptr_msg_retorno = lista_de_equipamentos;
		return ptr_msg_retorno;
	}

	char *palavra; palavra = strtok(mensagem, " ");
	int equipamento_solicitado;
	if(strcmp(palavra, "request") == 0){ // assume que comando foi enviado corretamente
		while(palavra != NULL){
            palavra = strtok(NULL, " ");
			if(strcmp(palavra, "from") == 0){
				palavra = strtok(NULL, " ");
				equipamento_solicitado = atoi(palavra);
				break;
			}
		}

		float temperatura = requisitar_temperatura(&equipamentos[id_atual], equipamento_solicitado);
		char *ptr_msg_retorno = NULL, auxiliar[30] = "", id_char[10];
		if(temperatura == -1){ // caso equipamento não seja encontrado (2.b)
			printf("Equipment %d not found\n", equipamento_solicitado); // (2.b.i)
			strcat(auxiliar, "Target equipment not found"); // (2.b.i.1)
		}else{
			// Informa equipamento do requerimento de informação (2.b.ii.1)
			enviar_mensagem_unicast(equipamentos[equipamento_solicitado].socket_id, "requested information");

			strcat(auxiliar, "Value from ");
			sprintf(id_char, "%d: ", equipamento_solicitado);
			strcat(auxiliar, id_char);
			sprintf(id_char, "%.2f", temperatura);
			strcat(auxiliar, id_char); // (2.b.ii.2.b.ii)
		}

		ptr_msg_retorno = auxiliar;
		return ptr_msg_retorno;
	}

	return NULL;
}

/* Adiciona id do equipamento atual a lista dos outros equipamentos
conectados no servidor */
void adicionar_id_a_equipamentos_conectados(Equipamento* equipamentos, int id_atual){
	for(int i = 0; i < MAXCONNECTED; i++){
		if(equipamentos[i].id != -1 && equipamentos[i].id != id_atual){
			for(int j = 0; j < MAXCONNECTED; j++){
				if(equipamentos[i].equipamentos_conectados[j] == -1){
					equipamentos[i].equipamentos_conectados[j] = id_atual; // adiciona na lista dos outros conectados
					break;
				}
			}
		}
	}
}

/* Remove id do equipamento removido da lista de outros equipamentos
conectados no servidor */
void remover_id_de_equipamentos_conectados(Equipamento* equipamentos, int id_atual){
	for(int i = 0; i < MAXCONNECTED; i++){
		if(equipamentos[i].id != -1 && equipamentos[i].id != id_atual){
			for(int j = 0; j < MAXCONNECTED; j++){
				if(equipamentos[i].equipamentos_conectados[j] == id_atual){
					equipamentos[i].equipamentos_conectados[j] = -1;
					break;
				}
			}
		}
	}
}

/* Atualiza lista de equipamentos já conectados na rede
para o equipamento atual */
void atualizar_lista_equipamentos_conectados(Equipamento* equipamentos, int id_atual){
	int equipamento_atual;
	for(int i = 0; i < MAXCONNECTED; i++){
		equipamento_atual = equipamentos[i].id;

		if(equipamento_atual != -1 && equipamento_atual != id_atual){
			for(int j = 0; j < MAXCONNECTED; j++){
				if(equipamentos[id_atual].equipamentos_conectados[j] == -1){
					// adiciona equipamento já conectado na lista do equipamento atual
					equipamentos[id_atual].equipamentos_conectados[j] = equipamento_atual;
					break;
				}
			}
		}
	}
}

void* comunicar(void* equipamento){
	Equipamento* equipamentos = (Equipamento*) equipamento;
	int id_atual = equipamentos[10].id;
	int socket_do_cliente = equipamentos[id_atual].socket_id;
	char *mensagem_para_retornar = NULL;
	char mensagem[MAX_SIZE], resposta_para_cliente[MAX_SIZE];

	char mensagem_novo_id[10] = "New ID: ", id_char[3];
	sprintf(id_char, "%d", id_atual);  // converte id (int to char)
	strcat(mensagem_novo_id, id_char);

	// Envia número de id a equipamento recentemente conectado
	enviar_mensagem_unicast(socket_do_cliente, mensagem_novo_id);

	memset(id_char, 0, 3);
	memset(resposta_para_cliente, 0, MAX_SIZE);
	strcat(resposta_para_cliente, "Equipment ");
	sprintf(id_char, "%d", id_atual);
	strcat(resposta_para_cliente, id_char);
	strcat(resposta_para_cliente, " added");

	adicionar_id_a_equipamentos_conectados(equipamentos, id_atual); // (Abertura de Conexão com Servidor: 2.b)
	atualizar_lista_equipamentos_conectados(equipamentos, id_atual); // (Abertura de Conexão com Servidor: 2.b.ii e 2.b.iii)

	// Enviar broadcast de equipamento adicionado a outros equipamentos conectados (Abertura de Conexão com Servidor: 2.b.i)
	enviar_mensagem_broadcast(equipamentos, id_atual, resposta_para_cliente);

	while(true){
		ssize_t num_bytes_recebidos = recv(socket_do_cliente, mensagem, MAX_SIZE, 0);

		if(num_bytes_recebidos < 0)
            informa_erro_e_termina_programa("Falha no recv() ao receber mensagem do equipamento.\n");
        else if(num_bytes_recebidos == 0){
            if(DEBUG == true)
                printf("Conexao encerrada pelo equipamento %d.\n", id_atual);

			// Se equipamento encerrar abrutamente, remova equipamento
			printf("Equipment %d removed\n", id_atual);

			memset(id_char, 0, 3);
			memset(resposta_para_cliente, 0, MAX_SIZE);
			strcat(resposta_para_cliente, "Equipment ");
			sprintf(id_char, "%d", id_atual);
			strcat(resposta_para_cliente, id_char);
			strcat(resposta_para_cliente, " removed");

			// Enviar broadcast de equipamento removido arbitrariamente a outros equipamentos conectados (Encerramento de Conexão com Servidor: 2.b.ii)
			enviar_mensagem_broadcast(equipamentos, id_atual, resposta_para_cliente);
			equipamentos[10].total_equipamentos_conectados -= 1; // Diminui da lista de equipamentos conectados
            break;
        }

		mensagem[num_bytes_recebidos] = '\0';
		if(DEBUG == true)
			printf("recebido no servidor: %s", mensagem); // mensagem[strlen(mensagem) - 1] == '\n'

		mensagem_para_retornar = processar_comando(mensagem, equipamentos, id_atual);
		if(mensagem_para_retornar == NULL){
			printf("Mensagem desconhecida enviada por equipamento.\nA conexao com cliente sera encerrada.\n");
			break;
		}

		if(strcmp(mensagem_para_retornar, "close connection") == 0){
			printf("Equipment %d removed\n", id_atual);
			memset(resposta_para_cliente, 0, MAX_SIZE);
			strcpy(resposta_para_cliente, "Sucess");

			// Envia unicast de equipamento removido (Encerramento de Conexão com Servidor: 2.b)
			enviar_mensagem_unicast(socket_do_cliente, resposta_para_cliente);

			memset(id_char, 0, 3);
			memset(resposta_para_cliente, 0, MAX_SIZE);
			strcat(resposta_para_cliente, "Equipment ");
			sprintf(id_char, "%d", id_atual);
			strcat(resposta_para_cliente, id_char);
			strcat(resposta_para_cliente, " removed");

			// Enviar broadcast de equipamento removido a outros equipamentos conectados (Encerramento de Conexão com Servidor: 2.b.ii)
			enviar_mensagem_broadcast(equipamentos, id_atual, resposta_para_cliente);
			equipamentos[10].total_equipamentos_conectados -= 1; // Diminui da lista de equipamentos conectados
			break;
		}

		strcpy(resposta_para_cliente, mensagem_para_retornar);
		enviar_mensagem_unicast(socket_do_cliente, resposta_para_cliente);
	}

	remover_id_de_equipamentos_conectados(equipamentos, id_atual);
	remover_equipamento(&equipamentos[id_atual]);
	close(socket_do_cliente);

	return NULL;
}


int main(int argc, char* argv[]){
	// Args:
    // Número de porta
    if(argc != 2){
        fprintf(stderr,
        "O servidor precisa de exatamente de 1 parametro de entrada:\n\
        ./server <numero da porta>\n");
        exit(1);
    }
	int numero_de_porta = atoi(argv[1]);

	int socket_do_servidor = criar_conexao_tcp();
	abrir_conexao_tcp(socket_do_servidor, numero_de_porta);

	int socket_do_cliente;
	struct sockaddr_in endereco_cliente;
	socklen_t tamanho_endereco_cliente;
	tamanho_endereco_cliente = sizeof(endereco_cliente);

	// Inicializa os equipamentos com valores padrão
	inicializar_equipamentos(vetor_de_equipamentos);
	int posicao_valida = 0; // próxima posicao válida do vetor

	while(true){
		if(vetor_de_equipamentos[10].total_equipamentos_conectados == 10)
			continue;
		else if(vetor_de_equipamentos[10].total_equipamentos_conectados <= 9){
			// Espera conexão de equipamento
			socket_do_cliente = accept(socket_do_servidor,
				(struct sockaddr*) &vetor_de_equipamentos[posicao_valida].endereco_equipamento,
				&tamanho_endereco_cliente);
			if(socket_do_cliente < 0)
				informa_erro_e_termina_programa("Falha no accept().\nEquipamento nao conectado\n");

			vetor_de_equipamentos[posicao_valida].socket_id = socket_do_cliente;
			vetor_de_equipamentos[posicao_valida].id = posicao_valida;

			printf("Equipment %d added\n", posicao_valida);

			// Work around: posição do id desse equipamento a ser tratado na funcao comunicar()
			vetor_de_equipamentos[10].id = posicao_valida;

			int valor_de_retorno = pthread_create(&vetor_de_threads[posicao_valida], NULL, comunicar,
				(void *) &vetor_de_equipamentos);
			if(valor_de_retorno != 0){
				printf("Thread %lu: ", (unsigned long int) vetor_de_threads[posicao_valida]);
				informa_erro_e_termina_programa("Falha no pthread_create().\n");
			}

			posicao_valida = encontrar_posicao_livre(vetor_de_equipamentos);
			vetor_de_equipamentos[10].total_equipamentos_conectados += 1;
		}
 
	}

	for(int i = 0; i < vetor_de_equipamentos[10].total_equipamentos_conectados; i++)
		pthread_join(vetor_de_threads[i], NULL);

	return 0;
}