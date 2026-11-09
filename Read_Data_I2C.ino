
#include <Wire.h>
#include <Adafruit_PN532.h>

// I2C configuration: IRQ connected to D2, RESET to D3
#define PN532_IRQ 2
#define PN532_RESET 3

Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);

void setup() {
  Serial.begin(115200);
  while (!Serial); // Wait for Serial Monitor to open

  Serial.println("--- PN532 NFC Card Reader ---");
  
  nfc.begin();

  uint32_t version = nfc.getFirmwareVersion();
  if (!version) {
    Serial.println("PN532 not detected. Check wiring and mode.");
    while (1) {
      delay(10);
    }
  }

  // Print firmware version details
  Serial.print("PN532 firmware version: ");
  Serial.print((version >> 16) & 0xFF, HEX);
  Serial.print(".");
  Serial.println((version >> 8) & 0xFF, HEX);

  nfc.SAMConfig();
  Serial.println("Ready. Hold your NFC card or tag steady against the antenna to read...");
}

void loop() {
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the read UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes)

  // Try to read a passive target (ISO14443A card/tag) with a 1-second timeout
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 1000);

  if (success) {
    Serial.println("\n-----------------------------------");
    Serial.println("Card detected!");
    
    // Print UID
    Serial.print("UID Length: "); Serial.print(uidLength, DEC);
    Serial.print(" bytes\nUID: ");
    for (uint8_t i = 0; i < uidLength; i++) {
      if (uid[i] < 0x10) Serial.print("0");
      Serial.print(uid[i], HEX);
      if (i < uidLength - 1) Serial.print(" ");
    }
    Serial.println();

    // Check if it's a standard 4-byte MIFARE Classic card
    if (uidLength == 4) {
      uint8_t blockNumber = 1; // Block 1 (Sector 0) where we wrote the data
      
      // Standard factory default key
      uint8_t keyA[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

      Serial.print("Authenticating Block ");
      Serial.print(blockNumber);
      Serial.println(" using Factory Default Key A...");

      // Authenticate Block 1 using Key A (0)
      success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, blockNumber, 0, keyA);

      if (success) {
        Serial.println("Authentication successful!");

        // Read the 16-byte data block
        uint8_t readBuffer[16];
        success = nfc.mifareclassic_ReadDataBlock(blockNumber, readBuffer);

        if (success) {
          Serial.println("SUCCESS: Data read successfully!");
          
          // Print as Text (Characters)
          Serial.print("Text Content: ");
          for (int i = 0; i < 16; i++) {
            Serial.print((char)readBuffer[i]);
          }
          Serial.println();

          // Print as Hexadecimal bytes
          Serial.print("Hex Content:  ");
          for (int i = 0; i < 16; i++) {
            if (readBuffer[i] < 0x10) Serial.print("0");
            Serial.print(readBuffer[i], HEX);
            Serial.print(" ");
          }
          Serial.println();

        } else {
          Serial.println("ERROR: Failed to read data from block.");
        }
      } else {
        Serial.println("ERROR: Authentication failed on Block 1. Card might use custom keys.");
      }
    } else {
      Serial.println("This reader example requires a 4-byte UID card.");
    }

    Serial.println("-----------------------------------\n");
    delay(3000); // Wait 3 seconds before next read
  }
}
