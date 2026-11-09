
#include <SPI.h>
#include <Adafruit_PN532.h>

// Define the Chip Select (SS) pin connected to the PN532 SS/CS pin (D10)
#define PN532_SS (10)

// Initialize Adafruit_PN532 for Hardware SPI using only the Chip Select pin
Adafruit_PN532 nfc(PN532_SS);

void setup(void) {
  Serial.begin(115200);
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
  
  Serial.println("Waiting for an ISO14443A card to read data...");
}

// Auxiliary function to display array in Hex format
void printHex(byte *buffer, byte bufferSize) {
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
    Serial.println("\n----------------------------------");
    Serial.println("Card found!");
    Serial.print("UID Length: "); Serial.print(uidLength, DEC); Serial.println(" bytes");
    Serial.print("UID Value: "); printHex(uid, uidLength);
    Serial.println();

    // Target Block 1 (where you wrote your data)
    uint8_t blockNumber = 1; 
    
    // Default factory key A (all 0xFF)
    uint8_t keyA[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

    // Step 1: Authenticate the block using Key A
    Serial.print("Authenticating Block "); Serial.print(blockNumber); Serial.println("...");
    success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, blockNumber, 0, keyA);

    if (success) {
      Serial.println("Authentication successful!");
      
      uint8_t data[16];
      
      // Step 2: Read the 16-byte data block
      success = nfc.mifareclassic_ReadDataBlock(blockNumber, data);
      
      if (success) {
        Serial.println("SUCCESS: Data read successfully!");
        
        // Print as text characters
        Serial.print("Text Content: ");
        for (int i = 0; i < 16; i++) {
          if (data[i] >= 32 && data[i] <= 126) {
            Serial.print((char)data[i]);
          } else {
            Serial.print('.'); // Print dot for non-printable bytes
          }
        }
        Serial.println();

        // Print as raw Hex bytes
        Serial.print("Hex Content:  ");
        printHex(data, 16);
        Serial.println();
        
      } else {
        Serial.println("Error: Unable to read the data block.");
      }
    } else {
      Serial.println("Authentication failed for this block.");
    }
    
    Serial.println("----------------------------------");
    delay(3000); // 3-second delay to prevent spamming reads
  }
}
