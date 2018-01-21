#define SERIAL_DEBUG 1

#include <OneWire.h>
#include "ds1961.h"
#include "ds1961_sha.h"
#include "EEPROMAnything.h"
#if (SERIAL_DEBUG)
  #include "hexdump.h"
#endif

static OneWire ow(2);
static DS1961 ds(&ow);

static uint8_t compareUint8_t(uint8_t mac1[], uint8_t mac2[], uint8_t length) {
  uint8_t compare = 1;
  for(uint8_t i = 0; (i < length) && compare; i++) {
    if (mac1[i] != mac2[i]) {
      compare = 0;
    }
  }
  return compare;
}

typedef struct uid_t {
    uint8_t id[8];
    uint8_t secret[8];
}; 
uint8_t uidPosition;
static initUid(struct uid_t* uid) {
  for(uint8_t i = 0; i < sizeof(uid->id); i++) {
    uid->id[i] = 0xFF;
  }
  for(uint8_t i = 0; i < sizeof(uid->secret); i++) {
    uid->secret[i] = 0xFF;
  }
}
static const uint8_t number_of_ids = 1;
static uint8_t ids[number_of_ids][8] = {
  { 0x33, 0x3A, 0x58, 0x25, 0x05, 0x00, 0x00, 0x1E }
};
static uint8_t secrets[number_of_ids][8] = {
  { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88 } 
};
static void clearEeprom() {
  for(uint16_t i = 0; i <= E2END; i++) {
    EEPROM.write(i, 0xFF);
  }
}
static void dumpEeprom(uint8_t length) {
  uint8_t r;
  for(uint16_t i = 0; i < length; i++) {
    r = EEPROM.read(i);
    if (r <= 16) {
      Serial.print("0");
    }
    Serial.print(r, HEX);
  }
  Serial.println();
}
static void setupEeprom() {
   uid_t uid;
  for(uint8_t i = 0; i < number_of_ids; i++) {
    for(uint8_t j = 0; j < sizeof(uid.id); j++) {
       uid.id[j] = ids[i][j];
    }
    for(uint8_t j = 0; j < sizeof(uid.secret); j++) {
       uid.secret[j] = secrets[i][j];
    }
    Serial.print("debug first by without or :");
    Serial.println(uid.secret[0],HEX);
    if (uid.secret[0] % 2 == 0) uid.secret[0] = uid.secret[0] + 1;
    hexdump(uid.secret, 8);
    Serial.print("debug first by with or :");
    Serial.println(uid.secret[0],HEX);
    EEPROM_writeAnything(i, uid);
  }
}

static struct uid_t findUidById(uint8_t id[]) {
  uid_t foundUid; initUid(&foundUid);
  uid_t uid;
  for(uidPosition = 0; uidPosition < number_of_ids; uidPosition++) {
    EEPROM_readAnything(uidPosition, uid);
    if (compareUint8_t(id, uid.id, sizeof(uid.id))) {
      foundUid = uid;
      break;
    }
  }
  return foundUid;
}
static uint8_t isValidUid(struct uid_t uid) {
  uint8_t isValid = 0;
  for(uint8_t i = 0; i < sizeof(uid.id) && !isValid; i++) {
    if (uid.id[i] != 0xFF) {
      isValid = 1;
    }
  }
  return isValid;
}
static void writeSecret(struct uid_t* uid) {
  uint8_t firstByteSecret = uid->secret[0];
  Serial.print("debug firstbytescret");
  Serial.println(firstByteSecret);
  uint8_t hasSecretNotBeenWritten = (firstByteSecret & 0x01);
  Serial.print("debug secret not been written");
  Serial.println(hasSecretNotBeenWritten);
  if (hasSecretNotBeenWritten) {
    uid->secret[0] = uid->secret[0] - 1;    
    if(ds.WriteSecret(uid->id, uid->secret)) {
      EEPROM_writeAnything(uidPosition, uid);
#if(SERIAL_DEBUG)
      Serial.print("Secret written: ");
      hexdump(uid->secret, sizeof(uid->secret));
#endif
    }
  }
}

static void setup() {  
  randomSeed(analogRead(2));
  
  Serial.begin(9600);
#if(SERIAL_DEBUG)
  Serial.println("reset");
#endif

  //clearEeprom();
  setupEeprom();
  dumpEeprom(16);
  uid_t u;
  EEPROM_readAnything(0, u);
  hexdump(u.id, 8);
  hexdump(u.secret, 8);

  // Read id.
  uint8_t id[8];
  ow.reset_search();
  while(!ow.search(id)) {}
#if(SERIAL_DEBUG)
  Serial.print("Found id: ");
  hexdump(id, sizeof(id));  
#endif
  
  // Find uid.
  uid_t uid = findUidById(id);
  if (isValidUid(uid)) {
#if(SERIAL_DEBUG)
    Serial.print("Found id in eeprom: ");
    hexdump(uid.id, sizeof(uid.id));
#endif     
    writeSecret(&uid);   
  
    // Read mac from iButton.
    uint8_t challenge[3] = { (uint8_t) random(0, 256), (uint8_t) random(0, 256), (uint8_t) random(0, 256) };
    uint8_t data[32];
    uint8_t macIButton[20];
    if (ds.ReadAuthWithChallenge(uid.id, 0, challenge, data, macIButton)) {
#if(SERIAL_DEBUG)
      Serial.print("Challange: ");
      hexdump(challenge, sizeof(challenge));
      Serial.print("Datapage: ");
      hexdump(data, sizeof(data));
      Serial.print("Hash calculated by iButton: ");
      hexdump(macIButton, sizeof(macIButton));
#endif
    }
  
    // Calculate mac.
    uint8_t macCalculated[20];
    uint8_t address = 0;
    calcmac_readauthpage(macCalculated, address, data, uid.secret, challenge, uid.id);
#if(SERIAL_DEBUG)
    Serial.print("Hash calculated by arduino: ");
    hexdump(macCalculated, sizeof(macCalculated));
#endif
  
    // Compare macs.
    compareUint8_t(macIButton, macCalculated, 20) ? Serial.println("Key is valid") : Serial.println("Key is _NOT_ valid.");
  }
  delay(5000);
}
static void loop() {
}
