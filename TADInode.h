#ifndef TADINODE_H
#define TADINODE_H
#define MAX_BLOCOS    5000   /* limite fisico do vetor de disco */
#define TAM_BLOCO     10     /* bytes por bloco                 */
#define MAX_NOME      31
#define MAX_ENTRADAS  10     /* entradas de diretorio por bloco */
#define MAX_CAMINHO   200
#define MAX_LISTA     300    /* buffers auxiliares de blocos    */

#define LIVRE        'F'
#define BAD          'B'
#define TIPO_INODE   'I'
#define ARQUIVO      'A'

#define DIRETOS       5      /* ponteiros diretos do inode principal */
#define PONT_INODE    5      /* ponteiros do inode de extensao       */
#define MAX_BLOCOS_ARQ 160   /* 5 + 5 + 25 + 125                     */

/* ============================================================
   TADInode.h
   Estruturas do sistema de arquivos com inode.
   Usa apenas struct, lista encadeada e pilha (sem union).

   - o disco e um vetor de blocos, cada posicao guarda o TIPO
	 do bloco: F(livre) B(defeituoso) I(inode) A(arquivo)
   - os inodes ficam em uma LISTA ENCADEADA, indexados pelo
	 numero do bloco onde estao gravados
   - os blocos livres ficam em uma PILHA e sao devolvidos de
	 forma ALEATORIA (item b do enunciado)
   - a arvore de diretorios e uma lista encadeada de filhos
   ============================================================ */

/* ---------- inode (principal ou de extensao) ---------- */
struct TpInode{
	int  bloco;          /* bloco do disco onde o inode esta gravado */
	int  principal;      /* 1 = inode principal / 0 = extensao       */
	int  nivel;          /* 0=principal 1=simples 2=duplo 3=triplo   */
	char permissoes[12]; /* ex: -rwxr-xr-- / drwxr-xr-x / lrwxrwxrwx */
	char data[12];
	char hora[10];
	int  tamanho;        /* em bytes */
	char usuario[15];
	char grupo[15];
	int  links;          /* contador de links fisicos */
	int  ponteiros[8];   /* 0..4 diretos, 5 simples, 6 duplo, 7 triplo */
	TpInode *prox;
};

/* ---------- descritor da lista encadeada de inodes ---------- */
struct TpDescInode{
	int qtde;
	TpInode *inicio, *fim;
};

/* ---------- entrada da arvore de diretorios ---------- */
struct TpEntrada;

/* descritor da lista encadeada de entradas de um diretorio */
struct TpDescEntrada{
	int qtde;
	TpEntrada *inicio, *fim;
};

struct TpEntrada{
	char nome[MAX_NOME];
	int  inode;                 /* bloco do inode principal */
	int  ehDiretorio;
	int  ehLinkSimbolico;
	int  ehLinkFisico;
	char destino[MAX_CAMINHO];  /* caminho guardado no link simbolico */
	TpEntrada *pai;
	TpDescEntrada filhos;       /* lista encadeada com as entradas do diretorio */
	TpEntrada *prox;
};

/* ---------- pilha de blocos livres ---------- */
struct TpCaixaPilha{
	int bloco;
	TpCaixaPilha *prox;
};

struct TpPilha{
	int qtde;
	TpCaixaPilha *topo;
};

/* ---------- disco e pilha de livres ---------- */
void inicializarDisco(int qtd);
void inicializarPilha(TpPilha &p);
bool pilhaVazia(TpPilha p);
void empilhar(TpPilha &p, int bloco);
int  desempilhar(TpPilha &p);
int  desempilharAleatorio(TpPilha &p);
int  reservarBloco(int b);
int  alocarBloco();
void liberarBloco(int b);

/* ---------- inodes ---------- */
void     inicializarListaInode(TpDescInode &d);
TpInode *criarInode(int bloco, int principal, int nivel, char perm[], int tamanho);
TpInode *buscarInode(int bloco);
void     removerInode(int bloco);
int      inodesExtras(int qtdDados);
int      blocosNecessarios(int bytes);
int      alocarIndireto(int nivel, int &faltam);
int      alocarArquivo(TpInode *ip, int qtdDados);
void     percorrerExt(int bloco, int nivel, int dados[], int &qd, int inodes[], int &qi);
void     blocosDoArquivo(TpInode *ip, int dados[], int &qd, int inodes[], int &qi);
void     liberarConteudo(TpInode *ip);

/* ---------- arvore de diretorios ---------- */
TpEntrada *criarNo(char nome[], int inode, int ehDir);
void       inserirFilho(TpEntrada *pai, TpEntrada *filho);
TpEntrada *buscarFilho(TpEntrada *pai, char nome[]);
int        removerFilho(TpEntrada *pai, char nome[]);
TpEntrada *resolverNo(TpEntrada *base, char caminho[]);
TpEntrada *resolverPai(TpEntrada *base, char caminho[], char nomeFinal[]);
void       caminhoDe(TpEntrada *n, char out[]);
int        atualizarDiretorio(TpEntrada *dir);
int        temPermissao(TpInode *ip, char c);
int        arquivoCorrompido(TpInode *ip);
int        contarFilhos(TpEntrada *dir);

/* ---------- comandos do prompt ---------- */
void cmdLs(int detalhado);
void cmdCd(char caminho[]);
void cmdTouch(char caminho[], int bytes);
void cmdMkdir(char caminho[]);
void cmdRm(char caminho[]);
void cmdRmdir(char caminho[]);
void cmdVi(char caminho[]);
void cmdChmod(char quem[], char bits[], char caminho[]);
void cmdBad(int bloco);
void cmdDf();
void cmdLinkFisico(char origem[], char destino[]);
void cmdLinkSimbolico(char origem[], char destino[]);
void cmdUnlink(int simbolico, char nome[]);

/* ---------- relatorios ---------- */
void relBlocosArquivo(char nome[]);  /* 1 */
void relMaiorArquivo();              /* 2 */
void relIntegridade();               /* 3 */
void relPerdidos();                  /* 4 */
void telaBlocos();                   /* 5 */
void relExplorer();                  /* 6 */
void relArvoreInodes();              /* 7 */
void relLinks();                     /* 8 */

void carregarExemplo();


/* ==================== IMPLEMENTACAO ==================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <conio2.h>

/* ============================================================
   VARIAVEIS GLOBAIS
   ============================================================ */
char        disco[MAX_BLOCOS];
int         totalBlocos = 0;
TpDescInode listaInodes;
TpEntrada   *raiz = NULL;
TpEntrada   *atual = NULL;
TpPilha     livres;
char        usuarioAtual[15] = "aluno";
char        grupoAtual[15] = "so";

/* ============================================================
   PILHA DE BLOCOS LIVRES  (item b do enunciado)
   ============================================================ */
void inicializarPilha(TpPilha &p){
	p.topo = NULL;
	p.qtde = 0;
}

bool pilhaVazia(TpPilha p){
	return p.topo == NULL;
}

void empilhar(TpPilha &p, int bloco){
	TpCaixaPilha *caixa = new TpCaixaPilha;

	caixa->bloco = bloco;
	caixa->prox = p.topo;
	p.topo = caixa;
	p.qtde++;
}

int desempilhar(TpPilha &p){
	TpCaixaPilha *aux;
	int b = -1;

	if(!pilhaVazia(p)){
		aux = p.topo;
		b = aux->bloco;
		p.topo = aux->prox;
		delete(aux);
		p.qtde--;
	}
	return b;
}

/* retorna um bloco livre ALEATORIO de dentro da pilha */
int desempilharAleatorio(TpPilha &p){
	TpCaixaPilha *ant = NULL, *aux;
	int k, i, b = -1;

	if(!pilhaVazia(p)){
		k = rand() % p.qtde;
		aux = p.topo;

		for(i=0; i<k; i++){
			ant = aux;
			aux = aux->prox;
		}

		if(ant == NULL){
			p.topo = aux->prox;
		}
		else{
			ant->prox = aux->prox;
		}

		b = aux->bloco;
		delete(aux);
		p.qtde--;
	}
	return b;
}

/* retira da pilha um bloco especifico (usado na carga do exemplo) */
int reservarBloco(int b){
	TpCaixaPilha *ant = NULL, *aux = livres.topo;
	int achou = 0;

	while(aux != NULL && aux->bloco != b){
		ant = aux;
		aux = aux->prox;
	}

	if(aux != NULL){
		if(ant == NULL){
			livres.topo = aux->prox;
		}
		else{
			ant->prox = aux->prox;
		}
		delete(aux);
		livres.qtde--;
		achou = 1;
	}
	return achou;
}

int alocarBloco(){
	return desempilharAleatorio(livres);
}

void liberarBloco(int b){
	/* bloco defeituoso nao volta para os livres */
	if(b >= 0 && b < totalBlocos && disco[b] != BAD){
		disco[b] = LIVRE;
		empilhar(livres, b);
	}
}

/* ============================================================
   INODES (lista encadeada com descritor)
   ============================================================ */
void inicializarListaInode(TpDescInode &d){
	d.qtde = 0;
	d.inicio = d.fim = NULL;
}

TpInode *criarInode(int bloco, int principal, int nivel, char perm[], int tamanho){
	TpInode *ip = NULL;
	time_t t;
	struct tm *dt;
	int i;

	if(bloco >= 0){
		ip = new TpInode;
		ip->bloco = bloco;
		ip->principal = principal;
		ip->nivel = nivel;
		strcpy(ip->permissoes, perm);

		t = time(NULL);
		dt = localtime(&t);
		sprintf(ip->data, "%02d/%02d/%04d", dt->tm_mday, dt->tm_mon+1, dt->tm_year+1900);
		sprintf(ip->hora, "%02d:%02d", dt->tm_hour, dt->tm_min);

		ip->tamanho = tamanho;
		strcpy(ip->usuario, usuarioAtual);
		strcpy(ip->grupo, grupoAtual);
		ip->links = 1;

		for(i=0; i<8; i++){
			ip->ponteiros[i] = -1;
		}

		/* insere no fim da lista de inodes */
		ip->prox = NULL;
		if(listaInodes.inicio == NULL){
			listaInodes.inicio = listaInodes.fim = ip;
		}
		else{
			listaInodes.fim->prox = ip;
			listaInodes.fim = ip;
		}
		listaInodes.qtde++;

		disco[bloco] = TIPO_INODE;
	}
	return ip;
}

TpInode *buscarInode(int bloco){
	TpInode *aux = listaInodes.inicio;

	while(aux != NULL && aux->bloco != bloco){
		aux = aux->prox;
	}
	return aux;
}

void removerInode(int bloco){
	TpInode *aux = listaInodes.inicio, *ant = NULL;

	while(aux != NULL && aux->bloco != bloco){
		ant = aux;
		aux = aux->prox;
	}

	if(aux != NULL){
		if(ant == NULL){
			listaInodes.inicio = aux->prox;
		}
		else{
			ant->prox = aux->prox;
		}
		if(aux == listaInodes.fim){
			listaInodes.fim = ant;
		}
		delete(aux);
		listaInodes.qtde--;
	}
}

/* quantos inodes de extensao sao necessarios para n blocos de dados */
int inodesExtras(int qtdDados){
	int r = qtdDados - DIRETOS, total = 0, x;

	if(r > 0){
		/* indireto simples: ate 5 blocos */
		if(r > 5){
			x = 5;
		}
		else{
			x = r;
		}
		total = total + 1;
		r = r - x;

		if(r > 0){
			/* indireto duplo: ate 25 blocos */
			if(r > 25){
				x = 25;
			}
			else{
				x = r;
			}
			total = total + 1 + ((x + 4) / 5);
			r = r - x;

			if(r > 0){
				/* indireto triplo: ate 125 blocos */
				if(r > 125){
					x = 125;
				}
				else{
					x = r;
				}
				total = total + 1 + ((x + 24) / 25) + ((x + 4) / 5);
			}
		}
	}
	return total;
}

/* total de blocos gastos por um arquivo de "bytes" bytes */
int blocosNecessarios(int bytes){
	int dados = (bytes + TAM_BLOCO - 1) / TAM_BLOCO;

	if(dados < 1){
		dados = 1;
	}
	return 1 + dados + inodesExtras(dados);
}

/* cria recursivamente um inode de extensao de um determinado nivel */
int alocarIndireto(int nivel, int &faltam){
	TpInode *ip;
	int b, i = 0, dado, erro = 0;

	b = alocarBloco();
	if(b >= 0){
		ip = criarInode(b, 0, nivel, "----------", 0);

		while(i < PONT_INODE && faltam > 0 && !erro){
			if(nivel == 1){
				dado = alocarBloco();
				if(dado < 0){
					erro = 1;
				}
				else{
					disco[dado] = ARQUIVO;
					ip->ponteiros[i] = dado;
					faltam--;
				}
			}
			else{
				ip->ponteiros[i] = alocarIndireto(nivel-1, faltam);
			}
			i++;
		}
	}
	return b;
}

/* distribui qtdDados blocos entre diretos / simples / duplo / triplo */
int alocarArquivo(TpInode *ip, int qtdDados){
	int faltam = qtdDados, i = 0, b, erro = 0;

	while(i < DIRETOS && faltam > 0 && !erro){
		b = alocarBloco();
		if(b < 0){
			erro = 1;
		}
		else{
			disco[b] = ARQUIVO;
			ip->ponteiros[i] = b;
			faltam--;
		}
		i++;
	}

	if(!erro && faltam > 0){
		ip->ponteiros[5] = alocarIndireto(1, faltam);
	}
	if(!erro && faltam > 0){
		ip->ponteiros[6] = alocarIndireto(2, faltam);
	}
	if(!erro && faltam > 0){
		ip->ponteiros[7] = alocarIndireto(3, faltam);
	}

	return !erro && faltam == 0;
}

/* percorre um inode de extensao coletando blocos */
void percorrerExt(int bloco, int nivel, int dados[], int &qd, int inodes[], int &qi){
	TpInode *ip = NULL;
	int i;

	if(bloco >= 0){
		ip = buscarInode(bloco);
	}

	if(ip != NULL){
		if(qi < MAX_LISTA){
			inodes[qi++] = bloco;
		}

		for(i=0; i<PONT_INODE; i++){
			if(ip->ponteiros[i] >= 0){
				if(nivel == 1){
					if(qd < MAX_LISTA){
						dados[qd++] = ip->ponteiros[i];
					}
				}
				else{
					percorrerExt(ip->ponteiros[i], nivel-1, dados, qd, inodes, qi);
				}
			}
		}
	}
}

/* devolve todos os blocos de dados e todos os blocos de inode do arquivo */
void blocosDoArquivo(TpInode *ip, int dados[], int &qd, int inodes[], int &qi){
	int i;

	qd = 0;
	qi = 0;

	if(ip != NULL){
		inodes[qi++] = ip->bloco;

		for(i=0; i<DIRETOS; i++){
			if(ip->ponteiros[i] >= 0){
				dados[qd++] = ip->ponteiros[i];
			}
		}

		percorrerExt(ip->ponteiros[5], 1, dados, qd, inodes, qi);
		percorrerExt(ip->ponteiros[6], 2, dados, qd, inodes, qi);
		percorrerExt(ip->ponteiros[7], 3, dados, qd, inodes, qi);
	}
}

/* libera dados + inodes de extensao (mantem o inode principal) */
void liberarConteudo(TpInode *ip){
	int dados[MAX_LISTA], inodes[MAX_LISTA], qd, qi, i;

	if(ip != NULL){
		blocosDoArquivo(ip, dados, qd, inodes, qi);

		for(i=0; i<qd; i++){
			liberarBloco(dados[i]);
		}

		for(i=1; i<qi; i++){
			removerInode(inodes[i]);
			liberarBloco(inodes[i]);
		}

		for(i=0; i<8; i++){
			ip->ponteiros[i] = -1;
		}
	}
}

/* ============================================================
   ARVORE DE DIRETORIOS
   ============================================================ */
TpEntrada *criarNo(char nome[], int inode, int ehDir){
	TpEntrada *caixa = new TpEntrada;

	strncpy(caixa->nome, nome, MAX_NOME-1);
	caixa->nome[MAX_NOME-1] = '\0';
	caixa->inode = inode;
	caixa->ehDiretorio = ehDir;
	caixa->ehLinkSimbolico = 0;
	caixa->ehLinkFisico = 0;
	caixa->destino[0] = '\0';
	caixa->pai = NULL;
	caixa->filhos.qtde = 0;
	caixa->filhos.inicio = caixa->filhos.fim = NULL;
	caixa->prox = NULL;
	return caixa;
}

/* insere a entrada no fim da lista do diretorio pai */
void inserirFilho(TpEntrada *pai, TpEntrada *filho){
	filho->pai = pai;
	filho->prox = NULL;

	if(pai->filhos.inicio == NULL){
		pai->filhos.inicio = pai->filhos.fim = filho;
	}
	else{
		pai->filhos.fim->prox = filho;
		pai->filhos.fim = filho;
	}
	pai->filhos.qtde++;
}

TpEntrada *buscarFilho(TpEntrada *pai, char nome[]){
	TpEntrada *aux = NULL;

	if(pai != NULL){
		aux = pai->filhos.inicio;
		while(aux != NULL && strcmp(aux->nome, nome) != 0){
			aux = aux->prox;
		}
	}
	return aux;
}

int removerFilho(TpEntrada *pai, char nome[]){
	TpEntrada *aux = pai->filhos.inicio, *ant = NULL;
	int removeu = 0;

	while(aux != NULL && strcmp(aux->nome, nome) != 0){
		ant = aux;
		aux = aux->prox;
	}

	if(aux != NULL){
		if(ant == NULL){
			pai->filhos.inicio = aux->prox;
		}
		else{
			ant->prox = aux->prox;
		}
		if(aux == pai->filhos.fim){
			pai->filhos.fim = ant;
		}
		delete(aux);
		pai->filhos.qtde--;
		removeu = 1;
	}
	return removeu;
}

int contarFilhos(TpEntrada *dir){
	int n = 0;

	if(dir != NULL){
		n = dir->filhos.qtde;
	}
	return n;
}

/* caminho absoluto (da raiz ate o no) */
void caminhoDe(TpEntrada *n, char out[]){
	char partes[40][MAX_NOME];
	int k = 0, i;
	TpEntrada *cur = n;

	strcpy(out, "/");

	if(n != NULL && n != raiz){
		while(cur != NULL && cur != raiz && k < 40){
			strcpy(partes[k++], cur->nome);
			cur = cur->pai;
		}

		for(i=k-1; i>=0; i--){
			strcat(out, partes[i]);
			if(i > 0){
				strcat(out, "/");
			}
		}
	}
}

/* resolve um caminho absoluto (/a/b) ou relativo (a/b, ., ..) */
TpEntrada *resolverNo(TpEntrada *base, char caminho[]){
	char tmp[MAX_CAMINHO], parte[MAX_NOME];
	int i = 0, j, erro = 0;
	TpEntrada *cur, *f;

	if(caminho == NULL || caminho[0] == '\0'){
		cur = base;
	}
	else{
		strncpy(tmp, caminho, MAX_CAMINHO-1);
		tmp[MAX_CAMINHO-1] = '\0';

		if(tmp[0] == '/'){
			cur = raiz;
			i = 1;
		}
		else{
			cur = base;
		}

		while(tmp[i] != '\0' && !erro){
			j = 0;
			while(tmp[i] != '/' && tmp[i] != '\0' && j < MAX_NOME-1){
				parte[j++] = tmp[i++];
			}
			parte[j] = '\0';
			while(tmp[i] == '/'){
				i++;
			}

			if(strcmp(parte, "..") == 0){
				if(cur->pai != NULL){
					cur = cur->pai;
				}
			}
			else{
				/* parte vazia ou "." mantem o diretorio atual */
				if(parte[0] != '\0' && strcmp(parte, ".") != 0){
					f = buscarFilho(cur, parte);
					if(f != NULL && f->ehLinkSimbolico){
						/* segue o link simbolico */
						f = resolverNo(raiz, f->destino);
					}

					if(f == NULL){
						erro = 1;
						cur = NULL;
					}
					else{
						cur = f;
					}
				}
			}
		}
	}
	return cur;
}

/* separa o caminho em diretorio pai + nome final */
TpEntrada *resolverPai(TpEntrada *base, char caminho[], char nomeFinal[]){
	char tmp[MAX_CAMINHO];
	int len, i;
	TpEntrada *pai;

	strncpy(tmp, caminho, MAX_CAMINHO-1);
	tmp[MAX_CAMINHO-1] = '\0';

	len = strlen(tmp);
	while(len > 1 && tmp[len-1] == '/'){
		tmp[len-1] = '\0';
		len--;
	}

	i = len - 1;
	while(i >= 0 && tmp[i] != '/'){
		i--;
	}

	if(i < 0){
		strncpy(nomeFinal, tmp, MAX_NOME-1);
		nomeFinal[MAX_NOME-1] = '\0';
		pai = base;
	}
	else{
		strncpy(nomeFinal, tmp+i+1, MAX_NOME-1);
		nomeFinal[MAX_NOME-1] = '\0';

		if(i == 0){
			pai = raiz;
		}
		else{
			tmp[i] = '\0';
			pai = resolverNo(base, tmp);
		}
	}
	return pai;
}

/* recalcula quantos blocos o arquivo de diretorio precisa (10 entradas/bloco) */
int atualizarDiretorio(TpEntrada *dir){
	TpInode *ip = NULL;
	int dados[MAX_LISTA], inodes[MAX_LISTA], qd, qi;
	int n, precisa, disponivel, necessario, ok = 0;

	if(dir != NULL){
		ip = buscarInode(dir->inode);
	}

	if(ip != NULL){
		n = contarFilhos(dir) + 2;   /* entradas . e .. */
		precisa = (n + MAX_ENTRADAS - 1) / MAX_ENTRADAS;
		if(precisa < 1){
			precisa = 1;
		}

		blocosDoArquivo(ip, dados, qd, inodes, qi);

		if(qd == precisa){
			ip->tamanho = precisa * TAM_BLOCO;
			ok = 1;
		}
		else{
			disponivel = livres.qtde + qd + (qi - 1);
			necessario = precisa + inodesExtras(precisa);
			if(necessario <= disponivel){
				liberarConteudo(ip);
				alocarArquivo(ip, precisa);
				ip->tamanho = precisa * TAM_BLOCO;
				ok = 1;
			}
		}
	}
	return ok;
}

int temPermissao(TpInode *ip, char c){
	int tem = 0;

	if(ip != NULL){
		switch(c){
			case 'r':
				tem = ip->permissoes[1] == 'r';
				break;
			case 'w':
				tem = ip->permissoes[2] == 'w';
				break;
			case 'x':
				tem = ip->permissoes[3] == 'x';
				break;
		}
	}
	return tem;
}

/* arquivo corrompido = possui pelo menos um bloco defeituoso */
int arquivoCorrompido(TpInode *ip){
	int dados[MAX_LISTA], inodes[MAX_LISTA], qd, qi, i, corrompido = 0;

	if(ip != NULL){
		blocosDoArquivo(ip, dados, qd, inodes, qi);

		i = 0;
		while(i < qd && !corrompido){
			if(disco[dados[i]] == BAD){
				corrompido = 1;
			}
			i++;
		}

		i = 0;
		while(i < qi && !corrompido){
			if(disco[inodes[i]] == BAD){
				corrompido = 1;
			}
			i++;
		}
	}
	return corrompido;
}

/* ============================================================
   INICIALIZACAO DO DISCO
   ============================================================ */
void inicializarDisco(int qtd){
	int i, b;

	if(qtd < 20){
		qtd = 20;
	}
	if(qtd > MAX_BLOCOS){
		qtd = MAX_BLOCOS;
	}

	totalBlocos = qtd;
	inicializarListaInode(listaInodes);

	for(i=0; i<totalBlocos; i++){
		disco[i] = LIVRE;
	}

	inicializarPilha(livres);
	for(i=totalBlocos-1; i>=0; i--){
		empilhar(livres, i);
	}

	/* cria o diretorio raiz */
	b = alocarBloco();
	raiz = criarNo("/", b, 1);
	raiz->pai = NULL;
	criarInode(b, 1, 0, "drwxr-xr-x", TAM_BLOCO);
	atualizarDiretorio(raiz);

	atual = raiz;
}

#endif
