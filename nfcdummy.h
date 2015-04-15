#ifndef _NFCDUMMY_H_
#define _NFCDUMMY_H_

#define DEVICE_NAME_LENGTH  256
#define NFC_BUFSIZE_CONNSTRING 1024
#define MAX_USER_DEFINED_DEVICES 4
typedef char nfc_connstring[NFC_BUFSIZE_CONNSTRING];
typedef struct nfc_device nfc_device;
typedef struct nfc_context nfc_context;
struct nfc_user_defined_device {
  char name[DEVICE_NAME_LENGTH];
  nfc_connstring connstring;
  bool optional;
};
struct nfc_context {
  bool allow_autoscan;
  bool allow_intrusive_scan;
  uint32_t  log_level;
  struct nfc_user_defined_device user_defined_devices[MAX_USER_DEFINED_DEVICES];
  unsigned int user_defined_device_count;
};
struct nfc_device {
  const nfc_context *context;
  const struct nfc_driver *driver;
  void *driver_data;
  void *chip_data;
	  /** Device name string, including device wrapper firmware */
  char    name[DEVICE_NAME_LENGTH];
  /** Device connection string */
  nfc_connstring connstring;
  /** Is the CRC automaticly added, checked and removed from the frames */
  bool    bCrc;
  /** Does the chip handle parity bits, all parities are handled as data */
  bool    bPar;
  /** Should the chip handle frames encapsulation and chaining */
  bool    bEasyFraming;
  /** Should the chip try forever on select? */
  bool    bInfiniteSelect;
  /** Should the chip switch automatically activate ISO14443-4 when
      selecting tags supporting it? */
  bool    bAutoIso14443_4;
  /** Supported modulation encoded in a byte */
  uint8_t  btSupportByte;
  /** Last reported error */
  int     last_error;
};
typedef struct {
  uint8_t  abtAtqa[2];
  uint8_t  btSak;
  size_t  szUidLen;
  uint8_t  abtUid[10];
  size_t  szAtsLen;
  uint8_t  abtAts[254]; // Maximal theoretical ATS is FSD-2, FSD=256 for FSDI=8 in RATS
} nfc_iso14443a_info;
typedef enum {
  /**
   * Default command processing timeout
   * Property value's (duration) unit is ms and 0 means no timeout (infinite).
   * Default value is set by driver layer
   */
  NP_TIMEOUT_COMMAND,
  /**
   * Timeout between ATR_REQ and ATR_RES
   * When the device is in initiator mode, a target is considered as mute if no
   * valid ATR_RES is received within this timeout value.
   * Default value for this property is 103 ms on PN53x based devices.
   */
  NP_TIMEOUT_ATR,
  /**
   * Timeout value to give up reception from the target in case of no answer.
   * Default value for this property is 52 ms).
   */
  NP_TIMEOUT_COM,
  /** Let the PN53X chip handle the CRC bytes. This means that the chip appends
  * the CRC bytes to the frames that are transmitted. It will parse the last
  * bytes from received frames as incoming CRC bytes. They will be verified
  * against the used modulation and protocol. If an frame is expected with
  * incorrect CRC bytes this option should be disabled. Example frames where
  * this is useful are the ATQA and UID+BCC that are transmitted without CRC
  * bytes during the anti-collision phase of the ISO14443-A protocol. */
  NP_HANDLE_CRC,
  /** Parity bits in the network layer of ISO14443-A are by default generated and
   * validated in the PN53X chip. This is a very convenient feature. On certain
   * times though it is useful to get full control of the transmitted data. The
   * proprietary MIFARE Classic protocol uses for example custom (encrypted)
   * parity bits. For interoperability it is required to be completely
   * compatible, including the arbitrary parity bits. When this option is
   * disabled, the functions to communicating bits should be used. */
  NP_HANDLE_PARITY,
  /** This option can be used to enable or disable the electronic field of the
   * NFC device. */
  NP_ACTIVATE_FIELD,
  /** The internal CRYPTO1 co-processor can be used to transmit messages
   * encrypted. This option is automatically activated after a successful MIFARE
   * Classic authentication. */
  NP_ACTIVATE_CRYPTO1,
  /** The default configuration defines that the PN53X chip will try indefinitely
   * to invite a tag in the field to respond. This could be desired when it is
   * certain a tag will enter the field. On the other hand, when this is
   * uncertain, it will block the application. This option could best be compared
   * to the (NON)BLOCKING option used by (socket)network programming. */
  NP_INFINITE_SELECT,
  /** If this option is enabled, frames that carry less than 4 bits are allowed.
   * According to the standards these frames should normally be handles as
   * invalid frames. */
  NP_ACCEPT_INVALID_FRAMES,
  /** If the NFC device should only listen to frames, it could be useful to let
   * it gather multiple frames in a sequence. They will be stored in the internal
   * FIFO of the PN53X chip. This could be retrieved by using the receive data
   * functions. Note that if the chip runs out of bytes (FIFO = 64 bytes long),
   * it will overwrite the first received frames, so quick retrieving of the
   * received data is desirable. */
  NP_ACCEPT_MULTIPLE_FRAMES,
  /** This option can be used to enable or disable the auto-switching mode to
   * ISO14443-4 is device is compliant.
   * In initiator mode, it means that NFC chip will send RATS automatically when
   * select and it will automatically poll for ISO14443-4 card when ISO14443A is
   * requested.
   * In target mode, with a NFC chip compliant (ie. PN532), the chip will
   * emulate a 14443-4 PICC using hardware capability */
  NP_AUTO_ISO14443_4,
  /** Use automatic frames encapsulation and chaining. */
  NP_EASY_FRAMING,
  /** Force the chip to switch in ISO14443-A */
  NP_FORCE_ISO14443_A,
  /** Force the chip to switch in ISO14443-B */
  NP_FORCE_ISO14443_B,
  /** Force the chip to run at 106 kbps */
  NP_FORCE_SPEED_106,
} nfc_property;
typedef union {
  nfc_iso14443a_info nai;
  //nfc_felica_info nfi;
  //nfc_iso14443b_info nbi;
  //nfc_iso14443bi_info nii;
  //nfc_iso14443b2sr_info nsi;
  //nfc_iso14443b2ct_info nci;
  //nfc_jewel_info nji;
  //nfc_dep_info ndi;
} nfc_target_info;
typedef enum {
  NBR_UNDEFINED = 0,
  NBR_106,
  NBR_212,
  NBR_424,
  NBR_847,
} nfc_baud_rate;
typedef enum {
  NMT_ISO14443A = 1,
  NMT_JEWEL,
  NMT_ISO14443B,
  NMT_ISO14443BI, // pre-ISO14443B aka ISO/IEC 14443 B' or Type B'
  NMT_ISO14443B2SR, // ISO14443-2B ST SRx
  NMT_ISO14443B2CT, // ISO14443-2B ASK CTx
  NMT_FELICA,
  NMT_DEP,
} nfc_modulation_type;
typedef struct {
  nfc_modulation_type nmt;
  nfc_baud_rate nbr;
} nfc_modulation;
typedef struct {
  nfc_target_info nti;
  nfc_modulation nm;
} nfc_target;

#define NFC_SUCCESS			 0

void nfc_init(nfc_context **context);
int nfc_initiator_init(nfc_device *pnd);
int nfc_device_set_property_bool(nfc_device *pnd, const nfc_property property, const bool bEnable);
int nfc_initiator_list_passive_targets(nfc_device *pnd, const nfc_modulation nm, nfc_target ant[], const size_t szTargets);
int nfc_initiator_target_is_present(nfc_device *pnd, const nfc_target *pnt);
int nfc_device_get_last_error(const nfc_device *pnd);
const char *nfc_strerror(const nfc_device *pnd);
int nfc_initiator_select_passive_target(nfc_device *pnd, const nfc_modulation nm, const uint8_t *pbtInitData, const size_t szInitData, nfc_target *pnt);
int nfc_initiator_transceive_bytes(nfc_device *pnd, const uint8_t *pbtTx, const size_t szTx, uint8_t *pbtRx, const size_t szRx, int timeout);
int nfc_initiator_deselect_target(nfc_device *pnd);
void iso14443a_crc(uint8_t *pbtData, size_t szLen, uint8_t *pbtCrc);
void iso14443a_crc_append(uint8_t *pbtData, size_t szLen);
nfc_device *nfc_open(nfc_context *context, const nfc_connstring connstring);
size_t nfc_list_devices(nfc_context *context, nfc_connstring connstrings[], size_t connstrings_len);
#endif