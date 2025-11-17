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
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
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


// --------------------------------------
void setup() {
  Serial.begin(9600);
  Serial.println("telcado ta ligado");
  Wire.begin();

  //oled
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Erro ao iniciar OLED"));
    while (true);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(10, 25);
  display.println("Inicializando...");
  display.display();

  // RFID
  SPI.begin();
  mfrc522.PCD_Init();


  uint8_t version = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  Serial.print("RFID VersionReg = 0x");
  Serial.println(version, HEX);

  if (version == 0x00 || version == 0xFF) {
  Serial.println("RFID: sem comunicacao (verifique pinos, Vcc, GND e CS).");
    Serial.println("RFID: comunicacao OK!");
  }
  delay(500);

  //oled
  display.clearDisplay();
  display.setCursor(0, 20);
  display.println("Aproxime o cartao");
  display.display();
  Serial.println("Sistema pronto!");
  delay(3000);
  
 
}
String lerVerificacao(){
    String senha = "";
    char tecla;
    while (true) {
      tecla = kpd.getKey();
      if (tecla) {
        if (tecla == '#') break;
        if(tecla == '*'){
          if(senha.length() > 0){
            senha.remove(senha.length() -1);
          }
          continue;
        }
        senha += tecla;
        
      }
    }
    return senha;
}
// ------------------- gravaaaeeeee -------------------
void gravar() {
  
  /*if(mfrc522.PICC_ReadCardSerial() == true){
    Serial.println("Cartao detectado!");
    Serial.print("UID: ");
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
      Serial.print(mfrc522.uid.uidByte[i], HEX);
    }
    Serial.println();
    
  }*/
  
  byte buffer[MAX_SIZE_BLOCK] = {0};
  byte bloco = 1;
  
  String senha = lerVerificacao();
  byte tamanho = senha.length();
  

  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
  
  senha.toCharArray((char*)buffer, MAX_SIZE_BLOCK);
  /*for(byte i = tamanho; i < MAX_SIZE_BLOCK; i++) {
    buffer[i] = ' ';
    Serial.print(" ");
  }*/
  

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, bloco, &key, &(mfrc522.uid)); 
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Falha na autenticacao"));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  } 

  

  status = mfrc522.MIFARE_Write(bloco, buffer, MAX_SIZE_BLOCK);
  if (status == MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() success: "));
    //Serial.println(mfrc522.GetStatusCodeName(status));
    for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF; {
   }
    for (byte i = 0; i < MAX_SIZE_BLOCK; i++) {
      Serial.print(key.keyByte[i], HEX);
      Serial.print(" ");
    }
    Serial.println();

    
      
    //oled
    display.clearDisplay();
    display.setCursor(0, 20);
    display.println("deletaduuuuu");
    Serial.print("pago");
    //Serial.println(mfrc522.GetStatusCodeName(status));
    display.display();
  }

  
  
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

void deletar(){
  
  byte buffer[MAX_SIZE_BLOCK] = {0};
  byte blocoD = 1;
  

  status = mfrc522.MIFARE_Write(blocoD, buffer, MAX_SIZE_BLOCK);
  if (status == MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() success: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
   }
    for (byte i = 0; i < MAX_SIZE_BLOCK; i++) {
      Serial.print(buffer[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();      
  }
}

// ------------------- le saporra -------------------
void leitura(){
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
  byte buffer[SIZE_BUFFER] = {};
  byte bloco = 1;
  byte blocoD = 1;
  byte tamanho = SIZE_BUFFER;

  

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, bloco, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Falha autenticacao"));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }


  //ler uid sem atribuir valor a variavel
  
  //Serial.println();


  if (mfrc522.MIFARE_Read(bloco, buffer, &tamanho) == mfrc522.MIFARE_Write(bloco, buffer, MAX_SIZE_BLOCK) || mfrc522.MIFARE_Read(bloco, buffer, &tamanho) != mfrc522.MIFARE_Write(blocoD, buffer, MAX_SIZE_BLOCK)  ) {
    Serial.print("Deubao: ");
    for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    } 
    //for (uint8_t i = 0; i < MAX_SIZE_BLOCK; i++){
      //Serial.print(buffer[i], HEX);
     // Serial.print(" ");
   // }
    Serial.println();

  } else {
    
    Serial.println("ENTRA NAO FDP");
  }
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}




char lerTecla() {
  // essa parte é só para ler a telca que foi cricada e dps 
  char tecla = kpd.getKey();
  if (tecla != NO_KEY) {
    Serial.print("Tecla: ");
    Serial.println(tecla);

    display.clearDisplay();
    display.setCursor(0, 20);
    if (tecla == 'A') display.println("lendo...");
    else if (tecla == 'B') display.println("gravando...");
    else if (tecla == 'C') display.println("pagandu...");
    display.display();
  }
  return tecla;
}


// ------------------- LOOP ----------
void loop() {

  if(!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()){ 
    return;
  }
  display.clearDisplay();
  display.setCursor(0, 20);
  display.println("A - ler");
  display.println("B  gravar");
  display.println("C - deletar");
  display.display();

  char tecla = lerTecla();
  
  if(tecla == 'A'){
    leitura();
    delay(2000);
  } else if(tecla == 'B'){
    gravar();
    delay(2000);
  } else if(tecla == 'C'){
    deletar();
    delay(2000);
  }
  
}
