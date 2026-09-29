======================================================================
PROJETO 1 - COMPILADORES: Análise Léxica e Análise Sintática
======================================================================

NOME DOS INTEGRANTES:
- Gabriel Tortolio Fonseca - 10416751

1. ESTADO DE CONCLUSÃO DO TRABALHO
O projeto foi totalmente concluído, contemplando as três etapas exigidas:
- Etapa #1: As expressões regulares e a Gramática Livre de Contexto (GLC) foram definidas e incluídas diretamente no cabeçalho do código-fonte em formato de comentário.
- Etapa #2: O analisador léxico foi implementado na íntegra. Gera um ficheiro de saída (saida_tokens.txt) contendo os tokens identificados, e imprime o mesmo resultado no ecrã no formato `<Linha># <Token> | <Atributo>`. A interrupção por erro léxico também está a funcionar.
- Etapa #3: O analisador sintático foi implementado com sucesso. Trabalha em conjunto com o léxico comunicando-se através da função nextToken(). É capaz de analisar toda a gramática do MiniVisualg e parar a execução quando encontra erros sintáticos.

2. COMO COMPILAR E EXECUTAR
Para compilar o programa (utilizando o compilador MinGW), execute o exato comando especificado no documento do projeto:

gcc -Wall -Wno-unused-result -g -Og compilador.c -o compilador

Para executar o compilador, utilize a linha de comandos passando o ficheiro fonte MiniVisualg como argumento:

./compilador <nome_do_ficheiro_fonte.alg>

Exemplos de execução:
./compilador teste1_basico.alg
./compilador teste2_controle.alg

3. DECISÕES DE DESIGN E IMPLEMENTAÇÃO
- Arquitetura do Analisador Sintático: Optou-se por construir um analisador sintático preditivo descendente recursivo (Recursive Descent Parser). Cada regra não-terminal da nossa gramática EBNF foi mapeada para uma função em C (ex: analisa_algoritmo, analisa_comando, analisa_expressao), tornando o fluxo lógico muito limpo.
- Estrutura de Dados do Token: Seguiu-se a exigência de usar um registo com um conjunto de campos de tipos diferentes. Utilizou-se a estrutura "struct attribute" contendo uma "union" para otimizar memória e armazenar eficientemente atributos exclusivos (como inteiros, números reais e sub-códigos de operadores relacionais).
- Leitura e Comentários: O salto de espaços em branco, quebras de linha e comentários iniciados por "//" é processado logo no início do ciclo de leitura de carateres (fgetc), simplificando a lógica principal de formação dos lexemas.
