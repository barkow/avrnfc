#include <freefare.h>
int nfc_initiator_init(nfc_device *pnd){
	return 0;
}
int nfc_device_set_property_bool(nfc_device *pnd, const nfc_property property, const bool bEnable){
	return 0;
}
int nfc_initiator_list_passive_targets(nfc_device *pnd, const nfc_modulation nm, nfc_target ant[], const size_t szTargets){
	return 0;
}
int nfc_initiator_target_is_present(nfc_device *pnd, const nfc_target *pnt){
	return 0;
}
int nfc_device_get_last_error(const nfc_device *pnd){
	return 0;
}
const char *nfc_strerror(const nfc_device *pnd){
	return "Hallo";
}
int nfc_initiator_select_passive_target(nfc_device *pnd, const nfc_modulation nm, const uint8_t *pbtInitData, const size_t szInitData, nfc_target *pnt){
	return 0;
}
int nfc_initiator_transceive_bytes(nfc_device *pnd, const uint8_t *pbtTx, const size_t szTx, uint8_t *pbtRx, const size_t szRx, int timeout){
	return 0;
}
int nfc_initiator_deselect_target(nfc_device *pnd){
	return 0;
}
void iso14443a_crc(uint8_t *pbtData, size_t szLen, uint8_t *pbtCrc){
}
void iso14443a_crc_append(uint8_t *pbtData, size_t szLen){
}
void nfc_init(nfc_context **context){
}
nfc_device *nfc_open(nfc_context *context, const nfc_connstring connstring){
	return NULL;
}
size_t nfc_list_devices(nfc_context *context, nfc_connstring connstrings[], size_t connstrings_len){
	return 0;
}