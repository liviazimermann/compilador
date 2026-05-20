# Trabalho 1 - M2 - Analise Semantica  
Alunos: Bruno Petroski Enghi, Livia Zimermann  
Link Video: https://www.youtube.com/watch?v=K6BSbGHT6sc

## Execução
Abrir a pasta do projeto IDE_Compilador no QT Creator  
Após isso basta rodar o programa

### Requisito 1 - Inserir na tabela de simbolos
```
var globalVar: inteiro;
var globalVetor[10]: real;

nulo funcaoTeste(param1: inteiro, paramVetor[5]: string) {
    var varLocal: booleano;
    var vetorLocal[2]: char;
    
    varLocal = verdadeiro;
    globalVar = 10;
}
```

### Requisito 2 - Tratamento de escopos
```
nulo funcaoA() {
    var a: inteiro;
    a = 1;
}

nulo funcaoB() {
    var b: inteiro;
    
    // erro - a foi declarado em funcaoA e não existe em funcaoB - descomentar linha de baixo para ver o erro
    //b = a; 
}

var  global : inteiro;

inteiro main() {
    var x: inteiro;

    global = 10; // permite usar variavel global
    
    // Escopo interno de bloco
    se ( 1 > 2 ){
        var y: inteiro;
        y = 10;
        x = 10; // permite usar variavel do escopo de cima
    } fim
    
    // erro - y foi declarado no escopo do se - descomentar linha de baixo para ver o erro
    //x = y; 
}
```

### Requisito 3 - identificadores unicos no escopo
```
inteiro main(){ 
    var a: string;
    //var a: booleano;  // erro - a duplicado
    
    // erro - b duplicado
    //var b, b: inteiro; 
 
    se( 1 > 2 ){
        var a : inteiro; 
        a = 10;
    } fim
}
```

### Requisito 4 -  Variaveis declaradas e nao usadas
```
nulo principal() {
    var usada: inteiro;
    var naoUsada: real; 
    
    usada = 10;
    escreva(usada);
}
```

### Requisito 5 - Uso de lixo de memoria
```
nulo principal() {
    var x, y, z: inteiro;    

    z = 5;
    y = x + z;  // aviso    
    x = y;
}
```

### Requisito 7 - Compatibilidade de tipos
```
nulo principal() {
    var num: inteiro;
    var numReal: real;
    var cha: char;
    var str: string;
    var bool: booleano;

    num = 10;
    numReal = 5.2;
    str = "texto";
    cha = 'S';
    bool = verdadeiro;

    // aviso - atribuicao
    cha = str;
    num = numReal;
    
    // erro - atribuicao
    //num = str;          // erro - atrib string em inteiro
    //bool = num;    // erro - atrib inteiro em booleano
    
    // erro - expressões
    // numReal = num + str;  // erro - soma string e inteiro
    //str = str * 2;  	       // erro - multiplicando string
    
}
```
### Código Fonte Exemplo
```
var x : inteiro;
var a : real;
var b : real;
var c : real;
var i : inteiro;
var y : booleano;
var vet[5] : inteiro;
var s : string;

inteiro soma(a : inteiro, b : inteiro) {
    escreva(a + b);
}

nulo imprime() {
    escreva(42);
}

{
    x = 5;
    a = 1.5;
    b = 2.5;
    c = a + b;
    vet[0] = 10;
    vet[1] = 20;
    escreva(vet[0]);
    leia(x);
    escreva(x);
    soma(3, 5);
    imprime();
    x = 1;

    se (x == 1) {
        x = 2;
    } fim

    se (x == 0) {
        x = 1;
    } senao {
        x = 2;
    } fim
    x = 0;

    enquanto (x < 10) {
        x = x + 1;
    }

    para (i = 0; i < 10; i = i + 1) {
        escreva(i);
    }

    x = 0;
    faca {
        x = x + 1;
    } enquanto (x < 5);

    y = verdadeiro;
    se (x > 3 && y == verdadeiro) {
        escreva(x);
    } fim

    se (x == 1) {
        var z : inteiro;
        z = 2;
        se (z == 2) {
            escreva(z);
        } fim
    } fim
}
```