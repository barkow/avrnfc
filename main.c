#include <avr/io.h>
#include <freefare.h>
#include <nfcPN532.h>

#define IKAFKAPAYMENT_AID 				0xf7
#define IKAFKAPAYMENT_VALFILENO 	1
#define IKAFKAPAYMENT_DEBITVALUE	100

nfc_device* openNfcDevice(void){
	nfc_context *context;
	nfc_connstring devices[8];
	size_t device_count;
	
	/*nfc_init (&context);
  if (context == NULL)
		return -1;
	device_count = nfc_list_devices (context, devices, 8);
	if (device_count <= 0)
		return NULL;
	return nfc_open (context, devices[0]);*/
	Adafruit_PN532_begin();
	Adafruit_PN532_setPassiveActivationRetries(0xFF);
	//SAMConfig();
	return NULL;
}
 
int main (void) {
 
  int res;
	unsigned char* uid;
	uint8_t key_data[16]  = {0xf9,0x28,0x7d,0x1f,0xe8,0xb0,0xf2,0xf2,0x70,0xf0,0xe1,0x9f,0x05,0x8a,0xe0,0x51};
	uint8_t buffer[512];
	nfc_device *d;
	MifareTag tag;
	MifareDESFireAID aid;
	MifareDESFireKey key;
	printf("This is test");
  MifareTag *tags = NULL;
  
	d = openNfcDevice();
	  
	if (!d) {
		return -1;
	}
	while(1) {	
		tags = freefare_get_tags(d);
		while (!(tags[0])){
			freefare_free_tags (tags);
			tags = freefare_get_tags(d);
		}
		tag = tags[0];

		res = mifare_desfire_connect (tag);
		printf("Connect: %i\n", res);
	
		aid = mifare_desfire_aid_new (IKAFKAPAYMENT_AID);
		res = mifare_desfire_select_application (tag, aid);
		printf("Select App: %i\n", res);
	
		uid = freefare_get_tag_uid(tag);
		printf("GetUid: %i\n", res);
		printf("%i %i %i %i %i %i\n", uid[0], uid[1], uid[2], uid[3], uid[4], uid[5]);
	
		//Key berechnen
	
		key = mifare_desfire_aes_key_new (key_data);
		res = mifare_desfire_authenticate (tag, 1, key);
		printf("Auth: %i\n", res);
	
		res = mifare_desfire_debit_ex (tag, IKAFKAPAYMENT_VALFILENO, IKAFKAPAYMENT_DEBITVALUE, MDCM_ENCIPHERED);
		res = mifare_desfire_commit_transaction (tag);
		
		return 0;
	}
}