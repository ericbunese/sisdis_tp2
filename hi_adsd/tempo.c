/*

 Trabalho Prático 1
 Sistemas Distribuídos
 Aluno Eric Eduardo Bunese
 Professor Elias P. Duarte Jr.

 Última edição feita em 09/04/2017

*/

#include <stdio.h>
#include <stdlib.h>
#include "smpl.h"

// Eventos
#define TEST 1
#define FAULT 2
#define REPAIR 3

// Nodo
typedef struct tnodo
{
 int id;
 int *STATE;
}tnodo;

// Vetor de nodos
tnodo* nodo;
// Informações gerais da simulação
static int N, token, event, r, i;
static char fa_name[5];
double maxTime, tempoEvento;
int nodosFalhos, contador;

// Imprime o vetor STATE de um nodo.
void imprimeNodo(int token)
{
 printf("Vetor STATE do nodo %d: [ ", token);
 for (int i=0;i<N;++i)
 {
  printf("%d", nodo[token].STATE[i]);
  if (i<N-1)
   printf(", ");
 }
 printf(" ]\n\n");
}

// Retorna o maior entre dois números inteiros
int max(int a, int b)
{
 return (a>b?a:b);
}

// Retorna o maior entre dois números double
double maxD(double a, double b)
{
 return (a>b?a:b);
}

// O nodo token1 obtém informações a partir de token2
void obtemInfo(int token1, int token2, int testes)
{
 int cont=0;
 int aviso=0;
 printf("O nodo %d obtém informações sobre os seguintes nodos, a partir do nodo %d: [", token1, token2);
 for (int i=0;i<N;++i)
 {
  // Não obtém a informação dos nodos que testou.
  if (i!=token1 && (i<token2-testes || i>token2))
  {
   if (nodo[token1].STATE[i]<nodo[token2].STATE[i])
   {
    if (++contador==N-nodosFalhos)
    {
     aviso=1;
    }
   }
   nodo[token1].STATE[i] = max(nodo[token2].STATE[i], nodo[token1].STATE[i]);
   if (cont>0)
    printf(", %d", i);
   else
    printf("%d", i);
   cont++;
  }
 }
 printf("]\n");
 if (aviso)
     printf("Evento diagnosticado.\nAtraso: %d rodadas de testes.\n", (int)((time()-tempoEvento)/30.0+1));
}

// Função que testa um nodo a partir do token do nodo atual
int testarNodo(int token1, int token2, int testes)
{
 int st = status(nodo[token2].id);

 char *c = (st==0?"SEM FALHA":"FALHO");

 if (nodo[token1].STATE[token1]==-1)
     nodo[token1].STATE[token1] = 0;

 // Verifica se houve troca de estado.
 if (nodo[token1].STATE[token2]==-1)
     nodo[token1].STATE[token2] = st;
 else
 {
  if (st!=0)
  {
   // Se o STATE estiver sem falha, atualiza para com falha.
   if (nodo[token1].STATE[token2] % 2 == 0)
       ++nodo[token1].STATE[token2];
  }
  else
  {
   // Se o STATE estiver com falha, atualiza para sem falha.
   if (nodo[token1].STATE[token2] % 2 == 1)
       ++nodo[token1].STATE[token2];
  }
 }

 printf("O nodo %d TESTOU o nodo %d como %s no tempo %5.1f\n", token1, token2, c, time());
 // Obtem informações do nodo testado
 if (st==0)
     obtemInfo(token1, token2, testes);

 imprimeNodo(token1);
 return st;
}

// Retorna uma string entre separadores.
char* tokenize(char *source,int pos,char sep)
{
 int p=0, i, c, counter=0,l=strlen(source);
 char* ret=(char*)malloc(sizeof(char)*l);
 if (!ret) return NULL;
 for (i=0;i<l;i++)
 {
  c=source[i];
  if (counter==pos)
  {
   ret[p]=c;
   p++;
  }
  if (c==sep || c==source[l])
  {
   counter++;
   if (counter>pos)
   {
    ret[p-1]='\0';
    break;
   }
   p=0;
  }
 }
 for(i=0;ret[i]!='\0';i++)
 {}
 if(realloc(ret,i)!=NULL)
    return ret;
 else return NULL;
}

// Faz o escalonamento de eventos a partir do arquivo de entrada
// Cada linha do arquivo de entrada tem a sintaxe F,token,tempo ou R,token,tempo
void escalona_eventos(char* fname)
{
 FILE* f;
 f = fopen(fname, "r");
 if (f!=NULL)
 {
  char line[15], *op, *tk, *dl;
  int token;
  double delay;
  do
  {
   if (fgets(line, 15, f)!=NULL)
   {
    op = tokenize(line, 0, ',');
    tk = tokenize(line, 1, ',');
    dl = tokenize(line, 2, ',');
    if (op!=NULL && tk!=NULL && dl!=NULL)
    {
     token = atoi(tk);
     delay = atof(dl);
     maxTime = maxD(maxTime, delay+90.0);
     if (strcmp(op, "F")==0)
     {
      printf("*Evento FAULT escalonado para o nodo %d no tempo %5.1f\n", token, delay);
      schedule(FAULT, delay, token);
     }
     else if (strcmp(op, "R")==0)
     {
      printf("*Evento REPAIR escalonado para o nodo %d no tempo %5.1f\n", token, delay);
      schedule(REPAIR, delay, token);
     }
    }
   }
  } while(!feof(f));
  fclose(f);
 }
}

// Programa Principal
int main(int argc, char * argv[])
{
 if(argc !=3)
 {
  puts("Uso correto: tempo <num-nodos> <arquivo de escalonamento>");
  exit(1);
 }

 printf("\n\n====================\nTrabalho Prático 1 de Sistemas Distribuídos\nAutor: Eric Eduardo Bunese\nProfessor: Elias P. Duarte Jr.\n\n");

 N = atoi(argv[1]);
 smpl(0, "Trabalho Prático 1 SisDis");
 reset();
 stream(1);
 nodo = (tnodo*)malloc(sizeof(tnodo)*N);

 for(i = 0; i < N; i++)
 {
  memset(fa_name,'\0', 5);
  sprintf(fa_name, "%d", i);
  nodo[i].id = facility(fa_name, 1);
  nodo[i].STATE = (int*)malloc(sizeof(int)*N);
  for (int j=0;j<N;++j)
       nodo[i].STATE[j] = -1;
 }

 // Escalonamento de eventos
 for(i = 0; i < N; i++)
     schedule(TEST, 30.0, i);

 // Faz o escalonamento de eventos a partir do arquivo de entrada.
 escalona_eventos(argv[2]);
 printf("\n====================\nInicializando Simulação\n====================\n\n");

 // Checagem de eventos
 while(time() < maxTime)
 {
  cause(&event, &token);
  switch(event)
  {
   case TEST:
     if (status(nodo[token].id) != 0) break;
     int offset = 1;
     int token2, st;

     // Testa todos os nodos até encontrar um sem falha.
     do
     {
      token2 = (token+offset)%N;
      if (token2!=token)
      {
       st = testarNodo(token, token2, offset-1);
       offset+=1;
      }
     }
     while (st!=0 && token2!=token);

     schedule(TEST, 30.0, token);
   break;

   case FAULT:
     r = request(nodo[token].id, token, 0);
     if(r != 0)
     {
      puts("Nao consegui falhar nodo");
      exit(1);
     }
     printf("O nodo %d FALHOU no tempo %5.1f\n", token, time());
     tempoEvento = time();
     nodosFalhos++;
     contador = 0;
   break;

   case REPAIR:
     release(nodo[token].id, token);
     printf("O nodo %d RECUPEROU no tempo %5.1f \n", token, time());
     schedule(TEST, 30.0, token);
     tempoEvento = time();
     nodosFalhos--;
     contador = 0;
   break;
  }
 }

 printf("\n\nEncerrando Simulação\n\n");

 return 0;
}
