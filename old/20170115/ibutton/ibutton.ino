#define SERIAL_DEBUG 1
#define WRITE_SECRET 0

#include <OneWire.h>
#include "ds1961.h"
#include "ds1961_sha.h"
#if (SERIAL_DEBUG)
  #include "hexdump.h"
#endif

static OneWire ow(2);
static DS1961 ds(&ow);

uint8_t compareMacs(uint8_t mac1[], uint8_t mac2[]) {
  uint8_t compare = 1;
  uint8_t length = 20;  
  for(uint8_t i = 0; (i < length) && compare; i++) {
    if (mac1[i] != mac2[i]) {
      compare = 0;
    }
  }
  return compare;
}

void setup() {
  randomSeed(analogRead(2));
}

void loop() {
  Serial.begin(9600);
#if(SERIAL_DEBUG)
  Serial.println("reset");
#endif
  
  // Read id.
  uint8_t id[8];
  ow.reset_search();
  while(!ow.search(id)) {}
  
#if(SERIAL_DEBUG)
  Serial.print("Found id: ");
  hexdump(id, sizeof(id));
#endif

  
  // Secret.
  uint8_t secret[] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88 };
#if(WRITE_SECRET)
  if (ds.WriteSecret(id, secret)) {
#if(SERIAL_DEBUG)
    Serial.print("Secret written: ");
    hexdump(secret, sizeof(secret));
#endif
  }
#endif
  
  // Read mac from iButton.
  uint8_t challenge[3] = { (uint8_t) random(0, 256), (uint8_t) random(0, 256), (uint8_t) random(0, 256) };
  uint8_t data[32];
  uint8_t macIButton[20];
  if (ds.ReadAuthWithChallenge(id, 0, challenge, data, macIButton)) {
#if(SERIAL_DEBUG)
    hexdump(challenge, sizeof(challenge));
    hexdump(data, sizeof(data));
    hexdump(macIButton, sizeof(macIButton));
#endif
  }
  
  // Calculate mac.
  uint8_t macCalculated[20];
  uint8_t address = 0;
  calcmac_readauthpage(macCalculated, address, data, secret, challenge, id);
#if(SERIAL_DEBUG)
  hexdump(macCalculated, sizeof(macCalculated));
#endif
  
  // Compare macs.
  compareMacs(macIButton, macCalculated) ? Serial.println("Key is valid") : Serial.println("Key is _NOT_ valid.");
  
  delay(5000);
}
