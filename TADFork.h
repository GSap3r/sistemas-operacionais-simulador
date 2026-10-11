#ifndef TADFORK_H
#define TADFORK_H

/* ============================================================
   TADFork.h
   Simulacao do fork()/wait() do enunciado:
   ao digitar um comando o prompt (processo pai) cria um
   processo FILHO para executar o comando e fica BLOQUEADO em
   wait(). Quando o filho termina (exit), o pai e desbloqueado
   e o prompt e liberado para um novo comando.
   Os processos sao controlados por uma PILHA.
   ============================================================ */

struct TpProc{
	int  pid;
	int  pidPai;
	char comando[120];
	char estado[15];      /* EXECUTANDO / ESPERANDO(WAIT) / PRONTO */
	TpProc *prox;
};

struct TpPilhaProc{
	int qtde;
	TpProc *topo;
};

void  inicializarPilhaProc(TpPilhaProc &p);
TpProc *forkProcesso(TpPilhaProc &p, int pid, int pidPai, char cmd[]);
void  waitProcesso(TpPilhaProc &p);
void  telaProcessos(TpPilhaProc &p, char fase[], char cmd[], int cor);


/* ==================== IMPLEMENTACAO ==================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio2.h>

void inicializarPilhaProc(TpPilhaProc &p){
	p.topo = NULL;
	p.qtde = 0;
}

/* cria o processo filho e empilha (o pai fica em wait) */
TpProc *forkProcesso(TpPilhaProc &p, int pid, int pidPai, char cmd[]){
	TpProc *caixa = new TpProc;

	caixa->pid = pid;
	caixa->pidPai = pidPai;
	strncpy(caixa->comando, cmd, 119);
	caixa->comando[119] = '\0';
	strcpy(caixa->estado, "EXECUTANDO");

	caixa->prox = p.topo;
	p.topo = caixa;
	p.qtde++;
	return caixa;
}

/* o filho terminou (exit) e o pai sai do wait */
void waitProcesso(TpPilhaProc &p){
	TpProc *aux;

	if(p.topo != NULL){
		aux = p.topo;
		p.topo = aux->prox;
		delete(aux);
		p.qtde--;
	}
}

/* tela que mostra o pai (prompt) e a pilha de processos filhos */
void telaProcessos(TpPilhaProc &p, char fase[], char cmd[], int cor){
	TpProc *aux;
	int linha;

	MolduraMenu();
	Titulo();

	Moldura(3,6,117,28,8,0);

	gotoxy(5,7);
	textcolor(14);
	printf("SIMULACAO fork() / wait()");

	gotoxy(5,9);
	textcolor(15);
	printf("Comando digitado: ");
	textcolor(11);
	printf("%.70s", cmd);

	gotoxy(5,11);
	textcolor(cor);
	printf("%.100s", fase);

	Moldura(5,13,58,26,8,0);
	gotoxy(7,14);
	textcolor(14);
	printf("PROCESSO PAI (PROMPT)");
	gotoxy(7,16);
	textcolor(7);
	printf("PID: 1");
	gotoxy(7,17);
	if(p.qtde > 0){
		textcolor(12);
		printf("Estado: ESPERANDO (wait)");
		gotoxy(7,19);
		textcolor(8);
		printf("Bloqueado ate o filho executar exit().");
	}
	else{
		textcolor(10);
		printf("Estado: EXECUTANDO");
		gotoxy(7,19);
		textcolor(8);
		printf("Prompt liberado para novo comando.");
	}

	Moldura(60,13,115,26,8,0);
	gotoxy(62,14);
	textcolor(11);
	printf("PILHA DE PROCESSOS FILHOS (topo -> base)");

	if(p.topo == NULL){
		gotoxy(62,16);
		textcolor(10);
		printf("Pilha vazia");
	}
	else{
		linha = 16;
		aux = p.topo;
		while(aux != NULL && linha < 25){
			gotoxy(62,linha);
			textcolor(15);
			printf("PID %-3d (pai %d)  %-11s", aux->pid, aux->pidPai, aux->estado);
			linha++;
			gotoxy(62,linha);
			textcolor(8);
			printf("  cmd: %.40s", aux->comando);
			linha = linha + 2;
			aux = aux->prox;
		}
	}

	textcolor(7);
}

#endif
