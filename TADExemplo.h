#ifndef TADEXEMPLO_H
#define TADEXEMPLO_H

#include "TADTela.h"
#include "TADInode.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================
   Carga do disco de exemplo (figuras A, B e D do enunciado).
   Serve para ja iniciar a simulacao com a arvore da raiz /
   preenchida com os mesmos arquivos e blocos do trabalho.
   ============================================================ */

/* tenta pegar um bloco especifico; se nao estiver livre, pega qualquer um */
int pegarBloco(int preferido){
	int b;

	if(preferido >= 0 && preferido < totalBlocos && disco[preferido] == LIVRE && reservarBloco(preferido)){
		b = preferido;
	}
	else{
		b = alocarBloco();
	}
	return b;
}

/* cria um arquivo/diretorio ja com os blocos indicados no enunciado */
TpEntrada *exemploEntrada(TpEntrada *pai, char nome[], int ehDir, int blocoInode, int b[], int qtd, int indireto){
	TpInode *ip, *ipInd = NULL;
	TpEntrada *novo = NULL;
	int i = 0, bi, bd, erro = 0;

	blocoInode = pegarBloco(blocoInode);
	if(blocoInode >= 0){
		if(ehDir){
			ip = criarInode(blocoInode, 1, 0, "drwxr-xr-x", qtd * TAM_BLOCO);
		}
		else{
			ip = criarInode(blocoInode, 1, 0, "-rw-r--r--", qtd * TAM_BLOCO);
		}

		while(i < qtd && !erro){
			bd = pegarBloco(b[i]);
			if(bd < 0){
				erro = 1;
			}
			else{
				disco[bd] = ARQUIVO;

				if(i < DIRETOS){
					ip->ponteiros[i] = bd;
				}
				else{
					if(ipInd == NULL){
						bi = pegarBloco(indireto);
						if(bi < 0){
							erro = 1;
						}
						else{
							ipInd = criarInode(bi, 0, 1, "----------", 0);
							ip->ponteiros[5] = bi;
						}
					}
					if(!erro && i - DIRETOS < PONT_INODE){
						ipInd->ponteiros[i-DIRETOS] = bd;
					}
				}
			}
			i++;
		}

		novo = criarNo(nome, blocoInode, ehDir);
		inserirFilho(pai, novo);
	}
	return novo;
}

/* marca um bloco como defeituoso (mesmo que ja pertenca a um arquivo) */
void marcarBad(int b){
	if(b >= 0 && b < totalBlocos){
		if(disco[b] == LIVRE){
			reservarBloco(b);
		}
		disco[b] = BAD;
	}
}

/* bloco ocupado que nao pertence a nenhum arquivo (bloco perdido) */
void marcarPerdido(int b){
	int x = pegarBloco(b);

	if(x >= 0){
		disco[x] = ARQUIVO;
	}
}

void carregarExemplo(){
	int b[12];
	TpEntrada *boot, *bin;

	/* o exemplo da figura usa blocos de 0 a 79 */
	if(totalBlocos >= 80){
		/* ---- arquivos da raiz (figura B / figura D) ---- */

		/* Readme.txt: inode 16, 6 blocos -> o 6o usa o inode indireto simples 45 */
		b[0]=7; b[1]=12; b[2]=23; b[3]=26; b[4]=1; b[5]=58;
		exemploEntrada(raiz, "Readme.txt", 0, 16, b, 6, 45);

		b[0]=63; b[1]=51; b[2]=46;
		exemploEntrada(raiz, "Icone.gif", 0, 14, b, 3, -1);

		b[0]=19;
		exemploEntrada(raiz, "programa", 0, 3, b, 1, -1);

		b[0]=36; b[1]=20; b[2]=33;
		exemploEntrada(raiz, "Retrato.jpg", 0, 29, b, 3, -1);

		/* Relat.pdf: 8 blocos -> 5 diretos + 3 no indireto simples */
		b[0]=17; b[1]=28; b[2]=11; b[3]=53; b[4]=59; b[5]=72; b[6]=10; b[7]=13;
		exemploEntrada(raiz, "Relat.pdf", 0, 6, b, 8, -1);

		b[0]=37;
		exemploEntrada(raiz, "Format.exe", 0, 31, b, 1, -1);

		b[0]=60; b[1]=44;
		exemploEntrada(raiz, "Carta.doc", 0, 67, b, 2, -1);

		b[0]=27; b[1]=55;
		exemploEntrada(raiz, "Programa.c", 0, 73, b, 2, -1);

		/* ---- diretorios ---- */
		b[0]=70;
		boot = exemploEntrada(raiz, "Boot", 1, 4, b, 1, -1);
		if(boot != NULL){
			b[0]=-1; b[1]=-1;
			exemploEntrada(boot, "kernel", 0, 35, b, 2, -1);
			b[0]=-1;
			exemploEntrada(boot, "config.sys", 0, -1, b, 1, -1);
			atualizarDiretorio(boot);
		}

		b[0]=39;
		bin = exemploEntrada(raiz, "bin", 1, 62, b, 1, -1);
		if(bin != NULL){
			b[0]=-1;
			exemploEntrada(bin, "ls.exe", 0, -1, b, 1, -1);
			b[0]=-1;
			exemploEntrada(bin, "cat.exe", 0, -1, b, 1, -1);
			atualizarDiretorio(bin);
		}

		atualizarDiretorio(raiz);

		/* ---- blocos defeituosos da figura A (deixam arquivos corrompidos) ---- */
		marcarBad(33);   /* Retrato.jpg */
		marcarBad(37);   /* Format.exe  */
		marcarBad(44);   /* Carta.doc   */
		marcarBad(55);   /* Programa.c  */

		/* ---- blocos perdidos da figura A ---- */
		marcarPerdido(8);
		marcarPerdido(9);
		marcarPerdido(15);
		marcarPerdido(21);
	}
}

#endif
