#ifndef TADCOMANDOS_H
#define TADCOMANDOS_H

#include "TADTela.h"
#include "TADInode.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <conio2.h>

/* ============================================================
   COMANDOS DO PROMPT
   Todos gravam o resultado no buffer de saida (escrever) e o
   main mostra o resultado com mostrarSaida().
   As validacoes usam a variavel "ok": quando alguma falha,
   ok vira 0 e o resto do comando nao e executado.
   ============================================================ */

/* devolve a letra do tipo da entrada: d=diretorio l=link -=arquivo */
char tipoDaEntrada(TpEntrada *n){
	char tipo = '-';

	if(n->ehLinkSimbolico){
		tipo = 'l';
	}
	else{
		if(n->ehDiretorio){
			tipo = 'd';
		}
	}
	return tipo;
}

/* devolve a cor usada para mostrar a entrada na tela */
int corDaEntrada(TpEntrada *n){
	int cor = 7;

	if(n->ehDiretorio){
		cor = 11;
	}
	else{
		if(n->ehLinkSimbolico){
			cor = 13;
		}
	}
	return cor;
}

/* ---------------- ls / ls -l ---------------- */
void cmdLs(int detalhado){
	TpEntrada *aux;
	TpInode *ip, *ipPai;
	char caminho[MAX_CAMINHO], seta[20], alvoLink[MAX_CAMINHO];
	int qtd = 0;

	ipPai = buscarInode(atual->inode);
	if(!temPermissao(ipPai, 'r')){
		escrever(12, "ls: permissao negada para leitura do diretorio.");
	}
	else{
		caminhoDe(atual, caminho);
		escrever(14, "Arquivo de diretorio: %s   (inode principal %d)", caminho, atual->inode);
		escrever(8, "");

		if(detalhado){
			escrever(11, "%-11s %-5s %-8s %-6s %-9s %-11s %-6s %-6s %s",
					 "PERMISSOES", "LINKS", "DONO", "GRUPO", "TAMANHO", "DATA", "HORA", "INODE", "NOME");
			escrever(8, "-------------------------------------------------------------------------------------------");

			ip = buscarInode(atual->inode);
			escrever(7, "%-11s %-5d %-8s %-6s %-9d %-11s %-6s %-6d %s",
					 ip->permissoes, ip->links, ip->usuario, ip->grupo, ip->tamanho,
					 ip->data, ip->hora, atual->inode, ".");

			if(atual->pai != NULL){
				ip = buscarInode(atual->pai->inode);
			}
			else{
				ip = buscarInode(atual->inode);
			}
			escrever(7, "%-11s %-5d %-8s %-6s %-9d %-11s %-6s %-6d %s",
					 ip->permissoes, ip->links, ip->usuario, ip->grupo, ip->tamanho,
					 ip->data, ip->hora, ip->bloco, "..");

			aux = atual->filhos.inicio;
			while(aux != NULL){
				ip = buscarInode(aux->inode);
				if(ip != NULL){
					strcpy(seta, "");
					strcpy(alvoLink, "");
					if(aux->ehLinkSimbolico){
						strcpy(seta, " -> ");
						strcpy(alvoLink, aux->destino);
					}
					else{
						if(aux->ehLinkFisico){
							strcpy(seta, "  (link fisico)");
						}
					}

					escrever(corDaEntrada(aux),
							 "%-11s %-5d %-8s %-6s %-9d %-11s %-6s %-6d %s%s%s",
							 ip->permissoes, ip->links, ip->usuario, ip->grupo, ip->tamanho,
							 ip->data, ip->hora, aux->inode, aux->nome, seta, alvoLink);
					qtd++;
				}
				aux = aux->prox;
			}
		}
		else{
			escrever(7, ".    ..");
			aux = atual->filhos.inicio;
			while(aux != NULL){
				escrever(corDaEntrada(aux),
						 "%c %-28s  bloco inicial: %d", tipoDaEntrada(aux), aux->nome, aux->inode);
				qtd++;
				aux = aux->prox;
			}
		}

		escrever(8, "");
		escrever(10, "Total de entradas: %d (+ . e ..)", qtd);
	}
}

/* ---------------- cd ---------------- */
void cmdCd(char caminho[]){
	TpEntrada *destino;
	TpInode *ip;
	char c[MAX_CAMINHO];

	destino = resolverNo(atual, caminho);
	if(destino == NULL){
		escrever(12, "cd: diretorio '%s' nao encontrado.", caminho);
	}
	else{
		if(!destino->ehDiretorio){
			escrever(12, "cd: '%s' nao e um diretorio.", caminho);
		}
		else{
			ip = buscarInode(destino->inode);
			if(!temPermissao(ip, 'x')){
				escrever(12, "cd: permissao negada (sem direito de execucao no diretorio).");
			}
			else{
				atual = destino;
				caminhoDe(atual, c);
				escrever(10, "Diretorio atual: %s", c);
			}
		}
	}
}

/* ---------------- touch ---------------- */
void cmdTouch(char caminho[], int bytes){
	char nome[MAX_NOME];
	TpEntrada *pai, *novo;
	TpInode *ip;
	int dados, total, bloco, i, ok = 1;
	int vd[MAX_LISTA], vi[MAX_LISTA], qd, qi;
	char linha[LARG_LINHA];
	char tmp[16];

	if(bytes < 1){
		bytes = 1;
	}

	dados = (bytes + TAM_BLOCO - 1) / TAM_BLOCO;
	total = blocosNecessarios(bytes);

	pai = resolverPai(atual, caminho, nome);
	if(pai == NULL || !pai->ehDiretorio){
		escrever(12, "touch: caminho invalido.");
		ok = 0;
	}
	if(ok && nome[0] == '\0'){
		escrever(12, "touch: informe o nome do arquivo.");
		ok = 0;
	}
	if(ok && buscarFilho(pai, nome) != NULL){
		escrever(12, "touch: ja existe '%s' nesse diretorio.", nome);
		ok = 0;
	}
	if(ok && !temPermissao(buscarInode(pai->inode), 'w')){
		escrever(12, "touch: permissao negada para gravar nesse diretorio.");
		ok = 0;
	}
	if(ok && dados > MAX_BLOCOS_ARQ){
		escrever(12, "touch: tamanho maximo suportado e de %d bytes (%d blocos).",
				 MAX_BLOCOS_ARQ * TAM_BLOCO, MAX_BLOCOS_ARQ);
		ok = 0;
	}
	if(ok && total + 1 > livres.qtde){
		escrever(12, "touch: espaco insuficiente. Precisa de %d blocos e existem %d livres.",
				 total, livres.qtde);
		ok = 0;
	}

	if(ok){
		bloco = alocarBloco();
		ip = criarInode(bloco, 1, 0, "-rw-r--r--", bytes);
		alocarArquivo(ip, dados);

		novo = criarNo(nome, bloco, 0);
		inserirFilho(pai, novo);
		atualizarDiretorio(pai);

		blocosDoArquivo(ip, vd, qd, vi, qi);

		escrever(10, "Arquivo '%s' criado com %d bytes.", nome, bytes);
		escrever(7, "Inode principal gravado no bloco %d.", bloco);

		strcpy(linha, "Blocos de dados: ");
		for(i=0; i<qd; i++){
			sprintf(tmp, "%d ", vd[i]);
			if(strlen(linha) + strlen(tmp) < LARG_LINHA - 2){
				strcat(linha, tmp);
			}
		}
		escrever(7, "%s", linha);

		if(qi > 1){
			strcpy(linha, "Inodes de extensao (indiretos): ");
			for(i=1; i<qi; i++){
				sprintf(tmp, "%d ", vi[i]);
				if(strlen(linha) + strlen(tmp) < LARG_LINHA - 2){
					strcat(linha, tmp);
				}
			}
			escrever(11, "%s", linha);
		}
		escrever(8, "Total de blocos gastos: %d", qd + qi);
	}
}

/* ---------------- mkdir ---------------- */
void cmdMkdir(char caminho[]){
	char nome[MAX_NOME], sub[MAX_CAMINHO];
	TpEntrada *pai, *novo;
	int bloco, ok = 1;

	pai = resolverPai(atual, caminho, nome);
	if(pai == NULL || !pai->ehDiretorio){
		escrever(12, "mkdir: caminho invalido.");
		ok = 0;
	}
	if(ok && nome[0] == '\0'){
		escrever(12, "mkdir: informe o nome do diretorio.");
		ok = 0;
	}
	if(ok && buscarFilho(pai, nome) != NULL){
		escrever(12, "mkdir: ja existe '%s' nesse diretorio.", nome);
		ok = 0;
	}
	if(ok && !temPermissao(buscarInode(pai->inode), 'w')){
		escrever(12, "mkdir: permissao negada para gravar nesse diretorio.");
		ok = 0;
	}
	if(ok && livres.qtde < 6){
		escrever(12, "mkdir: espaco insuficiente no disco.");
		ok = 0;
	}

	if(ok){
		bloco = alocarBloco();
		criarInode(bloco, 1, 0, "drwxr-xr-x", TAM_BLOCO);

		novo = criarNo(nome, bloco, 1);
		inserirFilho(pai, novo);
		atualizarDiretorio(novo);
		atualizarDiretorio(pai);

		escrever(10, "Diretorio '%s' criado. Inode principal no bloco %d.", nome, bloco);
		escrever(8, "");

		/* o enunciado pede pelo menos dois arquivos dentro do novo diretorio */
		caminhoDe(novo, sub);
		if(strcmp(sub, "/") != 0){
			strcat(sub, "/");
		}
		strcat(sub, "leiame.txt");
		cmdTouch(sub, 25);

		caminhoDe(novo, sub);
		if(strcmp(sub, "/") != 0){
			strcat(sub, "/");
		}
		strcat(sub, "notas.txt");
		cmdTouch(sub, 15);
	}
}

/* ---------------- rm ---------------- */
void cmdRm(char caminho[]){
	char nome[MAX_NOME];
	TpEntrada *pai, *alvo = NULL;
	TpInode *ip;
	int vd[MAX_LISTA], vi[MAX_LISTA], qd, qi, ok = 1;

	pai = resolverPai(atual, caminho, nome);
	if(pai == NULL){
		escrever(12, "rm: caminho invalido.");
		ok = 0;
	}
	if(ok){
		alvo = buscarFilho(pai, nome);
		if(alvo == NULL){
			escrever(12, "rm: arquivo '%s' nao encontrado.", nome);
			ok = 0;
		}
	}
	if(ok && alvo->ehDiretorio){
		escrever(12, "rm: '%s' e um diretorio. Use rmdir.", nome);
		ok = 0;
	}
	if(ok && !temPermissao(buscarInode(pai->inode), 'w')){
		escrever(12, "rm: permissao negada nesse diretorio.");
		ok = 0;
	}

	if(ok){
		ip = buscarInode(alvo->inode);

		/* se existirem outros links fisicos, apenas a entrada e removida */
		if(ip != NULL && ip->links > 1 && !alvo->ehLinkSimbolico){
			ip->links--;
			removerFilho(pai, nome);
			atualizarDiretorio(pai);
			escrever(10, "Entrada '%s' removida. O inode %d continua no disco (%d link(s)).",
					 nome, ip->bloco, ip->links);
		}
		else{
			blocosDoArquivo(ip, vd, qd, vi, qi);
			liberarConteudo(ip);
			removerInode(ip->bloco);
			liberarBloco(alvo->inode);
			removerFilho(pai, nome);
			atualizarDiretorio(pai);

			escrever(10, "Arquivo '%s' deletado.", nome);
			escrever(7, "%d bloco(s) de dados e %d bloco(s) de inode voltaram para a pilha de livres.", qd, qi);
			escrever(8, "Blocos livres agora: %d", livres.qtde);
		}
	}
}

/* ---------------- rmdir ---------------- */
void cmdRmdir(char caminho[]){
	char nome[MAX_NOME];
	TpEntrada *pai, *alvo = NULL;
	TpInode *ip;
	int ok = 1;

	pai = resolverPai(atual, caminho, nome);
	if(pai == NULL){
		escrever(12, "rmdir: caminho invalido.");
		ok = 0;
	}
	if(ok){
		alvo = buscarFilho(pai, nome);
		if(alvo == NULL || !alvo->ehDiretorio){
			escrever(12, "rmdir: diretorio '%s' nao encontrado.", nome);
			ok = 0;
		}
	}
	if(ok && alvo->filhos.qtde > 0){
		escrever(12, "rmdir: o diretorio '%s' nao esta vazio.", nome);
		ok = 0;
	}
	if(ok && alvo == atual){
		escrever(12, "rmdir: nao e possivel remover o diretorio atual.");
		ok = 0;
	}

	if(ok){
		ip = buscarInode(alvo->inode);
		liberarConteudo(ip);
		removerInode(ip->bloco);
		liberarBloco(alvo->inode);
		removerFilho(pai, nome);
		atualizarDiretorio(pai);

		escrever(10, "Diretorio '%s' removido. Blocos devolvidos para a pilha de livres.", nome);
	}
}

/* ---------------- vi ---------------- */
void cmdVi(char caminho[]){
	TpEntrada *alvo;
	TpInode *ip = NULL;
	int ok = 1;

	alvo = resolverNo(atual, caminho);
	if(alvo == NULL){
		escrever(12, "vi: arquivo '%s' nao encontrado.", caminho);
		ok = 0;
	}
	if(ok && alvo->ehDiretorio){
		escrever(12, "vi: '%s' e um diretorio.", caminho);
		ok = 0;
	}
	if(ok){
		ip = buscarInode(alvo->inode);
		if(ip == NULL){
			escrever(12, "vi: inode do arquivo nao encontrado.");
			ok = 0;
		}
	}
	if(ok && !temPermissao(ip, 'r')){
		escrever(12, "vi: permissao de leitura negada para '%s'.", alvo->nome);
		ok = 0;
	}
	if(ok && arquivoCorrompido(ip)){
		escrever(12, "vi: o arquivo '%s' esta CORROMPIDO por bloco(s) defeituoso(s).", alvo->nome);
		ok = 0;
	}

	if(ok){
		escrever(10, "Arquivo '%s' visualizado com sucesso.", alvo->nome);
		escrever(8, "(Simulacao: o conteudo real do arquivo nao e aberto.)");
	}
}

/* ---------------- chmod ---------------- */
void cmdChmod(char quem[], char bits[], char caminho[]){
	TpEntrada *alvo = NULL;
	TpInode *ip = NULL;
	int adicionar = 0, i, base, ok = 1;
	char grupos[4];
	int ng = 0;

	if(quem == NULL || bits == NULL || caminho == NULL || quem[0] == '\0'){
		escrever(12, "chmod: use chmod +ugo rwx NomeArquivo");
		ok = 0;
	}
	if(ok && quem[0] != '+' && quem[0] != '-'){
		escrever(12, "chmod: o primeiro caractere deve ser + ou -.");
		ok = 0;
	}

	if(ok){
		adicionar = quem[0] == '+';

		i = 1;
		while(quem[i] != '\0' && ok){
			if((quem[i] == 'u' || quem[i] == 'g' || quem[i] == 'o') && ng < 3){
				grupos[ng++] = quem[i];
			}
			else{
				escrever(12, "chmod: grupo invalido em '%s'. Use u, g e/ou o.", quem);
				ok = 0;
			}
			i++;
		}
	}
	if(ok && ng == 0){
		escrever(12, "chmod: informe u, g e/ou o depois de + ou -.");
		ok = 0;
	}
	if(ok){
		alvo = resolverNo(atual, caminho);
		if(alvo == NULL){
			escrever(12, "chmod: '%s' nao encontrado.", caminho);
			ok = 0;
		}
	}
	if(ok){
		ip = buscarInode(alvo->inode);
		if(ip == NULL){
			escrever(12, "chmod: inode nao encontrado.");
			ok = 0;
		}
	}

	if(ok){
		for(i=0; i<ng; i++){
			/* posicao do bit r de cada grupo: u=1 g=4 o=7 */
			switch(grupos[i]){
				case 'u':
					base = 1;
					break;
				case 'g':
					base = 4;
					break;
				default:
					base = 7;
			}

			if(strchr(bits, 'r') != NULL){
				if(adicionar){
					ip->permissoes[base] = 'r';
				}
				else{
					ip->permissoes[base] = '-';
				}
			}
			if(strchr(bits, 'w') != NULL){
				if(adicionar){
					ip->permissoes[base+1] = 'w';
				}
				else{
					ip->permissoes[base+1] = '-';
				}
			}
			if(strchr(bits, 'x') != NULL){
				if(adicionar){
					ip->permissoes[base+2] = 'x';
				}
				else{
					ip->permissoes[base+2] = '-';
				}
			}
		}

		escrever(10, "Permissoes de '%s' alteradas para %s.", alvo->nome, ip->permissoes);
	}
}

/* ---------------- bad ---------------- */
void cmdBad(int bloco){
	if(bloco < 0 || bloco >= totalBlocos){
		escrever(12, "bad: bloco %d fora do disco (0 a %d).", bloco, totalBlocos-1);
	}
	else{
		if(disco[bloco] == BAD){
			escrever(14, "bad: o bloco %d ja estava marcado como defeituoso.", bloco);
		}
		else{
			if(disco[bloco] == LIVRE){
				reservarBloco(bloco);
				disco[bloco] = BAD;
				escrever(10, "Bloco %d marcado como defeituoso (B) e retirado da pilha de livres.", bloco);
			}
			else{
				escrever(14, "Atencao: o bloco %d estava em uso (tipo %c).", bloco, disco[bloco]);
				disco[bloco] = BAD;
				escrever(12, "Bloco %d marcado como defeituoso. O arquivo dono ficou CORROMPIDO.", bloco);
			}
		}
	}
}

/* ---------------- df ---------------- */
void cmdDf(){
	int i, lvr = 0, bad = 0, ino = 0, arq = 0;

	for(i=0; i<totalBlocos; i++){
		switch(disco[i]){
			case LIVRE:
				lvr++;
				break;
			case BAD:
				bad++;
				break;
			case TIPO_INODE:
				ino++;
				break;
			default:
				arq++;
		}
	}

	escrever(14, "ESPACO EM DISCO");
	escrever(8, "");
	escrever(7, "Tamanho do bloco ....... %d bytes", TAM_BLOCO);
	escrever(7, "Total do disco ......... %d blocos = %d bytes", totalBlocos, totalBlocos*TAM_BLOCO);
	escrever(10, "Livres (F) ............. %d blocos = %d bytes", lvr, lvr*TAM_BLOCO);
	escrever(7, "Ocupados por arquivos(A) %d blocos = %d bytes", arq, arq*TAM_BLOCO);
	escrever(11, "Ocupados por inodes (I)  %d blocos = %d bytes", ino, ino*TAM_BLOCO);
	escrever(12, "Defeituosos (B) ........ %d blocos = %d bytes", bad, bad*TAM_BLOCO);
	escrever(8, "");
	escrever(15, "Total ocupado .......... %d blocos = %d bytes", arq+ino, (arq+ino)*TAM_BLOCO);
	escrever(8, "Blocos na pilha de livres: %d", livres.qtde);
}

/* ---------------- link -h (link fisico) ---------------- */
void cmdLinkFisico(char origem[], char destino[]){
	TpEntrada *orig, *pai = NULL, *novo;
	TpInode *ip = NULL;
	char nome[MAX_NOME];
	int ok = 1;

	orig = resolverNo(atual, origem);
	if(orig == NULL){
		escrever(12, "link -h: origem '%s' nao encontrada.", origem);
		ok = 0;
	}
	if(ok && orig->ehDiretorio){
		escrever(12, "link -h: nao e permitido criar link fisico para diretorio.");
		ok = 0;
	}
	if(ok){
		ip = buscarInode(orig->inode);
		if(ip == NULL || !temPermissao(ip, 'r')){
			escrever(12, "link -h: permissao de leitura negada na origem.");
			ok = 0;
		}
	}
	if(ok){
		pai = resolverPai(atual, destino, nome);
		if(pai == NULL || !pai->ehDiretorio){
			escrever(12, "link -h: caminho de destino invalido.");
			ok = 0;
		}
	}
	if(ok && buscarFilho(pai, nome) != NULL){
		escrever(12, "link -h: ja existe uma entrada chamada '%s'.", nome);
		ok = 0;
	}
	if(ok && !temPermissao(buscarInode(pai->inode), 'w')){
		escrever(12, "link -h: permissao de escrita negada no diretorio de destino.");
		ok = 0;
	}

	if(ok){
		novo = criarNo(nome, orig->inode, 0);
		novo->ehLinkFisico = 1;
		inserirFilho(pai, novo);
		ip->links++;
		atualizarDiretorio(pai);

		escrever(10, "Link fisico '%s' criado para '%s'.", nome, orig->nome);
		escrever(7, "Ambas as entradas apontam para o inode %d. Links fisicos: %d.", ip->bloco, ip->links);
	}
}

/* ---------------- link -s (link simbolico) ---------------- */
void cmdLinkSimbolico(char origem[], char destino[]){
	TpEntrada *orig, *pai = NULL, *novo;
	TpInode *ip = NULL;
	char nome[MAX_NOME], caminho[MAX_CAMINHO];
	int b = -1, qtdDados = 1, custo, ok = 1;

	orig = resolverNo(atual, origem);
	if(orig == NULL){
		escrever(12, "link -s: origem '%s' nao encontrada.", origem);
		ok = 0;
	}
	if(ok){
		ip = buscarInode(orig->inode);
		if(ip == NULL || !temPermissao(ip, 'r')){
			escrever(12, "link -s: permissao de leitura negada na origem.");
			ok = 0;
		}
	}
	if(ok){
		pai = resolverPai(atual, destino, nome);
		if(pai == NULL || !pai->ehDiretorio){
			escrever(12, "link -s: caminho de destino invalido.");
			ok = 0;
		}
	}
	if(ok && buscarFilho(pai, nome) != NULL){
		escrever(12, "link -s: ja existe uma entrada chamada '%s'.", nome);
		ok = 0;
	}
	if(ok && !temPermissao(buscarInode(pai->inode), 'w')){
		escrever(12, "link -s: permissao de escrita negada no diretorio de destino.");
		ok = 0;
	}
	if(ok){
		caminhoDe(orig, caminho);
		qtdDados = (strlen(caminho) + TAM_BLOCO - 1) / TAM_BLOCO;
		if(qtdDados < 1){
			qtdDados = 1;
		}
		custo = 1 + qtdDados + inodesExtras(qtdDados);
		if(custo > livres.qtde){
			escrever(12, "link -s: nao ha blocos livres suficientes.");
			ok = 0;
		}
	}
	if(ok){
		b = alocarBloco();
		if(b < 0){
			escrever(12, "link -s: nao foi possivel alocar inode.");
			ok = 0;
		}
	}
	if(ok){
		ip = criarInode(b, 1, 0, "lrwxrwxrwx", strlen(caminho));
		if(!alocarArquivo(ip, qtdDados)){
			removerInode(b);
			liberarBloco(b);
			escrever(12, "link -s: falha na alocacao dos blocos.");
			ok = 0;
		}
	}

	if(ok){
		novo = criarNo(nome, b, 0);
		novo->ehLinkSimbolico = 1;
		strncpy(novo->destino, caminho, MAX_CAMINHO-1);
		novo->destino[MAX_CAMINHO-1] = '\0';
		inserirFilho(pai, novo);
		atualizarDiretorio(pai);

		escrever(10, "Link simbolico '%s' criado.", nome);
		escrever(7, "inode proprio: %d | caminho armazenado: %s", b, novo->destino);
	}
}

/* ---------------- unlink -h / unlink -s ---------------- */
void cmdUnlink(int simbolico, char nomeCaminho[]){
	TpEntrada *pai, *alvo = NULL;
	TpInode *ip;
	char nome[MAX_NOME];
	int ok = 1, bloco;

	pai = resolverPai(atual, nomeCaminho, nome);
	if(pai == NULL){
		escrever(12, "unlink: caminho invalido.");
		ok = 0;
	}
	if(ok){
		alvo = buscarFilho(pai, nome);
		if(alvo == NULL){
			escrever(12, "unlink: entrada '%s' nao encontrada.", nome);
			ok = 0;
		}
	}

	if(ok){
		if(simbolico){
			if(!alvo->ehLinkSimbolico){
				escrever(12, "unlink -s: '%s' nao e um link simbolico.", nome);
			}
			else{
				ip = buscarInode(alvo->inode);
				if(ip != NULL){
					liberarConteudo(ip);
					removerInode(ip->bloco);
					liberarBloco(alvo->inode);
				}
				removerFilho(pai, nome);
				atualizarDiretorio(pai);
				escrever(10, "Link simbolico '%s' removido e seus blocos foram liberados.", nome);
			}
		}
		else{
			if(!alvo->ehLinkFisico){
				escrever(12, "unlink -h: '%s' nao e uma entrada de link fisico.", nome);
			}
			else{
				ip = buscarInode(alvo->inode);
				if(ip != NULL && ip->links > 0){
					ip->links--;
				}
				removerFilho(pai, nome);
				atualizarDiretorio(pai);
				escrever(10, "Link fisico '%s' removido.", nome);
				if(ip != NULL){
					if(ip->links == 0){
						/* era o ultimo link: o arquivo deixa de existir */
						bloco = ip->bloco;
						liberarConteudo(ip);
						removerInode(bloco);
						liberarBloco(bloco);
						escrever(7, "Era o ultimo link: inode %d e seus blocos voltaram para a pilha de livres.", bloco);
					}
					else{
						escrever(7, "inode %d permanece com %d link(s).", ip->bloco, ip->links);
					}
				}
			}
		}
	}
}

/* ============================================================
   RELATORIOS
   ============================================================ */

/* 1 - blocos ocupados por um arquivo escolhido */
void relBlocosArquivo(char nome[]){
	TpEntrada *alvo;
	TpInode *ip;
	int dados[MAX_LISTA], inodes[MAX_LISTA], qd, qi, i, tam = 0;

	alvo = resolverNo(atual, nome);
	if(alvo == NULL){
		escrever(12, "Arquivo/diretorio '%s' nao encontrado.", nome);
	}
	else{
		ip = buscarInode(alvo->inode);
		blocosDoArquivo(ip, dados, qd, inodes, qi);
		if(ip != NULL){
			tam = ip->tamanho;
		}

		escrever(14, "BLOCOS OCUPADOS POR: %s", alvo->nome);
		escrever(7, "Tamanho logico: %d bytes", tam);
		escrever(11, "Inodes (%d):", qi);
		for(i=0; i<qi; i++){
			escrever(11, "  inode[%d] = bloco %d", i, inodes[i]);
		}
		escrever(15, "Dados (%d):", qd);
		for(i=0; i<qd; i++){
			escrever(15, "  dado[%d] = bloco %d", i, dados[i]);
		}

		escrever(10, "Total fisico: %d bloco(s) = %d bytes.", qd+qi, (qd+qi)*TAM_BLOCO);
	}
}

/* 2 - maior arquivo que ainda pode ser criado */
void relMaiorArquivo(){
	int d, melhor = 0, custo = 0, c;

	for(d=1; d<=MAX_BLOCOS_ARQ; d++){
		c = 1 + d + inodesExtras(d);
		if(c <= livres.qtde){
			melhor = d;
			custo = c;
		}
	}

	escrever(14, "MAIOR ARQUIVO QUE AINDA PODE SER CRIADO");
	escrever(7, "Blocos livres no disco: %d", livres.qtde);
	if(melhor == 0){
		escrever(12, "Nao ha espaco nem para inode + 1 bloco de dados.");
	}
	else{
		escrever(10, "Maior arquivo: %d bloco(s) de dados = %d bytes.", melhor, melhor*TAM_BLOCO);
		escrever(7, "Custo total da alocacao: %d bloco(s), contando inode(s).", custo);
	}
}

/* percorre a arvore listando integros e corrompidos */
void relIntegridadeRec(TpEntrada *n, int &ok, int &ruim){
	char caminho[MAX_CAMINHO];
	TpInode *ip;
	TpEntrada *f;

	if(n != NULL){
		if(n != raiz && !n->ehDiretorio){
			ip = buscarInode(n->inode);
			caminhoDe(n, caminho);
			if(arquivoCorrompido(ip)){
				escrever(12, "[CORROMPIDO] %s", caminho);
				ruim++;
			}
			else{
				escrever(10, "[INTEGRO]    %s", caminho);
				ok++;
			}
		}

		f = n->filhos.inicio;
		while(f != NULL){
			relIntegridadeRec(f, ok, ruim);
			f = f->prox;
		}
	}
}

/* marca no vetor todos os blocos usados pelos arquivos da arvore */
void marcarUsadosRec(TpEntrada *n, char usado[]){
	int dados[MAX_LISTA], inodes[MAX_LISTA], qd, qi, i;
	TpInode *ip;
	TpEntrada *f;

	if(n != NULL){
		ip = buscarInode(n->inode);
		if(ip != NULL){
			blocosDoArquivo(ip, dados, qd, inodes, qi);
			for(i=0; i<qd; i++){
				if(dados[i] >= 0 && dados[i] < totalBlocos){
					usado[dados[i]] = 1;
				}
			}
			for(i=0; i<qi; i++){
				if(inodes[i] >= 0 && inodes[i] < totalBlocos){
					usado[inodes[i]] = 1;
				}
			}
		}

		f = n->filhos.inicio;
		while(f != NULL){
			marcarUsadosRec(f, usado);
			f = f->prox;
		}
	}
}

void explorerRec(TpEntrada *n, int nivel){
	TpEntrada *f;
	TpInode *ip;
	char prefixo[100], tipo[8], tipoBloco = '?';
	int i, k = 0, cor = 15, tam = 0;

	if(n != NULL){
		prefixo[0] = '\0';
		i = 0;
		while(i < nivel && k < 90){
			strcat(prefixo, "|   ");
			k = k + 4;
			i++;
		}

		if(n->ehLinkSimbolico){
			strcpy(tipo, "[LNKS]");
			cor = 14;
		}
		else{
			if(n->ehLinkFisico){
				strcpy(tipo, "[LNKH]");
			}
			else{
				if(n->ehDiretorio){
					strcpy(tipo, "[DIR]");
					cor = 11;
				}
				else{
					strcpy(tipo, "[ARQ]");
				}
			}
		}

		ip = buscarInode(n->inode);
		if(ip != NULL){
			tam = ip->tamanho;
		}
		if(n->inode >= 0 && n->inode < totalBlocos){
			tipoBloco = disco[n->inode];
		}

		escrever(cor, "%s%-6s %-24s inode=%-4d tipo=%c tam=%d",
				 prefixo, tipo, n->nome, n->inode, tipoBloco, tam);

		f = n->filhos.inicio;
		while(f != NULL){
			explorerRec(f, nivel+1);
			f = f->prox;
		}
	}
}

void imprimirExtensaoRec(int bloco, int nivel, int indent){
	TpInode *ip = NULL;
	int i;

	if(bloco >= 0){
		ip = buscarInode(bloco);
	}

	if(ip != NULL){
		escrever(8, "%*sInode indireto nivel %d: bloco %d", indent, "", nivel, bloco);
		for(i=0; i<PONT_INODE; i++){
			if(ip->ponteiros[i] >= 0){
				if(nivel == 1){
					escrever(7, "%*s-> dado bloco %d", indent+2, "", ip->ponteiros[i]);
				}
				else{
					imprimirExtensaoRec(ip->ponteiros[i], nivel-1, indent+2);
				}
			}
		}
	}
}

void arvoreInodesRec(TpEntrada *n, int nivel){
	TpEntrada *f;
	TpInode *ip;
	char caminho[MAX_CAMINHO];
	int i;

	if(n != NULL){
		ip = buscarInode(n->inode);
		caminhoDe(n, caminho);
		if(ip != NULL){
			escrever(14, "%*s%s  [inode %d]", nivel*2, "", caminho, ip->bloco);
			escrever(7, "%*sperm=%s dono=%s grupo=%s tam=%d data=%s hora=%s links=%d",
					 nivel*2+2, "", ip->permissoes, ip->usuario, ip->grupo, ip->tamanho, ip->data, ip->hora, ip->links);
			for(i=0; i<DIRETOS; i++){
				if(ip->ponteiros[i] >= 0){
					escrever(15, "%*sdireto[%d] -> bloco %d", nivel*2+4, "", i, ip->ponteiros[i]);
				}
			}
			if(ip->ponteiros[5] >= 0){
				imprimirExtensaoRec(ip->ponteiros[5], 1, nivel*2+4);
			}
			if(ip->ponteiros[6] >= 0){
				imprimirExtensaoRec(ip->ponteiros[6], 2, nivel*2+4);
			}
			if(ip->ponteiros[7] >= 0){
				imprimirExtensaoRec(ip->ponteiros[7], 3, nivel*2+4);
			}
		}

		f = n->filhos.inicio;
		while(f != NULL){
			arvoreInodesRec(f, nivel+1);
			f = f->prox;
		}
	}
}

void linksRec(TpEntrada *n, int &fis, int &sim){
	TpEntrada *f;
	TpInode *ip;
	char caminho[MAX_CAMINHO];
	int qtdLinks = 0;

	if(n != NULL){
		if(n->ehLinkSimbolico){
			caminhoDe(n, caminho);
			escrever(14, "[SIMBOLICO] %s -> %s | inode proprio %d", caminho, n->destino, n->inode);
			sim++;
		}
		else{
			if(n->ehLinkFisico){
				caminhoDe(n, caminho);
				ip = buscarInode(n->inode);
				if(ip != NULL){
					qtdLinks = ip->links;
				}
				escrever(11, "[FISICO]    %s -> inode %d | contador links=%d",
						 caminho, n->inode, qtdLinks);
				fis++;
			}
		}

		f = n->filhos.inicio;
		while(f != NULL){
			linksRec(f, fis, sim);
			f = f->prox;
		}
	}
}

/* 3 - arquivos integros x corrompidos */
void relIntegridade(){
	int ok = 0, ruim = 0;

	escrever(14, "INTEGRIDADE DOS ARQUIVOS");
	escrever(8, "");
	relIntegridadeRec(raiz, ok, ruim);
	escrever(8, "");
	escrever(10, "Integros: %d", ok);
	escrever(12, "Corrompidos: %d", ruim);
}

/* 4 - blocos perdidos */
void relPerdidos(){
	char usado[MAX_BLOCOS];
	int i, qtd = 0;

	for(i=0; i<MAX_BLOCOS; i++){
		usado[i] = 0;
	}
	marcarUsadosRec(raiz, usado);

	escrever(14, "BLOCOS PERDIDOS");
	for(i=0; i<totalBlocos; i++){
		if(!usado[i] && disco[i] != LIVRE && disco[i] != BAD){
			escrever(12, "Bloco perdido: %d (estado %c)", i, disco[i]);
			qtd++;
		}
	}
	if(qtd == 0){
		escrever(10, "Nenhum bloco perdido encontrado.");
	}
	escrever(7, "Total: %d bloco(s) = %d bytes perdidos.", qtd, qtd*TAM_BLOCO);
}

/* 5 - imprime o mapa do disco com cores (igual a figura A) */
void telaBlocos(){
	int i, linha = 12, col, porLinha = 40;

	MolduraMenu();
	Titulo();
	Moldura(3,6,117,28,8,0);
	gotoxy(5,7); textcolor(14); printf("RELATORIO 5 - MAPA ATUAL DOS BLOCOS");
	gotoxy(5,9); textcolor(8); printf("F=livre  A=dados  I=inode  B=defeituoso");

	for(i=0; i<totalBlocos; i++){
		if(i % porLinha == 0){
			if(linha >= 26){
				gotoxy(5,27); textcolor(8); printf("Pressione uma tecla para continuar...");
				getch();
				clrscr();
				MolduraMenu();
				Titulo();
				Moldura(3,6,117,28,8,0);
				gotoxy(5,7); textcolor(14); printf("RELATORIO 5 - MAPA ATUAL DOS BLOCOS (continuacao)");
				linha = 10;
			}
			gotoxy(5,linha); textcolor(8); printf("%4d: ", i);
		}

		col = 11 + (i % porLinha) * 2;
		gotoxy(col,linha);

		switch(disco[i]){
			case LIVRE:
				textcolor(10);
				break;
			case ARQUIVO:
				textcolor(15);
				break;
			case TIPO_INODE:
				textcolor(11);
				break;
			default:
				textcolor(12);
		}
		printf("%c", disco[i]);

		if((i+1) % porLinha == 0 || i == totalBlocos-1){
			linha = linha + 2;
		}
	}
	pausarTela();
}

/* 6 - visualizacao estilo "windows explorer" */
void relExplorer(){
	escrever(14, "EXPLORER DO SISTEMA DE ARQUIVOS");
	escrever(8, "");
	explorerRec(raiz, 0);
}

/* 7 - arvore de diretorio detalhada (igual a figura D) */
void relArvoreInodes(){
	escrever(14, "ARVORE DE DIRETORIOS E INODES");
	escrever(8, "");
	arvoreInodesRec(raiz, 0);
}

/* 8 - links simbolicos e fisicos */
void relLinks(){
	int fis = 0, sim = 0;

	escrever(14, "LINKS FISICOS E SIMBOLICOS");
	escrever(8, "");
	linksRec(raiz, fis, sim);
	if(fis == 0 && sim == 0){
		escrever(8, "Nenhum link criado ate o momento.");
	}
	escrever(8, "");
	escrever(11, "Links fisicos: %d", fis);
	escrever(14, "Links simbolicos: %d", sim);
}

#endif
