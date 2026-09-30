/*
  1. EXPRESSOES REGULARES (Analise Lexica)
  ----------------------------------------------------------------------------
  Identificadores (TOKEN_ID)   : [a-zA-Z_][a-zA-Z0-9_]*
  Numeros Inteiros             : [0-9]+
  Numeros Reais                : [0-9]+\.[0-9]+
  Strings (Cadeias)            : "[^"]*"
  Atribuicao                   : <-
  Operadores Relacionais       : < | <= | = | > | >= | <>
  Operadores Aritmeticos       : \+ | - | \* | / | \\
  Palavras Reservadas          : algoritmo, var, inicio, fimalgoritmo, escreva,
                                 leia, se, entao, senao, fimse, para, enquanto,
                                 procedimento, funcao, retorne, vetor, etc.
  Comentarios (Ignorados)      : //.*
  
  
  2. GRAMATICA LIVRE DE CONTEXTO - GLC (Analise Sintatica - Notacao EBNF)
  ----------------------------------------------------------------------------
  Programa ::= 'algoritmo' STRING { Rotina } BlocoVariaveis 'inicio' ListaComandos 'fimalgoritmo'
  
  Rotina ::= Procedimento | Funcao
  Procedimento ::= 'procedimento' TOKEN_ID [ '(' ListaParametros ')' ] 'inicio' ListaComandos 'fimprocedimento'
  Funcao ::= 'funcao' TOKEN_ID [ '(' ListaParametros ')' ] ':' Tipo 'inicio' ListaComandos 'fimfuncao'
  
  ListaParametros ::= Parametro { ',' Parametro }
  Parametro ::= TOKEN_ID ':' Tipo
  
  BlocoVariaveis ::= [ 'var' { DeclaracaoVariavel } ]
  DeclaracaoVariavel ::= ListaIDs ':' TipoDeclaracao
  ListaIDs ::= TOKEN_ID { ',' TOKEN_ID }
  TipoDeclaracao ::= Tipo | 'vetor' '[' TOKEN_NUM_INT '..' TOKEN_NUM_INT ']' 'de' Tipo
  Tipo ::= 'inteiro' | 'real' | 'caractere' | 'logico'
  
  ListaComandos ::= { Comando }
  Comando ::= CmdAtribuicaoOuChamada
            | CmdEscreva
            | CmdLeia
            | CmdSe
            | CmdPara
            | CmdEnquanto
            | CmdRetorne
  
  CmdAtribuicaoOuChamada ::= TOKEN_ID ( CmdChamada | CmdAcessoVetorAtribuicao )
  CmdChamada ::= '(' [ ListaExpressoes ] ')'
  CmdAcessoVetorAtribuicao ::= [ '[' Expressao ']' ] '<-' Expressao
  
  CmdEscreva ::= ( 'escreva' | 'escreval' ) '(' ListaExpressoes ')'
  CmdLeia ::= 'leia' '(' TOKEN_ID [ '[' Expressao ']' ] ')'
  CmdSe ::= 'se' [ '(' ] Expressao [ ')' ] 'entao' ListaComandos [ 'senao' ListaComandos ] 'fimse'
  CmdPara ::= 'para' TOKEN_ID 'de' Expressao 'ate' Expressao [ 'passo' Expressao ] 'faca' ListaComandos 'fimpara'
  CmdEnquanto ::= 'enquanto' [ '(' ] Expressao [ ')' ] 'faca' ListaComandos 'fimenquanto'
  CmdRetorne ::= 'retorne' Expressao
  
  ListaExpressoes ::= Expressao { ',' Expressao }
  Expressao ::= ExpressaoSimples { OpLogicoOuRelacional ExpressaoSimples }
  ExpressaoSimples ::= Termo { OpAritmetico Termo }
  Termo ::= TOKEN_ID [ '[' Expressao ']' | '(' [ ListaExpressoes ] ')' ]
          | TOKEN_NUM_INT
          | TOKEN_NUM_FLOAT
          | STRING
          | 'verdadeiro'
          | 'falso'
          | 'NAO' Termo
          | '-' Termo
          | '(' Expressao ')'
  
  OpLogicoOuRelacional ::= TOKEN_OP_REL | 'E' | 'OU' | 'MOD'
  OpAritmetico ::= '+' | '-' | '*' | '/' | '\'
  ============================================================================
 */
//Gabriel Tortolio Fonseca - 10416751
//github.com/migikai15/PROJETO-Fase-1-An-lise-L-xica-e-An-lise-Sint-tica
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAM_LEXEMA 256
#define TAM_ARQUIVO_SAIDA 256

typedef enum {
    OP_LT,
    OP_LE,
    OP_EQ,
    OP_GT,
    OP_GE,
    OP_NE
} OpRelAtributo;

typedef enum {
    TOKEN_EOF,
    TOKEN_ID,
    TOKEN_NUM_INT,
    TOKEN_NUM_FLOAT,
    TOKEN_OP_REL,
    TOKEN_KEYWORD,
    TOKEN_STRING,
    TOKEN_ASSIGN,
    TOKEN_ARITH,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_COLON,
    TOKEN_COMMA,
    TOKEN_RANGE,
    TOKEN_INVALID
} TokenNome;

typedef struct attribute {
    TokenNome nome;
    int linha;
    char lexema[TAM_LEXEMA];
    union {
        int table_index;
        int int_value;
        float float_value;
        OpRelAtributo op_code;
    };
} Token;

static FILE *arquivo_fonte;
static FILE *arquivo_saida;
static int linha_atual = 1;
static int indice_id = 1;
static Token lookahead;

static void finalizar_com_erro(void)
{
    if (arquivo_saida != NULL) {
        fclose(arquivo_saida);
        arquivo_saida = NULL;
    }
    if (arquivo_fonte != NULL) {
        fclose(arquivo_fonte);
        arquivo_fonte = NULL;
    }
    exit(EXIT_FAILURE);
}

static const char *nome_token(TokenNome nome)
{
    switch (nome) {
    case TOKEN_EOF: return "TOKEN_EOF";
    case TOKEN_ID: return "TOKEN_ID";
    case TOKEN_NUM_INT: return "TOKEN_NUM_INT";
    case TOKEN_NUM_FLOAT: return "TOKEN_NUM_FLOAT";
    case TOKEN_OP_REL: return "TOKEN_OP_REL";
    case TOKEN_KEYWORD: return "TOKEN_KEYWORD";
    case TOKEN_STRING: return "TOKEN_STRING";
    case TOKEN_ASSIGN: return "TOKEN_ASSIGN";
    case TOKEN_ARITH: return "TOKEN_ARITH";
    case TOKEN_LPAREN: return "TOKEN_LPAREN";
    case TOKEN_RPAREN: return "TOKEN_RPAREN";
    case TOKEN_LBRACKET: return "TOKEN_LBRACKET";
    case TOKEN_RBRACKET: return "TOKEN_RBRACKET";
    case TOKEN_COLON: return "TOKEN_COLON";
    case TOKEN_COMMA: return "TOKEN_COMMA";
    case TOKEN_RANGE: return "TOKEN_RANGE";
    case TOKEN_INVALID: return "TOKEN_INVALID";
    }
    return "TOKEN_INVALID";
}

static const char *nome_op_rel(OpRelAtributo op)
{
    switch (op) {
    case OP_LT: return "OP_LT";
    case OP_LE: return "OP_LE";
    case OP_EQ: return "OP_EQ";
    case OP_GT: return "OP_GT";
    case OP_GE: return "OP_GE";
    case OP_NE: return "OP_NE";
    }
    return "OP_REL";
}

static void erro_lexico(const char *sequencia, int linha)
{
    printf("ERRO LÉXICO na linha %d: %s\n", linha, sequencia);
    finalizar_com_erro();
}

static void erro_sintatico(const char *esperado)
{
    printf("ERRO SINTÁTICO na linha %d: token incorreto '%s' (%s); esperado %s\n",
           lookahead.linha, lookahead.lexema, nome_token(lookahead.nome), esperado);
    finalizar_com_erro();
}

static void registrar_token(const Token *token)
{
    char atributo[TAM_LEXEMA];

    switch (token->nome) {
    case TOKEN_ID:
        snprintf(atributo, sizeof(atributo), "%d", token->table_index);
        break;
    case TOKEN_NUM_INT:
        snprintf(atributo, sizeof(atributo), "%d", token->int_value);
        break;
    case TOKEN_NUM_FLOAT:
        snprintf(atributo, sizeof(atributo), "%g", token->float_value);
        break;
    case TOKEN_OP_REL:
        snprintf(atributo, sizeof(atributo), "%s", nome_op_rel(token->op_code));
        break;
    case TOKEN_EOF:
        snprintf(atributo, sizeof(atributo), "EOF");
        break;
    default:
        snprintf(atributo, sizeof(atributo), "%s", token->lexema);
        break;
    }

    printf("%d# %s | %s\n", token->linha, nome_token(token->nome), atributo);
    if (arquivo_saida != NULL) {
        fprintf(arquivo_saida, "%d# %s | %s\n",
                token->linha, nome_token(token->nome), atributo);
    }
}

static int eh_palavra_reservada(const char *lexema)
{
    static const char *const palavras[] = {
        "algoritmo", "var", "inicio", "fimalgoritmo", "escreval", "escreva",
        "leia", "se", "entao", "senao", "fimse", "para", "de", "ate", "passo",
        "faca", "fimpara", "enquanto", "fimenquanto", "vetor", "procedimento",
        "fimprocedimento", "funcao", "fimfuncao", "retorne", "inteiro", "real",
        "caractere", "logico", "verdadeiro", "falso", "E", "OU", "NAO", "MOD"
    };
    size_t i;
    for (i = 0; i < sizeof(palavras) / sizeof(palavras[0]); ++i) {
        if (strcmp(lexema, palavras[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

static Token novo_token(TokenNome nome, int linha, const char *lexema)
{
    Token token;
    memset(&token, 0, sizeof(token));
    token.nome = nome;
    token.linha = linha;
    strncpy(token.lexema, lexema, TAM_LEXEMA - 1);
    token.lexema[TAM_LEXEMA - 1] = '\0';
    return token;
}

static Token obterToken(void)
{
    int c;
    Token token;
    char lexema[TAM_LEXEMA];
    size_t tamanho = 0;
    int linha_token;

    for (;;) {
        c = fgetc(arquivo_fonte);
        if (c == EOF) {
            token = novo_token(TOKEN_EOF, linha_atual, "EOF");
            registrar_token(&token);
            return token;
        }
        if (isspace((unsigned char)c)) {
            if (c == '\n') {
                ++linha_atual;
            }
            continue;
        }
        if (c == '/') {
            int proximo = fgetc(arquivo_fonte);
            if (proximo == '/') {
                while ((c = fgetc(arquivo_fonte)) != EOF && c != '\n') {
                }
                if (c == '\n') {
                    ++linha_atual;
                }
                continue;
            }
            if (proximo != EOF) {
                ungetc(proximo, arquivo_fonte);
            }
        }
        break;
    }

    linha_token = linha_atual;

    if (c == '"' || c == '\'') {
        int delimitador = c;
        lexema[tamanho++] = (char)c;
        while ((c = fgetc(arquivo_fonte)) != EOF && c != delimitador && c != '\n') {
            if (tamanho + 1 >= sizeof(lexema)) {
                erro_lexico("literal muito longo", linha_token);
            }
            lexema[tamanho++] = (char)c;
        }
        if (c != delimitador) {
            lexema[tamanho] = '\0';
            erro_lexico(lexema, linha_token);
        }
        lexema[tamanho++] = (char)delimitador;
        lexema[tamanho] = '\0';
        token = novo_token(TOKEN_STRING, linha_token, lexema);
        registrar_token(&token);
        return token;
    }

    if (isalpha((unsigned char)c) || c == '_') {
        lexema[tamanho++] = (char)c;
        while ((c = fgetc(arquivo_fonte)) != EOF &&
               (isalnum((unsigned char)c) || c == '_')) {
            if (tamanho + 1 >= sizeof(lexema)) {
                erro_lexico("identificador muito longo", linha_token);
            }
            lexema[tamanho++] = (char)c;
        }
        if (c != EOF) {
            ungetc(c, arquivo_fonte);
        }
        lexema[tamanho] = '\0';
        if (eh_palavra_reservada(lexema)) {
            token = novo_token(TOKEN_KEYWORD, linha_token, lexema);
        } else {
            token = novo_token(TOKEN_ID, linha_token, lexema);
            token.table_index = indice_id++;
        }
        registrar_token(&token);
        return token;
    }

    if (isdigit((unsigned char)c)) {
        int tem_ponto = 0;
        lexema[tamanho++] = (char)c;
        while ((c = fgetc(arquivo_fonte)) != EOF && isdigit((unsigned char)c)) {
            if (tamanho + 1 >= sizeof(lexema)) {
                erro_lexico("número muito longo", linha_token);
            }
            lexema[tamanho++] = (char)c;
        }
        if (c == '.') {
            int seguinte = fgetc(arquivo_fonte);
            if (seguinte == '.') {
                ungetc(seguinte, arquivo_fonte);
                ungetc(c, arquivo_fonte);
            } else {
                tem_ponto = 1;
                lexema[tamanho++] = '.';
                if (seguinte != EOF) {
                    ungetc(seguinte, arquivo_fonte);
                }
                while ((c = fgetc(arquivo_fonte)) != EOF && isdigit((unsigned char)c)) {
                    if (tamanho + 1 >= sizeof(lexema)) {
                        erro_lexico("número muito longo", linha_token);
                    }
                    lexema[tamanho++] = (char)c;
                }
                if (c != EOF) {
                    ungetc(c, arquivo_fonte);
                }
            }
        } else if (c != EOF) {
            ungetc(c, arquivo_fonte);
        }
        lexema[tamanho] = '\0';
        token = novo_token(tem_ponto ? TOKEN_NUM_FLOAT : TOKEN_NUM_INT,
                           linha_token, lexema);
        if (tem_ponto) {
            token.float_value = (float)strtod(lexema, NULL);
        } else {
            token.int_value = (int)strtol(lexema, NULL, 10);
        }
        registrar_token(&token);
        return token;
    }

    lexema[0] = (char)c;
    lexema[1] = '\0';
    if (c == '<') {
        int seguinte = fgetc(arquivo_fonte);
        if (seguinte == '-' || seguinte == '=' || seguinte == '>') {
            lexema[1] = (char)seguinte;
            lexema[2] = '\0';
        } else if (seguinte != EOF) {
            ungetc(seguinte, arquivo_fonte);
        }
        if (lexema[1] == '-') {
            token = novo_token(TOKEN_ASSIGN, linha_token, lexema);
        } else {
            token = novo_token(TOKEN_OP_REL, linha_token, lexema);
            token.op_code = (lexema[1] == '=') ? OP_LE :
                            (lexema[1] == '>') ? OP_NE : OP_LT;
        }
    } else if (c == '>') {
        int seguinte = fgetc(arquivo_fonte);
        if (seguinte == '=') {
            lexema[1] = '=';
            lexema[2] = '\0';
        } else if (seguinte != EOF) {
            ungetc(seguinte, arquivo_fonte);
        }
        token = novo_token(TOKEN_OP_REL, linha_token, lexema);
        token.op_code = (lexema[1] == '=') ? OP_GE : OP_GT;
    } else if (c == '=') {
        token = novo_token(TOKEN_OP_REL, linha_token, lexema);
        token.op_code = OP_EQ;
    } else if (strchr("+-*/\\", c) != NULL) {
        token = novo_token(TOKEN_ARITH, linha_token, lexema);
    } else if (c == '(') {
        token = novo_token(TOKEN_LPAREN, linha_token, lexema);
    } else if (c == ')') {
        token = novo_token(TOKEN_RPAREN, linha_token, lexema);
    } else if (c == '[') {
        token = novo_token(TOKEN_LBRACKET, linha_token, lexema);
    } else if (c == ']') {
        token = novo_token(TOKEN_RBRACKET, linha_token, lexema);
    } else if (c == ':') {
        token = novo_token(TOKEN_COLON, linha_token, lexema);
    } else if (c == ',') {
        token = novo_token(TOKEN_COMMA, linha_token, lexema);
    } else if (c == '.') {
        int seguinte = fgetc(arquivo_fonte);
        if (seguinte != '.') {
            if (seguinte != EOF) ungetc(seguinte, arquivo_fonte);
            erro_lexico(lexema, linha_token);
        }
        strcpy(lexema, "..");
        token = novo_token(TOKEN_RANGE, linha_token, lexema);
    } else {
        erro_lexico(lexema, linha_token);
    }

    registrar_token(&token);
    return token;
}

static Token nextToken(void)
{
    return obterToken();
}

static int eh_palavra(const char *palavra)
{
    return lookahead.nome == TOKEN_KEYWORD && strcmp(lookahead.lexema, palavra) == 0;
}

static int eh_um_dos(const char *a, const char *b, const char *c)
{
    return eh_palavra(a) || eh_palavra(b) || eh_palavra(c);
}

static void consome(TokenNome esperado)
{
    if (lookahead.nome != esperado) {
        erro_sintatico(nome_token(esperado));
    }
    lookahead = nextToken();
}

static void consome_palavra(const char *palavra)
{
    if (!eh_palavra(palavra)) {
        erro_sintatico(palavra);
    }
    lookahead = nextToken();
}

static void consome_id(void)
{
    consome(TOKEN_ID);
}

static int inicio_expressao(void)
{
    return lookahead.nome == TOKEN_ID || lookahead.nome == TOKEN_NUM_INT ||
           lookahead.nome == TOKEN_NUM_FLOAT || lookahead.nome == TOKEN_STRING ||
           lookahead.nome == TOKEN_LPAREN || lookahead.nome == TOKEN_ARITH ||
           eh_um_dos("verdadeiro", "falso", "NAO");
}

static void analisa_expressao(void);
static void analisa_comando(void);
static void analisa_comandos_ate(const char *fim1, const char *fim2);

static void analisa_primaria(void)
{
    if (lookahead.nome == TOKEN_ID) {
        consome_id();
        if (lookahead.nome == TOKEN_LBRACKET) {
            consome(TOKEN_LBRACKET);
            analisa_expressao();
            consome(TOKEN_RBRACKET);
        } else if (lookahead.nome == TOKEN_LPAREN) {
            consome(TOKEN_LPAREN);
            if (lookahead.nome != TOKEN_RPAREN) {
                analisa_expressao();
                while (lookahead.nome == TOKEN_COMMA) {
                    consome(TOKEN_COMMA);
                    analisa_expressao();
                }
            }
            consome(TOKEN_RPAREN);
        }
    } else if (lookahead.nome == TOKEN_NUM_INT || lookahead.nome == TOKEN_NUM_FLOAT ||
               lookahead.nome == TOKEN_STRING || eh_um_dos("verdadeiro", "falso", "NAO")) {
        if (eh_palavra("NAO")) {
            consome_palavra("NAO");
            analisa_primaria();
        } else {
            lookahead = nextToken();
        }
    } else if (lookahead.nome == TOKEN_LPAREN) {
        consome(TOKEN_LPAREN);
        analisa_expressao();
        consome(TOKEN_RPAREN);
    } else if (lookahead.nome == TOKEN_ARITH && strcmp(lookahead.lexema, "-") == 0) {
        consome(TOKEN_ARITH);
        analisa_primaria();
    } else {
        erro_sintatico("início de expressão");
    }
}

static void analisa_expressao(void)
{
    analisa_primaria();
    while (lookahead.nome == TOKEN_ARITH || lookahead.nome == TOKEN_OP_REL ||
           eh_um_dos("E", "OU", "MOD")) {
        lookahead = nextToken();
        analisa_primaria();
    }
}

static void analisa_lista_expressoes(void)
{
    analisa_expressao();
    while (lookahead.nome == TOKEN_COMMA) {
        consome(TOKEN_COMMA);
        analisa_expressao();
    }
}

static void analisa_declaracao(void)
{
    consome_id();
    while (lookahead.nome == TOKEN_COMMA) {
        consome(TOKEN_COMMA);
        consome_id();
    }
    consome(TOKEN_COLON);
    if (eh_palavra("vetor")) {
        consome_palavra("vetor");
        consome(TOKEN_LBRACKET);
        consome(TOKEN_NUM_INT);
        consome(TOKEN_RANGE);
        consome(TOKEN_NUM_INT);
        consome(TOKEN_RBRACKET);
        consome_palavra("de");
    }
    if (!eh_um_dos("inteiro", "real", "caractere") && !eh_palavra("logico")) {
        erro_sintatico("tipo inteiro, real, caractere ou logico");
    }
    lookahead = nextToken();
}

static void analisa_bloco_var(void)
{
    if (!eh_palavra("var")) {
        return;
    }
    consome_palavra("var");
    while (lookahead.nome == TOKEN_ID) {
        analisa_declaracao();
    }
}

static void analisa_chamada_ou_atribuicao(void)
{
    consome_id();
    if (lookahead.nome == TOKEN_LBRACKET) {
        consome(TOKEN_LBRACKET);
        analisa_expressao();
        consome(TOKEN_RBRACKET);
    }
    if (lookahead.nome == TOKEN_ASSIGN) {
        consome(TOKEN_ASSIGN);
        analisa_expressao();
    } else if (lookahead.nome == TOKEN_LPAREN) {
        consome(TOKEN_LPAREN);
        if (lookahead.nome != TOKEN_RPAREN) {
            analisa_lista_expressoes();
        }
        consome(TOKEN_RPAREN);
    }
}

static void analisa_comando_se(void)
{
    consome_palavra("se");
    if (lookahead.nome == TOKEN_LPAREN) consome(TOKEN_LPAREN);
    analisa_expressao();
    if (lookahead.nome == TOKEN_RPAREN) consome(TOKEN_RPAREN);
    consome_palavra("entao");
    analisa_comandos_ate("senao", "fimse");
    if (eh_palavra("senao")) {
        consome_palavra("senao");
        analisa_comandos_ate("fimse", NULL);
    }
    consome_palavra("fimse");
}

static void analisa_comando_para(void)
{
    consome_palavra("para");
    consome_id();
    consome_palavra("de");
    analisa_expressao();
    consome_palavra("ate");
    analisa_expressao();
    if (eh_palavra("passo")) {
        consome_palavra("passo");
        analisa_expressao();
    }
    consome_palavra("faca");
    analisa_comandos_ate("fimpara", NULL);
    consome_palavra("fimpara");
}

static void analisa_comando_enquanto(void)
{
    consome_palavra("enquanto");
    if (lookahead.nome == TOKEN_LPAREN) consome(TOKEN_LPAREN);
    analisa_expressao();
    if (lookahead.nome == TOKEN_RPAREN) consome(TOKEN_RPAREN);
    consome_palavra("faca");
    analisa_comandos_ate("fimenquanto", NULL);
    consome_palavra("fimenquanto");
}

static void analisa_comando(void)
{
    if (eh_palavra("escreva") || eh_palavra("escreval")) {
        lookahead = nextToken();
        consome(TOKEN_LPAREN);
        if (lookahead.nome != TOKEN_RPAREN) analisa_lista_expressoes();
        consome(TOKEN_RPAREN);
    } else if (eh_palavra("leia")) {
        consome_palavra("leia");
        consome(TOKEN_LPAREN);
        if (lookahead.nome == TOKEN_ID) {
            consome_id();
            if (lookahead.nome == TOKEN_LBRACKET) {
                consome(TOKEN_LBRACKET);
                analisa_expressao();
                consome(TOKEN_RBRACKET);
            }
        } else {
            erro_sintatico("identificador em leia");
        }
        consome(TOKEN_RPAREN);
    } else if (eh_palavra("se")) {
        analisa_comando_se();
    } else if (eh_palavra("para")) {
        analisa_comando_para();
    } else if (eh_palavra("enquanto")) {
        analisa_comando_enquanto();
    } else if (eh_palavra("retorne")) {
        consome_palavra("retorne");
        if (inicio_expressao()) analisa_expressao();
    } else if (lookahead.nome == TOKEN_ID) {
        analisa_chamada_ou_atribuicao();
    } else {
        erro_sintatico("comando MiniVisualg");
    }
}

static int eh_fim_de_comandos(const char *fim1, const char *fim2)
{
    if (fim1 != NULL && eh_palavra(fim1)) return 1;
    if (fim2 != NULL && eh_palavra(fim2)) return 1;
    return 0;
}

static void analisa_comandos_ate(const char *fim1, const char *fim2)
{
    while (!eh_fim_de_comandos(fim1, fim2)) {
        if (lookahead.nome == TOKEN_EOF) {
            erro_sintatico(fim1 != NULL ? fim1 : "fim do bloco");
        }
        analisa_comando();
    }
}

static void analisa_parametros(void)
{
    if (lookahead.nome == TOKEN_RPAREN) return;
    consome_id();
    consome(TOKEN_COLON);
    if (!eh_um_dos("inteiro", "real", "caractere") && !eh_palavra("logico")) {
        erro_sintatico("tipo de parâmetro");
    }
    lookahead = nextToken();
    while (lookahead.nome == TOKEN_COMMA) {
        consome(TOKEN_COMMA);
        consome_id();
        consome(TOKEN_COLON);
        if (!eh_um_dos("inteiro", "real", "caractere") && !eh_palavra("logico")) {
            erro_sintatico("tipo de parâmetro");
        }
        lookahead = nextToken();
    }
}

static void analisa_procedimento(void)
{
    consome_palavra("procedimento");
    consome_id();
    if (lookahead.nome == TOKEN_LPAREN) {
        consome(TOKEN_LPAREN);
        analisa_parametros();
        consome(TOKEN_RPAREN);
    }
    consome_palavra("inicio");
    analisa_comandos_ate("fimprocedimento", NULL);
    consome_palavra("fimprocedimento");
}

static void analisa_funcao(void)
{
    consome_palavra("funcao");
    consome_id();
    if (lookahead.nome == TOKEN_LPAREN) {
        consome(TOKEN_LPAREN);
        analisa_parametros();
        consome(TOKEN_RPAREN);
    }
    consome(TOKEN_COLON);
    if (!eh_um_dos("inteiro", "real", "caractere") && !eh_palavra("logico")) {
        erro_sintatico("tipo de retorno");
    }
    lookahead = nextToken();
    consome_palavra("inicio");
    analisa_comandos_ate("fimfuncao", NULL);
    consome_palavra("fimfuncao");
}

static void analisa_algoritmo(void)
{
    consome_palavra("algoritmo");
    if (lookahead.nome != TOKEN_STRING) {
        erro_sintatico("string com nome do algoritmo");
    }
    consome(TOKEN_STRING);

    while (eh_palavra("procedimento") || eh_palavra("funcao")) {
        if (eh_palavra("procedimento")) analisa_procedimento();
        else analisa_funcao();
    }
    analisa_bloco_var();
    consome_palavra("inicio");
    analisa_comandos_ate("fimalgoritmo", NULL);
    consome_palavra("fimalgoritmo");
}

int main(int argc, char **argv)
{
    char nome_saida[TAM_ARQUIVO_SAIDA];

    if (argc != 2) {
        fprintf(stderr, "Uso: %s <arquivo_fonte>\n", argv[0]);
        return EXIT_FAILURE;
    }

    arquivo_fonte = fopen(argv[1], "r");
    if (arquivo_fonte == NULL) {
        fprintf(stderr, "Erro ao abrir o arquivo fonte '%s'.\n", argv[1]);
        return EXIT_FAILURE;
    }

    snprintf(nome_saida, sizeof(nome_saida), "saida_tokens.txt");
    arquivo_saida = fopen(nome_saida, "w");
    if (arquivo_saida == NULL) {
        fprintf(stderr, "Erro ao criar '%s'.\n", nome_saida);
        fclose(arquivo_fonte);
        return EXIT_FAILURE;
    }

    lookahead = nextToken();
    analisa_algoritmo();
    if (lookahead.nome != TOKEN_EOF) {
        erro_sintatico("fim do arquivo");
    }

    printf("\nAnálise concluída com sucesso.\n");
    printf("Tokens salvos em '%s'.\n", nome_saida);
    fclose(arquivo_saida);
    fclose(arquivo_fonte);
    arquivo_saida = NULL;
    arquivo_fonte = NULL;
    return EXIT_SUCCESS;
}
