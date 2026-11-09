#include <SPI.h>
#include <Adafruit_PN532.h>

// Define the Chip Select (SS) pin connected to the PN532 SS/CS pin (D10)
#define PN532_SS (10)

// Initialize Adafruit_PN532 for Hardware SPI using only the Chip Select pin
Adafruit_PN532 nfc(PN532_SS);

void setup(void) {
  Serial.begin(9600);
  while (!Serial); // Wait for Serial Monitor to open
  
  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    Serial.print("PN53x not found. Check your SPI wiring and DIP switches!");
    while (1); // Halt execution
  }

  // Display firmware and chip details
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
  
  // Configure board to read RFID/NFC tags
  nfc.setPassiveActivationRetries(0xFF);
  nfc.SAMConfig();
  
  Serial.println("Waiting for an ISO14443A card...");
}

// Auxiliary function to display the card UID in Hex format
void printArray(byte *buffer, byte bufferSize) {
   for (byte i = 0; i < bufferSize; i++) {
      Serial.print(buffer[i] < 0x10 ? " 0" : " ");
      Serial.print(buffer[i], HEX);
   }
}

void loop(void) {
  boolean success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 }; // Buffer to store 4 or 7 byte UID
  uint8_t uidLength;

  // Poll for a tag in the RF field
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);
  
  if (success) {
    Serial.println("Card found!");
    Serial.print("UID Length: "); Serial.print(uidLength, DEC); Serial.println(" bytes");
    Serial.print("UID Value: "); printArray(uid, uidLength);
    Serial.println("\n------------------------------");
  
    delay(1000); // 1-second delay to prevent spamming reads
  }
}
