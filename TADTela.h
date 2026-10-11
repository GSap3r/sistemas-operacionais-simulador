#ifndef TADTELA_H
#define TADTELA_H
#define MAX_LINHAS_TELA 900
#define LARG_LINHA      112

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <conio2.h>

/* ============================================================
   TADTela.h
   Funcoes de interface (moldura, cores e buffer de saida).
   Todo comando escreve no buffer com escrever() e depois a
   funcao mostrarSaida() desenha o resultado paginado na tela.
   ============================================================ */


void Moldura(int CI, int LI, int CF, int LF, int cortexto, int background);
void MolduraMenu();
void Titulo();
void pausarTela();

void limparSaida();
void escrever(int cor, char fmt[], ...);
void mostrarSaida(char titulo[]);
void telaEvento(char msg1[], char msg2[]);


/* ==================== IMPLEMENTACAO ==================== */



/* buffer de saida dos comandos (linha + cor de cada linha) */
char saida[MAX_LINHAS_TELA][LARG_LINHA];
int  corSaida[MAX_LINHAS_TELA];
int  totalSaida = 0;

void Moldura(int CI, int LI, int CF, int LF, int cortexto, int background){
	int i;

	textcolor(cortexto);
	textbackground(background);

	gotoxy(CI,LI); printf("%c",201);
	gotoxy(CF,LI); printf("%c",187);
	gotoxy(CI,LF); printf("%c",200);
	gotoxy(CF,LF); printf("%c",188);

	for(i=CI+1; i<CF; i++){
		gotoxy(i,LI); printf("%c",205);
		gotoxy(i,LF); printf("%c",205);
	}

	for(i=LI+1; i<LF; i++){
		gotoxy(CI,i); printf("%c",186);
		gotoxy(CF,i); printf("%c",186);
	}

	textcolor(7);
	textbackground(0);
}

void MolduraMenu(){
	clrscr();
	Moldura(1,1,119,30,3,0);
	Moldura(1,1,119,5,3,0);
}

void Titulo(){
	gotoxy(31,3);
	textcolor(8);
	printf("##### SIMULADOR - SISTEMA DE ARQUIVOS COM INODE #####");
}

void pausarTela(){
	gotoxy(45,29);
	textcolor(7);
	printf("Pressione qualquer tecla...");
	getch();
}

/* zera o buffer de saida (chamado antes de cada comando) */
void limparSaida(){
	totalSaida = 0;
}

/* grava uma linha no buffer de saida */
void escrever(int cor, char fmt[], ...){
	va_list arg;

	if(totalSaida < MAX_LINHAS_TELA){
		va_start(arg, fmt);
		vsprintf(saida[totalSaida], fmt, arg);
		va_end(arg);

		saida[totalSaida][LARG_LINHA-1] = '\0';
		corSaida[totalSaida] = cor;
		totalSaida++;
	}
}

/* desenha o buffer de saida paginado (18 linhas por pagina) */
void mostrarSaida(char titulo[]){
	int i = 0, linha, pagina = 1, total;

	if(totalSaida == 0){
		escrever(7, "Comando executado.");
	}

	total = (totalSaida + 17) / 18;

	while(i < totalSaida){
		MolduraMenu();
		Titulo();
		Moldura(3,6,117,28,8,0);

		gotoxy(5,7);
		textcolor(14);
		printf("%.100s", titulo);

		gotoxy(95,7);
		textcolor(8);
		printf("Pagina %d/%d", pagina, total);

		linha = 9;
		while(i < totalSaida && linha <= 26){
			gotoxy(6,linha);
			textcolor(corSaida[i]);
			printf("%.108s", saida[i]);
			i++;
			linha++;
		}

		gotoxy(5,29);
		textcolor(7);
		if(i < totalSaida){
			printf("[qualquer tecla] proxima pagina ...");
		}
		else{
			printf("[qualquer tecla] voltar ao prompt ...");
		}

		getch();
		pagina++;
	}

	textcolor(7);
}

/* tela simples de aviso com duas mensagens */
void telaEvento(char msg1[], char msg2[]){
	MolduraMenu();
	Titulo();

	Moldura(20,10,100,18,11,0);

	gotoxy(24,13);
	textcolor(15);
	printf("%.70s", msg1);

	gotoxy(24,15);
	textcolor(10);
	printf("%.70s", msg2);

	pausarTela();
}

#endif
