#define led0 10                 // LED mais significativo (bit 3 de W) no pino digital 10
#define led1 11                 // LED bit 2 de W no pino 11
#define led2 12                 // LED bit 1 de W no pino 12
#define led3 13                 // LED menos significativo (bit 0 de W) no pino 13

#define PC *PCmem               // Macro para acessar o Program Counter (PC) via ponteiro PCmem
#define W mem[1]->p1            // Macro: registrador W está em mem[1]->p1 (campo p1 da struct inst)
#define X mem[2]->p1            // Macro: registrador X está em mem[2]->p1
#define Y mem[3]->p1            // Macro: registrador Y está em mem[3]->p1

//--- Execution Control -----+
static bool step = false;  //|  // Se true, executa passo a passo (aguarda uma tecla no serial entre instruções)
static int waitSecs = 4;   //|  // Se step == false, espera 'waitSecs' segundos entre instruções
//---------------------------+

struct inst{
  byte p3 : 4;             // Campo de 4 bits: opcode/seleção S (0..F)
  byte p2 : 4;             // Campo de 4 bits: Y (0..F)
  byte p1 : 4;             // Campo de 4 bits: X (0..F)

  inst(){                  // Construtor padrão: inicializa com valores não-zero (apenas segurança)
    p1 = 0xB;
    p2 = 0xC;
    p3 = 0xB;
  }

  inst(char ca, char cb, char cc){     // Construtor que recebe caracteres hex (ex.: 'C','6','B')
    byte cca, ccb, ccc;
    if(ca>='A' && ca<='F') cca = ca - 55; // Converte 'A'..'F' em 10..15
    else cca = ca - 48;                   // Converte '0'..'9' em 0..9
    if(cb>='A' && cb<='F') ccb = cb - 55;
    else ccb = cb - 48;
    if(cc>='A' && cc<='F') ccc = cc - 55;
    else ccc = cc - 48;
    p1 = cca & 0x0F;                      // Garante 4 bits
    p2 = ccb & 0x0F;
    p3 = ccc & 0x0F;
  }
};

byte* PCmem;              // Ponteiro para tratar o primeiro byte de mem[0] como PC (via hack de casting)
inst** mem;               // Vetor de ponteiros para instruções (memória principal)
inst* ins;                // Ponteiro para a instrução corrente (em mem[PC])

int progSize = 0;         // Última posição válida do programa (para laço de execução)

int i = 0;                // Variável global não utilizada (aparentemente legada)
String in;                // Buffer onde chegam os caracteres do Serial (todas as instruções em sequência)

void setup(){
  pinMode(led0, OUTPUT);  // Configura os pinos dos LEDs como saída
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);

  Serial.begin(9600);     // Abre a serial a 9600 bps

  mem = new inst*[100];   // Aloca espaço para até 100 instruções
  for(int u=0; u<100; u++){
    mem[u] = new inst('0', '0', '0'); // Inicializa cada posição com 000 (X=0,Y=0,S=0)
  }
  PCmem = (byte*)((void*)&(mem[0]));   // Hack: reinterpreta o endereço de mem[0] como um byte* para ser o PC
                                       // (PC é lido/escrito pelo macro PC que faz *PCmem)

  /*
   *  Exemplos de teste/manual antigo:
   *  PC = 0x04;
   *  PC = PC + 0x01;
   *  dumpReg();
   *  Serial.println(PC);
   */
  Serial.println("Insira as instrucoes para a carga do vetor:");
}

void execInst(){          // Executa UMA instrução apontada por 'ins' (mem[PC])
  X = ins->p1;            // Carrega X a partir da instrução corrente (campo p1)
  Y = ins->p2;            // Carrega Y a partir da instrução corrente (campo p2)

  switch(ins->p3){        // Decodifica S (campo p3) conforme a tabela 2025/2
    case 0x1: W = 0xF; break;                    // umL (1111)
    case 0x0: W = 0x0; break;                    // zeroL
    case 0x2: W = (X | ((~Y)&0xF)) & 0xF; break; // A+B' (AonB)
    case 0x3: W = ((~X) | (~Y)) & 0xF; break;    // A' + B' (nAonB)
    case 0x4: W = (~(X & Y)) & 0xF; break;       // (A.B)' (AeBn)
    case 0x5: W = (~Y) & 0xF; break;             // B' (nB)
    case 0x6: W = (~X) & 0xF; break;             // A' (nA)
    case 0x7: W = (((~X)&0xF) ^ ((~Y)&0xF)) & 0xF; break; // A'⊕B' (nAxnB)
    case 0x8: W = (X ^ Y) & 0xF; break;          // A⊕B (AxB)
    case 0x9: W = X; break;                      // copiaA
    case 0xA: W = Y; break;                      // copiaB
    case 0xB: W = (X & Y) & 0xF; break;          // A.B (AeB)
    case 0xC: W = (X & ((~Y)&0xF)) & 0xF; break; // A.B' (AenB)
    case 0xD: W = (((~X)&0xF) & Y) & 0xF; break; // A'.B (nAeB)
    case 0xE: W = (X | Y) & 0xF; break;          // A+B (AoB)
    case 0xF: W = (~(((~X)&0xF) & Y)) & 0xF; break; // (A'.B)' (nAeBn)
    default:
      Serial.println("Instrucao desconhecida");
      W = 0x0;
      break;
  }
}

void execProgram(){       // Laço principal de execução: PC vai de 0x04 até progSize
  PC = 0x04;              // PC inicia na posição 4 (0..3 reservados para [PC,W,X,Y] visualizados nos dumps)
  while(PC<=progSize){
    //Serial.print("PC(");             // Logs antigos comentados
    //Serial.print(PC);
    //Serial.print(") ");
    ins = mem[PC];                     // Seleciona a instrução atual (mem[PC])
    //ins = new inst();                // (linha antiga/legada)
    execInst();                        // Executa a ULA para X,Y,S da instrução

    // Zera LEDs antes de escrever novo valor
    digitalWrite(led0, LOW);
    digitalWrite(led1, LOW);
    digitalWrite(led2, LOW);
    digitalWrite(led3, LOW);

    // Acende LEDs conforme bits de W (bit 3 → led0, bit 2 → led1, bit 1 → led2, bit 0 → led3)
    if((W&0b1000)==0b1000) digitalWrite(led0, HIGH);
    if((W&0b0100)==0b0100) digitalWrite(led1, HIGH);
    if((W&0b0010)==0b0010) digitalWrite(led2, HIGH);
    if((W&0b0001)==0b0001) digitalWrite(led3, HIGH);

    PC = PC + 0x01;                    // Avança PC para a próxima instrução
    dumpMem();                         // Imprime no Serial um dump da memória

    if(step){                          // Modo passo-a-passo: espera uma tecla no Serial
      Serial.println("Step");
      while(Serial.available()==0){}   // Bloqueia até chegar um byte
      char x = Serial.read();          // Lê e descarta o byte recebido (qualquer tecla)
      delay(300);                      // Debounce simples
    }
    else delay(waitSecs * 1000);       // Modo contínuo: espera alguns segundos
  }
}

void dumpReg(){                        // Imprime os 4 primeiros “registradores” mem[0..3] no formato hex
  for(int u=0; u<4; u++){
    if(mem[u]->p1 < 10) Serial.print(mem[u]->p1);
    else Serial.print((char)(mem[u]->p1 + 55)); // 10→'A', 11→'B', ...
    if(mem[u]->p2 < 10) Serial.print(mem[u]->p2);
    else Serial.print((char)(mem[u]->p2 + 55));
    if(mem[u]->p3 < 10) Serial.print(mem[u]->p3);
    else Serial.print((char)(mem[u]->p3 + 55));
    Serial.print(" | ");
  }
  Serial.println("");
}

void dumpMem(){                        // Imprime PC, W, X, Y e depois todo o programa mem[4..99]
  Serial.print(PC);
  Serial.print(" | ");
  if(mem[1]->p1 < 10) Serial.print(mem[1]->p1);       // W
  else Serial.print((char)(mem[1]->p1 + 55));
  Serial.print(" | ");
  if(mem[2]->p1 < 10) Serial.print(mem[2]->p1);       // X
  else Serial.print((char)(mem[2]->p1 + 55));
  Serial.print(" | ");
  if(mem[3]->p1 < 10) Serial.print(mem[3]->p1);       // Y
  else Serial.print((char)(mem[3]->p1 + 55));
  Serial.print(" | ");
  for(int u=4; u<100; u++){                            // Imprime todas as instruções como trinca X Y S
    if(mem[u]->p1 < 10) Serial.print(mem[u]->p1);
    else Serial.print((char)(mem[u]->p1 + 55));
    if(mem[u]->p2 < 10) Serial.print(mem[u]->p2);
    else Serial.print((char)(mem[u]->p2 + 55));
    if(mem[u]->p3 < 10) Serial.print(mem[u]->p3);
    else Serial.print((char)(mem[u]->p3 + 55));
    Serial.print(" | ");
  }
  Serial.println("");
}

void loadMem(){                        // Carrega mem[4..] a partir da string 'in' recebida pela Serial
  int reps = in.length(),              // Quantidade total de caracteres recebidos
  progPos = 4;                         // Próxima posição de escrita no vetor mem (começa em 4)
  char a = '0',                        // Buffers para formar uma instrução (3 dígitos hex): a,b,c
  b = '0',
  c = '0';
  for(int j=0; j<reps; j++){
    if(j%4==0) a = in.charAt(j);       // Posição 0,4,8,... ← primeiro dígito (X)
    else if(j%4==1) b = in.charAt(j);  // Posição 1,5,9,... ← segundo dígito (Y)
    else if(j%4==2) c = in.charAt(j);  // Posição 2,6,10,... ← terceiro dígito (S)
    else mem[progPos++] = new inst(a, b, c); // Posição 3,7,11,... finaliza a trinca e grava mem[progPos]
  }
  progSize = progPos - 1;              // Marca a última posição válida
}

void loop(){
  while(Serial.available()>0){                     // Se há dados chegando na Serial…
    in = Serial.readStringUntil('\0');             // Lê até NUL (o Arduino IDE geralmente termina com '\n'; aqui lê tudo que chegou)

    loadMem();                                     // Converte a string recebida em instruções (mem[4..])
    execProgram();                                 // Executa o programa

    Serial.println("Insira as instrucoes para a carga do vetor:");
  }
}
