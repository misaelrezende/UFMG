#include "Bibliotecas/MinhaBiblioteca.h"

// ** referencia ** (slide Raquel)
void Insercao(long int *v){
	int i,j;
	long int aux;
	for(i = 1; i < 20; i++) {
		aux = v[i];
		j = i - 1;
		while (( j >= 0 ) && (aux < v[j])) {
			v[j+1] = v[j];
			j--;
		}
		v[j + 1] = aux;
	}
}

void escolheMediana(long int *valoresTempo, long int *mediana){
	Insercao(valoresTempo);
	// pega os dois valores do meio e divide p/ 2 para obter a mediana
	(*mediana) = (valoresTempo[9] + valoresTempo[10]) / 2; // "retorna" mediana
}

// preenche o vetor de acordo com a ordenacao escolhida
void gerarVetor(Item *Itens, int tamanho, char *tipo, Item *Auxiliar,
				int parametroOpcional){
	char *Ale = {"Ale\0"}, *OrdC = {"OrdC\0"};
	int i;
	
	if(strcmp(Ale, tipo) == 0){ // ordem aleatoria
		for(i = 0; i < tamanho; i++){
			Itens[i].Chave = (rand() % tamanho) + 1; // varia de 0 a tamanho
			if(parametroOpcional == 1)
				Auxiliar[i].Chave = Itens[i].Chave;
		}
	}else if(strcmp(OrdC, tipo) == 0){ // ordem crescente
		for(i = 0; i < tamanho; i++){
			Itens[i].Chave = i;
			if(parametroOpcional == 1)
				Auxiliar[i].Chave = Itens[i].Chave;
		}
	}else{ // ordem decresente
		int aux;
		for(i = 0, aux = tamanho - 1; i < tamanho; i++, aux--){
			Itens[i].Chave = aux;
			if(parametroOpcional == 1)
				Auxiliar[i].Chave = Itens[i].Chave;
		}
	}
}

// função imprime os resultados do teste do algoritmo
void imprimeResultados(char *variacao, char *tipo, int tamanho,
		int mediaQtdeComparacao, int mediaqtdeMovimentacao, long int medianaTempo){
		printf("%s %s %d %d %d %ld\n", variacao, tipo, tamanho, mediaQtdeComparacao,
		mediaqtdeMovimentacao, medianaTempo);
}


/*
	COMECO das funcoes relacionadas a Pilha
*/
// ** referencia ** (slide Raquel)
void FPVazia(TipoPilha *Pilha) {
	Pilha->Topo = (Apontador) malloc(sizeof(Celula));
	Pilha->Fundo = Pilha->Topo;
	Pilha->Topo->Prox = NULL;
	Pilha->Tamanho = 0;
}

int Vazia(const TipoPilha *Pilha){
	return (Pilha->Topo == Pilha->Fundo);
}

void Empilha(TipoItem x, TipoPilha *Pilha) {
	Apontador Aux;
	Aux = (Apontador) malloc(sizeof(Celula));
	Pilha->Topo->item.dir = x.dir;
	Pilha->Topo->item.esq = x.esq;
	Aux->Prox = Pilha->Topo;
	Pilha->Topo = Aux;
	Pilha->Tamanho++;
}

int Desempilha(TipoPilha *Pilha, TipoItem *item) {
	Apontador aux;
	if (Vazia(Pilha)) {
		printf("Erro: pilha vazia\n"); return 0;
	}
	aux = Pilha->Topo;
	Pilha->Topo = aux->Prox;
	free(aux);
	Pilha->Tamanho--;
	*item = Pilha->Topo->item;
	return 1;
}

int Tamanho(const TipoPilha *Pilha){
	return (Pilha->Tamanho);
}

/*
	FIM das funcoes relacionadas a Pilha
*/