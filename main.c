#include <avr/io.h>
#include <freefare.h>

MifareTag getTagFromList(MifareTag* tags, unsigned int index) {
    return tags[index];
}

bool isTagInList(MifareTag* tags, unsigned int index) {
    return tags[index] ? 1 : 0;
}

int getDesfireUid(MifareTag tag, char *uid){
	char* u;
	int(i);
	u = freefare_get_tag_uid(tag);
	strcpy(uid, u);
	return 0;
}
 
int main (void) {
 
  int res;
	unsigned char uid[8];
	uint8_t key_data[16]  = {0xf9,0x28,0x7d,0x1f,0xe8,0xb0,0xf2,0xf2,0x70,0xf0,0xe1,0x9f,0x05,0x8a,0xe0,0x51};
	uint8_t buffer[512];
	nfc_device *d;
	MifareTag tag;
	MifareDESFireAID aid;
	MifareDESFireKey key;
	printf("This is test");
  MifareTag *tags = NULL;
  nfc_connstring devices[8];
  size_t device_count;
  nfc_context *context;
  nfc_init (&context);
  if (context == NULL)
		return -1;
	device_count = nfc_list_devices (context, devices, 8);
	if (device_count <= 0)
		return NULL;
	d = nfc_open (context, devices[0]);
	if (!d) {
		return -1;
	}
	while(1) {	
		tags = freefare_get_tags(d);
		while (!(isTagInList(tags, 0) != 0)){
			freefare_free_tags (tags);
			tags = freefare_get_tags(d);
		}
		tag = getTagFromList(tags, 0);

		res = mifare_desfire_connect (tag);
		printf("Connect: %i\n", res);
	
		aid = mifare_desfire_aid_new (0xf8);
		res = mifare_desfire_select_application (tag, aid);
		printf("Select App: %i\n", res);
	
		res = getDesfireUid (tag, uid);
		printf("GetUid: %i\n", res);
		printf("%i %i %i %i %i %i\n", uid[0], uid[1], uid[2], uid[3], uid[4], uid[5]);
	
		key = mifare_desfire_aes_key_new (key_data);
		res = mifare_desfire_authenticate (tag, 1, key);
		printf("Auth: %i\n", res);
	
		res = mifare_desfire_read_records (tag, 1, 0, 0, buffer);
		printf("Read Records: %i\n", res);
		printf("Buffer: %i %i %i %i %i %i %i %i", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7]);
   }
   return 0;
}