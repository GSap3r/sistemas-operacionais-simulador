#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <conio2.h>
#include <windows.h>
#include <time.h>
#include "TADTela.h"
#include "TADInode.h"
#include "TADComandos.h"
#include "TADExemplo.h"
#include "TADFork.h"

/* ============================================================
   TRABALHO DE SISTEMAS OPERACIONAIS - BIMESTRE 1
   Simulacao do Sistema de Arquivos utilizado Inode
   FIPP / UNOESTE - Prof. Robson Siscoutto

   INTEGRANTES
   Gabriel Mendes Lopes ....... RA 262412810
   Igor Marcelino Santos ...... RA 262412985
   Guilherme Sales Pereira .... RA 262411873
   Thiago Marques Cristino .... RA 262412837

   INSTRUCOES PARA EXECUCAO
   1. Abrir o main.cpp no Dev-C++ (precisa da biblioteca conio2,
      em Projeto > Opcoes > Parametros > Linker: -lconio).
   2. Compilar e executar (F11).
   3. Na tela inicial pressionar qualquer tecla e depois ENTER.
   4. Digitar a quantidade de blocos do disco (ENTER = 1000).
   5. Escolher [S] para carregar o disco de exemplo do enunciado
      (figuras A, B e D) ou [N] para comecar com o disco vazio.
   6. Digitar os comandos no prompt (help mostra todos).
      Ex.: ls -l | touch teste.txt 120 | mkdir docs | cd docs
           link -s /Readme.txt atalho | bad 50 | df | rel
   7. Digitar exit para voltar ao menu e ESC para sair.

   ARQUIVOS
   TADInode.h ..... structs, pilha de blocos livres, lista de
                    inodes, arvore de diretorios e alocacao
   TADComandos.h .. comandos do prompt e os 8 relatorios
   TADFork.h ...... simulacao do fork()/wait() com pilha
   TADTela.h ...... moldura, cores e buffer de saida
   TADExemplo.h ... carga do disco de exemplo do enunciado
   ============================================================ */

TpPilhaProc processos;
int proxPID     = 2;   /* o PID 1 e do prompt (processo pai) */
int mostrarFork = 1;   /* mostra ou nao a tela do fork/wait  */

void menuRelatorios();

/* ---------- tela do prompt ---------- */
void telaPrompt(char caminho[]){
	MolduraMenu();
	Titulo();
	Moldura(3,6,117,25,8,0);

	gotoxy(5,7);
	textcolor(15);
	printf("DIRETORIO ATUAL: ");
	textcolor(11);
	printf("%.50s", caminho);

	gotoxy(80,7);
	textcolor(8);
	printf("Livres: ");
	textcolor(10);
	printf("%d", livres.qtde);
	textcolor(8);
	printf(" / %d blocos", totalBlocos);

	gotoxy(5,9);
	textcolor(14);
	printf("COMANDOS DISPONIVEIS");

	textcolor(7);
	gotoxy(6,11);  printf("ls                            lista o diretorio");
	gotoxy(6,12);  printf("ls -l                         lista com atributos");
	gotoxy(6,13);  printf("cd NomeDir | . | .. | /cam    navega nos diretorios");
	gotoxy(6,14);  printf("mkdir NomeDir                 cria diretorio (+2 arquivos)");
	gotoxy(6,15);  printf("rmdir NomeDir                 remove diretorio vazio");
	gotoxy(6,16);  printf("touch NomeArq Bytes           cria arquivo regular");
	gotoxy(6,17);  printf("rm NomeArq                    deleta arquivo");
	gotoxy(6,18);  printf("vi NomeArq                    visualiza arquivo");
	gotoxy(6,19);  printf("chmod +ugo rwx NomeArq        altera permissoes");

	gotoxy(62,11); printf("link -h origem destino        cria link fisico");
	gotoxy(62,12); printf("link -s origem destino        cria link simbolico");
	gotoxy(62,13); printf("unlink -h nome                remove link fisico");
	gotoxy(62,14); printf("unlink -s nome                remove link simbolico");
	gotoxy(62,15); printf("bad numeroBloco               marca bloco defeituoso");
	gotoxy(62,16); printf("df                            espaco livre/ocupado");
	gotoxy(62,17); printf("rel                           menu dos 8 relatorios");
	gotoxy(62,18); printf("fork                          liga/desliga tela do fork");
	gotoxy(62,19); printf("exit                          encerra o simulador");

	gotoxy(6,22);
	textcolor(8);
	printf("Obs.: ao digitar um comando o prompt executa fork() e fica em wait() ate o filho terminar.");

	gotoxy(5,27);
	textcolor(10);
	printf("%s@fipp", usuarioAtual);
	textcolor(15);
	printf(":");
	textcolor(11);
	printf("%.40s", caminho);
	textcolor(15);
	printf("$ ");
	textcolor(14);
}

/* ---------- ajuda detalhada ---------- */
void ajuda(){
	escrever(14, "AJUDA - COMANDOS DO SIMULADOR");
	escrever(8, "");
	escrever(7, "ls                             lista os nomes do arquivo de diretorio atual");
	escrever(7, "ls -l                          lista com permissoes, dono, grupo, tamanho, data e inode");
	escrever(7, "cd NomeDir | . | .. | /a/b     navegacao relativa ou absoluta");
	escrever(7, "mkdir NomeDir                  cria diretorio e ja insere dois arquivos dentro");
	escrever(7, "rmdir NomeDir                  remove diretorio somente se estiver vazio");
	escrever(7, "touch NomeArq Bytes            cria arquivo regular alocando os blocos necessarios");
	escrever(7, "rm NomeArq                     deleta o arquivo e devolve os blocos para a pilha");
	escrever(7, "vi NomeArq                     simula a visualizacao e avisa se estiver corrompido");
	escrever(7, "chmod +ugo rwx NomeArq         ex: chmod -o rw Readme.txt / chmod +ugo rwx bin");
	escrever(7, "link -h origem destino         link fisico (a entrada aponta para o mesmo inode)");
	escrever(7, "link -s origem destino         link simbolico (arquivo comum com o caminho da raiz)");
	escrever(7, "unlink -h nome                 remove o link fisico");
	escrever(7, "unlink -s nome                 remove o link simbolico e libera seus blocos");
	escrever(7, "bad numeroBloco                transforma um bloco em defeituoso (B)");
	escrever(7, "df                             espaco livre e ocupado do disco em bytes");
	escrever(7, "rel                            abre o menu dos 8 relatorios");
	escrever(7, "fork                           liga/desliga a tela de simulacao do fork/wait");
	escrever(7, "clear                          limpa a tela");
	escrever(7, "exit                           encerra o simulador");
	escrever(8, "");
	escrever(11, "Dica: os caminhos podem ser absolutos, ex: touch /Boot/teste.txt 120");
}

/* ---------- execucao do comando (feita pelo processo filho) ---------- */
/* retorna 0 quando o usuario digitou exit, senao retorna 1 */
int executarComando(char linha[]){
	char cmd[40], a1[120], a2[120], a3[120];
	int n, i, continuar = 1;

	cmd[0] = a1[0] = a2[0] = a3[0] = '\0';
	n = sscanf(linha, "%39s %119s %119s %119s", cmd, a1, a2, a3);

	if(n >= 1){
		for(i=0; cmd[i] != '\0'; i++){
			cmd[i] = tolower(cmd[i]);
		}

		if(strcmp(cmd,"exit") == 0 || strcmp(cmd,"sair") == 0){
			continuar = 0;
		}
		else if(strcmp(cmd,"ls") == 0){
			cmdLs(strcmp(a1,"-l") == 0);
		}
		else if(strcmp(cmd,"cd") == 0){
			if(n < 2){
				escrever(12, "cd: informe o diretorio.");
			}
			else{
				cmdCd(a1);
			}
		}
		else if(strcmp(cmd,"mkdir") == 0){
			if(n < 2){
				escrever(12, "mkdir: informe o nome do diretorio.");
			}
			else{
				cmdMkdir(a1);
			}
		}
		else if(strcmp(cmd,"rmdir") == 0){
			if(n < 2){
				escrever(12, "rmdir: informe o nome do diretorio.");
			}
			else{
				cmdRmdir(a1);
			}
		}
		else if(strcmp(cmd,"touch") == 0){
			if(n < 3){
				escrever(12, "touch: use touch NomeArquivo TamanhoBytes");
			}
			else{
				cmdTouch(a1, atoi(a2));
			}
		}
		else if(strcmp(cmd,"rm") == 0){
			if(n < 2){
				escrever(12, "rm: informe o nome do arquivo.");
			}
			else{
				cmdRm(a1);
			}
		}
		else if(strcmp(cmd,"vi") == 0){
			if(n < 2){
				escrever(12, "vi: informe o nome do arquivo.");
			}
			else{
				cmdVi(a1);
			}
		}
		else if(strcmp(cmd,"chmod") == 0){
			if(n < 4){
				escrever(12, "chmod: use chmod +ugo rwx NomeArquivo");
			}
			else{
				cmdChmod(a1, a2, a3);
			}
		}
		else if(strcmp(cmd,"bad") == 0){
			if(n < 2){
				escrever(12, "bad: informe o numero do bloco.");
			}
			else{
				cmdBad(atoi(a1));
			}
		}
		else if(strcmp(cmd,"df") == 0){
			cmdDf();
		}
		else if(strcmp(cmd,"link") == 0){
			if(n < 4){
				escrever(12, "link: use link -h|-s origem destino");
			}
			else{
				if(strcmp(a1,"-h") == 0){
					cmdLinkFisico(a2, a3);
				}
				else{
					if(strcmp(a1,"-s") == 0){
						cmdLinkSimbolico(a2, a3);
					}
					else{
						escrever(12, "link: opcao invalida (use -h ou -s).");
					}
				}
			}
		}
		else if(strcmp(cmd,"unlink") == 0){
			if(n < 3){
				escrever(12, "unlink: use unlink -h|-s nome");
			}
			else{
				if(strcmp(a1,"-h") == 0 || strcmp(a1,"-s") == 0){
					cmdUnlink(strcmp(a1,"-s") == 0, a2);
				}
				else{
					escrever(12, "unlink: opcao invalida (use -h ou -s).");
				}
			}
		}
		else if(strcmp(cmd,"rel") == 0 || strcmp(cmd,"relatorios") == 0){
			menuRelatorios();
			limparSaida();
			escrever(10, "Relatorios encerrados.");
		}
		else if(strcmp(cmd,"help") == 0 || strcmp(cmd,"ajuda") == 0){
			ajuda();
		}
		else if(strcmp(cmd,"fork") == 0){
			mostrarFork = !mostrarFork;
			if(mostrarFork){
				escrever(10, "Tela de simulacao do fork/wait: LIGADA");
			}
			else{
				escrever(10, "Tela de simulacao do fork/wait: DESLIGADA");
			}
		}
		else if(strcmp(cmd,"clear") == 0 || strcmp(cmd,"cls") == 0){
			escrever(10, "Tela limpa.");
		}
		else{
			escrever(12, "Comando '%s' nao reconhecido. Digite help para ver a lista.", cmd);
		}
	}

	return continuar;
}

/* ---------- menu dos relatorios ---------- */
void menuRelatorios(){
	char op;
	char entrada[120];
	int i;

	do{
		MolduraMenu();
		Titulo();
		Moldura(25,7,95,27,8,0);

		gotoxy(52,9);
		textcolor(15);
		printf("RELATORIOS");

		textcolor(7);
		gotoxy(29,12); printf("[1] Blocos ocupados por um arquivo");
		gotoxy(29,14); printf("[2] Maior arquivo que ainda pode ser criado");
		gotoxy(29,16); printf("[3] Arquivos integros x corrompidos (badblocks)");
		gotoxy(29,18); printf("[4] Blocos perdidos do disco");
		gotoxy(29,20); printf("[5] Mapa de todos os blocos (figura A)");
		gotoxy(29,22); printf("[6] Arquivos e diretorios alocados (explorer)");
		gotoxy(29,24); printf("[7] Arvore de diretorios e inodes (figura D)");
		gotoxy(29,25); printf("[8] Links simbolicos e fisicos criados");

		gotoxy(29,26);
		textcolor(12);
		printf("[ESC] Voltar ao prompt");

		op = getch();

		limparSaida();

		switch(op){
			case '1':
				MolduraMenu();
				Titulo();
				Moldura(20,12,100,18,8,0);
				gotoxy(23,15);
				textcolor(15);
				printf("Nome (ou caminho) do arquivo: ");
				textcolor(14);
				if(fgets(entrada, sizeof(entrada), stdin) != NULL){
					i = strlen(entrada);
					while(i > 0 && (entrada[i-1] == '\n' || entrada[i-1] == '\r')){
						entrada[--i] = '\0';
					}
					relBlocosArquivo(entrada);
					mostrarSaida("RELATORIO 1");
				}
				break;
			case '2':
				relMaiorArquivo();
				mostrarSaida("RELATORIO 2");
				break;
			case '3':
				relIntegridade();
				mostrarSaida("RELATORIO 3");
				break;
			case '4':
				relPerdidos();
				mostrarSaida("RELATORIO 4");
				break;
			case '5':
				telaBlocos();
				break;
			case '6':
				relExplorer();
				mostrarSaida("RELATORIO 6");
				break;
			case '7':
				relArvoreInodes();
				mostrarSaida("RELATORIO 7");
				break;
			case '8':
				relLinks();
				mostrarSaida("RELATORIO 8");
				break;
		}

	}while(op != 27);
}

/* ---------- laco do prompt (processo pai) ---------- */
void prompt(){
	char linha[160], caminho[MAX_CAMINHO], titulo[200];
	int continuar = 1, i, pidFilho;

	inicializarPilhaProc(processos);

	while(continuar){
		caminhoDe(atual, caminho);
		telaPrompt(caminho);

		if(fgets(linha, sizeof(linha), stdin) == NULL){
			continuar = 0;
		}
		else{
			i = strlen(linha);
			while(i > 0 && (linha[i-1] == '\n' || linha[i-1] == '\r')){
				linha[--i] = '\0';
			}

			/* linha vazia: apenas mostra o prompt de novo */
			if(linha[0] != '\0'){
				/* o prompt executa fork() e fica bloqueado em wait() */
				pidFilho = proxPID++;
				forkProcesso(processos, pidFilho, 1, linha);

				if(mostrarFork){
					telaProcessos(processos,
								  "fork() -> processo filho criado. O pai (prompt) executou wait().",
								  linha, 11);
					Sleep(900);
				}

				/* o processo filho simula a execucao do comando */
				limparSaida();
				continuar = executarComando(linha);

				if(continuar){
					sprintf(titulo, "PID %d executou: %.80s", pidFilho, linha);
					mostrarSaida(titulo);
				}

				/* exit() do filho -> o pai sai do wait e o prompt e liberado */
				waitProcesso(processos);

				if(continuar && mostrarFork){
					telaProcessos(processos,
								  "exit() do filho -> pai desbloqueado, prompt liberado para novo comando.",
								  linha, 10);
					Sleep(800);
				}
			}
		}
	}
}

/* ---------- inicializacao do disco ---------- */
void iniciarSimulador(){
	char entrada[40];
	int qtd;
	char op;

	MolduraMenu();
	Titulo();
	Moldura(25,10,95,18,8,0);

	gotoxy(28,13);
	textcolor(15);
	printf("Quantidade de blocos do disco (ENTER = 1000): ");
	textcolor(14);

	if(fgets(entrada, sizeof(entrada), stdin) != NULL){
		qtd = atoi(entrada);
		if(qtd <= 0){
			qtd = 1000;
		}

		inicializarDisco(qtd);

		gotoxy(28,16);
		textcolor(10);
		printf("Disco criado: %d blocos de %d bytes = %d bytes.",
			   totalBlocos, TAM_BLOCO, totalBlocos*TAM_BLOCO);
		Sleep(1200);

		if(totalBlocos >= 80){
			MolduraMenu();
			Titulo();
			Moldura(25,10,95,18,8,0);
			gotoxy(28,13);
			textcolor(15);
			printf("Carregar o disco de exemplo do enunciado (figuras A/B/D)?");
			gotoxy(28,15);
			textcolor(10);
			printf("[S] Sim   ");
			textcolor(12);
			printf("[N] Nao (disco vazio)");

			op = getch();
			if(op == 's' || op == 'S'){
				carregarExemplo();
				telaEvento("Disco de exemplo carregado na raiz /.",
						   "Use ls, ls -l e o menu rel para conferir.");
			}
		}

		prompt();
	}
}
/* ---------- menu principal ---------- */
void Menu(void){
	char op;

	MolduraMenu();

	gotoxy(46,10);
	textcolor(15);
	printf("TRABALHO SO ");
	textcolor(8);
	printf("[BIMESTRE 1]");

	gotoxy(38,7);
	textcolor(3);
	printf("SIMULACAO DO SISTEMA DE ARQUIVOS COM INODE");

	Moldura(30,12,90,22,8,0);

	gotoxy(37,14);
	textcolor(7);
	printf("Gabriel Mendes Lopes - ");
	textcolor(15);
	printf("RA: ");
	textcolor(9);
	printf("262412810");
	
	gotoxy(37,16);
	textcolor(7);
	printf("Igor Marcelino Santos - ");
	textcolor(15);
	printf("RA: ");
	textcolor(9);
	printf("262412985");

	gotoxy(37,18);
	textcolor(7);
	printf("Thiago Marques Cristino - ");
	textcolor(15);
	printf("RA: ");
	textcolor(9);
	printf("262412837");

	gotoxy(37,20);
	textcolor(7);
	printf("Guilherme Sales Pereira - ");
	textcolor(15);
	printf("RA: ");
	textcolor(9);
	printf("262411873");

	getch();

	do{
		MolduraMenu();
		Titulo();

		Moldura(30,10,90,22,8,0);

		gotoxy(56,12);
		textcolor(15);
		printf("MENU");

		gotoxy(43,15);
		textcolor(10);
		printf("[ENTER] ");
		textcolor(7);
		printf("INICIAR SIMULACAO");

		gotoxy(43,17);
		textcolor(12);
		printf("[ESC] ");
		textcolor(7);
		printf("SAIR");

		gotoxy(43,20);
		textcolor(14);
		printf("Tecle a opcao: ");

		op = getch();

		if(op != 27){
			srand(time(NULL));
			iniciarSimulador();
		}

	}while(op != 27);
}

int main(){
	Menu();
	return 0;
}
