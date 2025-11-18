#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <MFRC522.h>
#include <SPI.h>
#include <Keypad.h>
#include <Wire.h>

// -------------------CONFIG TECLADO -------------------
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {13, 12, 14, 27};
byte colPins[COLS] = {26, 25, 33, 32};
Keypad kpd = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// ------------------- CONFIG DISPLAY -------------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ------------------- CONFIG RFID -------------------
#define SS_PIN 5
#define RST_PIN 4
#define SIZE_BUFFER 18
#define MAX_SIZE_BLOCK 16

MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;
MFRC522::StatusCode status;

// ----------------------------------------------------------------
// Função auxiliar para exibir msg
void tela(String txt1, String txt2="") {
  display.clearDisplay();
  display.setCursor(0, 20);
  display.println(txt1);
  if (txt2 != "") display.println(txt2);
  display.display();
}

// ----------------------------------------------------------------
// Lê senha do teclado
String lerVerificacao() {
  String senha = "";
  char tecla;

  tela("Digite a senha", "# para OK");

  while (true) {
    tecla = kpd.getKey();
    if (tecla) {

      // Finaliza senha
      if (tecla == '#') break;

      // Backspace
      if (tecla == '*') {
        if (senha.length() > 0) senha.remove(senha.length() - 1);
        continue;
      }

      senha += tecla;
    }
  }
  return senha;
}

// ----------------------------------------------------------------
// AUTENTICA BLOCO
bool autenticar(byte bloco) {
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  status = mfrc522.PCD_Authenticate(
    MFRC522::PICC_CMD_MF_AUTH_KEY_A,
    bloco,
    &key,
    &mfrc522.uid
  );

  return status == MFRC522::STATUS_OK;
}

// ----------------------------------------------------------------
// VALIDA CARTÃO — só passa se estiver "CADASTRADO"
bool validarCartao() {
  if (!autenticar(1)) return false;

  byte buffer[SIZE_BUFFER] = {};
  byte tam = SIZE_BUFFER;

  status = mfrc522.MIFARE_Read(1, buffer, &tam);
  if (status != MFRC522::STATUS_OK) return false;

  String dado = "";
  for (int i = 0; i < 16; i++) dado += (char)buffer[i];
  dado.trim();

  return (dado.startsWith("CADASTRADO"));
}

// ----------------------------------------------------------------
// GRAVA senha + marcação de cartão cadastrado
void gravar() {

  String senha = lerVerificacao();

  // buffer que vai pro cartão
  byte buffer[MAX_SIZE_BLOCK] = {0};
  String conteudo = "CADASTRADO:" + senha;
  conteudo.toCharArray((char*)buffer, MAX_SIZE_BLOCK);

  if (!autenticar(1)) {
    tela("ERRO", "Autenticação");
    return;
  }

  status = mfrc522.MIFARE_Write(1, buffer, MAX_SIZE_BLOCK);

  if (status == MFRC522::STATUS_OK) {
    tela("Gravado com", "sucesso!");
  } else {
    tela("Falha ao", "gravar!");
  }

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

// ----------------------------------------------------------------
// LEITURA do bloco 1
void leitura() {
  if (!autenticar(1)) {
    tela("Erro", "autenticação");
    return;
  }

  byte buffer[SIZE_BUFFER] = {};
  byte tam = SIZE_BUFFER;

  status = mfrc522.MIFARE_Read(1, buffer, &tam);

  if (status != MFRC522::STATUS_OK) {
    tela("Erro", "na leitura");
    return;
  }

  Serial.print("Conteudo: ");
  for (int i = 0; i < 16; i++) Serial.write(buffer[i]);
  Serial.println();

  tela("LIDO!", "Veja no Serial");

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

// ----------------------------------------------------------------
// DELETAR conteúdo do cartão
void deletar() {
  byte buffer[MAX_SIZE_BLOCK] = {0};

  if (!autenticar(1)) {
    tela("Erro", "autenticação");
    return;
  }

  status = mfrc522.MIFARE_Write(1, buffer, MAX_SIZE_BLOCK);

  if (status == MFRC522::STATUS_OK)
    tela("Apagado!", "");
  else
    tela("Falha ao", "apagar");

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

// ----------------------------------------------------------------
// Ler tecla pressionada e atualizar display
char lerTecla() {
  char tecla = kpd.getKey();
  if (tecla != NO_KEY) {
    Serial.print("Tecla: ");
    Serial.println(tecla);
  }
  return tecla;
}

// ----------------------------------------------------------------
// LOOP PRINCIPAL
void loop() {

  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial())
    return;

  // VALIDA cartão antes do menu!
  if (!validarCartao()) {
    tela("Cartao NAO", "autorizado");
    delay(1500);
    return;
  }

  tela("A - Ler", "B - Gravar / C - Del");

  char tecla = lerTecla();

  if (tecla == 'A') {
    leitura();
    delay(2000);
  }
  else if (tecla == 'B') {
    gravar();
    delay(2000);
  }
  else if (tecla == 'C') {
    deletar();
    delay(2000);
  }
}
