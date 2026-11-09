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
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0DEC); 
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
  
  // Configure board to read/write RFID/NFC tags
  nfc.setPassiveActivationRetries(0xFF);
  nfc.SAMConfig();
  
  Serial.println("Waiting for an ISO14443A card to write data...");
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

    // Target Block 1 (First writable data block of Sector 1)
    uint8_t blockNumber = 1; 
    
    // Default factory key A (all 0xFF)
    uint8_t keyA[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

    // Step 1: Authenticate the block using Key A
    Serial.print("Authenticating Block "); Serial.print(blockNumber); Serial.println("...");
    success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, blockNumber, 0, keyA);

    if (success) {
      Serial.println("Authentication successful!");
      
      // Prepare exactly 16 bytes to write (e.g., "Hello" padded with spaces)
      uint8_t dataToWrite[16] = {
        'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '!', ' ', ' ', ' ', ' '
      };
      
      // Step 2: Write the 16-byte data block
      success = nfc.mifareclassic_WriteDataBlock(blockNumber, dataToWrite);
      
      if (success) {
        Serial.println("SUCCESS: Data written successfully to Block 1!");
      } else {
        Serial.println("Error: Unable to write to the data block.");
      }
    } else {
      Serial.println("Authentication failed for this block.");
    }
    
    Serial.println("----------------------------------");
    delay(10000); // Wait 10 seconds so it doesn't loop-write continuously
  }
}