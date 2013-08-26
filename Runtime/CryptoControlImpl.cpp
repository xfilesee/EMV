// CryptoControl service
// CryptoControl Interface Implementation -- Server Side
// This service is based on RSA B-SAFE Crypto-C ME library, which 
// implements all cryptographic operations.

#include "CryptoControlImpl.h"
#include "AccessManager.h"
#include "cryptlib.h"
//#include "../kartek/emv.h"

#define XDIGIT 4096
#define IDIGIT 4096
#define uchar unsigned char
#define MAX(x,y) x>y?x:y
#define MIN(x,y) x<y?x:y
#define	CA_PUBLIC_KEY_MOD	0
#define	ICC_PUBLIC_KEY_MOD	1
#define	ISS_PUBLIC_KEY_MOD	2
#define	ICC_PIN_ENC_PUBLIC_KEY_MOD	3
#define TRUE  1
#define FALSE 0

typedef unsigned long int UINT4;


typedef struct {
  UINT4 i[2];                   /* number of _bits_ handled mod 2^64 */
  UINT4 buf[4];                                    /* scratch buffer */
  unsigned char in[64];                              /* input buffer */
  unsigned char digest[16];     /* actual digest after MD5Final call */
} MD5_CTX;

typedef union{
	unsigned int x;
	unsigned char b[2];
} CHAR2BYTES;

CHAR2BYTES a;

#define PUB_KEY_MODULO_LEN	256

uchar ucCAPubKeyMod[PUB_KEY_MODULO_LEN];
uchar ucIssPubKeyMod[PUB_KEY_MODULO_LEN];
uchar ucICCPubKeyMod[PUB_KEY_MODULO_LEN];
uchar ucRecPubKeyMod[PUB_KEY_MODULO_LEN];
uchar ucRecoveredData[PUB_KEY_MODULO_LEN];
uchar ucICCPINEncPubKeyMod[PUB_KEY_MODULO_LEN];


int mpDivShort( unsigned char dividend[],
                unsigned char divisor,
                unsigned char quotient[],
                unsigned char remainder[]);
int mpMulShort( unsigned char multiplicand[],
                unsigned char multiplier,
                unsigned char product[]);
unsigned char *mpLeftByteShift( unsigned char x[],
                                unsigned int  s);
int mpSubShort( unsigned char minuend[],
                unsigned char subtrahend,
                unsigned char difference[]);
uchar RSAEncrypt(unsigned char ucPubKeyType, uchar PubKeyLen, unsigned char *exp, int expLen, unsigned char *mesaj, uchar mesajLen);
int mpCompare( unsigned char x[], unsigned char y[]);
int mpMod( unsigned char dividend[],
           unsigned char divisor[],
           unsigned char remainder[]);
int mpSub( unsigned char minuend[],
           unsigned char subtrahend[],
           unsigned char difference[]);
int mpMul( unsigned char multiplicand[],
           unsigned char multiplier[],
           unsigned char product[]);

				
void MD5Init (void);
void MD5Update (MD5_CTX *mdContext, unsigned char *inBuf, unsigned int inLen);
void MD5Final (MD5_CTX *mdContext);

unsigned char pucDum1[XDIGIT],pucDum2[XDIGIT],pucOne[XDIGIT];


/* F, G and H are basic MD5 functions: selection, majority, parity */
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

/* ROTATE_LEFT rotates x left n bits */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4 */
/* Rotation is separate from addition to prevent recomputation */
#define FF(a, b, c, d, x, s, ac) {(a) += F ((b), (c), (d)) + (x) + (UINT4)(ac); (a) = ROTATE_LEFT ((a), (s)); (a) += (b); }

#define GG(a, b, c, d, x, s, ac) {(a) += G ((b), (c), (d)) + (x) + (UINT4)(ac); (a) = ROTATE_LEFT ((a), (s)); (a) += (b); }
#define HH(a, b, c, d, x, s, ac) {(a) += H ((b), (c), (d)) + (x) + (UINT4)(ac); (a) = ROTATE_LEFT ((a), (s)); (a) += (b); }
#define II(a, b, c, d, x, s, ac) {(a) += I ((b), (c), (d)) + (x) + (UINT4)(ac); (a) = ROTATE_LEFT ((a), (s));(a) += (b); }


CryptoControlImpl::CryptoControlImpl(void):
	pubKeyArr (0),
	pubKeyLength(0),
	uiCryptoAlgID(0),
	uiHashAlgID(0)
{
	lib_ctx = NULL;  // Library context
	ctx = NULL;      // Cryptographic context
	pkey_ctx = NULL; // Key context
	LibStatus = ERR_CRYPTOCME_LIB_NOT_INITIALIZED;
	setServiceName("CryptControl");
	opEvent = new CryptoOperationEventImpl;
}

CryptoControlImpl::~CryptoControlImpl(void)
{
	clean();
	releaseCryptoCME();
}

long CryptoControlImpl::InitCriptoCME()
{

	int res;
	// Initialize CRYPTOLIB
	res = cryptInit();
	if(res != CRYPT_OK){
		printf("Crypt library initialization failure!\n");
		exit(1);
	}
	
	res = cryptAddRandom( NULL, CRYPT_RANDOM_SLOWPOLL );
	if(res != CRYPT_OK){
		printf("Crypt library initialization failure!\n");
		exit(1);
	}
	
/*	
	
	// 1. Create the library context 
    if (PRODUCT_LIBRARY_NEW(PRODUCT_DEFAULT_RESOURCE_LIST(),
									R_RES_FLAG_DEF, 
									&lib_ctx) != R_ERROR_NONE)
	{
		//Unable to create library context
		return (LibStatus = ERR_CRYPTOCME_SETTING_LIB_CONTEXT);
	}

    // 2. Create a new cryptographic context R_CR_CTX 
    if (R_CR_CTX_new(lib_ctx, R_RES_FLAG_DEF, &ctx) != R_ERROR_NONE)
    {
		// Unable to create crypto context
		return (LibStatus = ERR_CRYPTOCME_SETTING_CRYPTO_CONTEXT);
    }

	// 3. Create a key context R_PKEY_CTX for loading the keys from file 
    if (R_PKEY_CTX_new(lib_ctx, R_RES_FLAG_DEF, R_PKEY_TYPE_RSA,
        &pkey_ctx) != R_ERROR_NONE)
	{
		//Unable to create key context
		return (LibStatus = ERR_CRYPTOCME_SETTING_KEY_CONTEXT);
    }
    */
	return (LibStatus = R_ERROR_NONE);
}

void CryptoControlImpl::releaseCryptoCME()
{



	// Free CRYPTOLIB
	cryptEnd();
	
	
	// Free library resources
	if (pkey_ctx != NULL)
    {
        R_PKEY_CTX_free(pkey_ctx);
		pkey_ctx = NULL;
    }

    if (ctx != NULL)
    {
        R_CR_CTX_free(ctx);
		ctx= NULL;
    }

    if (lib_ctx != NULL)
    {
        PRODUCT_LIBRARY_FREE(lib_ctx);
		lib_ctx = NULL;
    }
	LibStatus = ERR_CRYPTOCME_LIB_NOT_INITIALIZED;
}

// checkInitialization
// Purpose: Checks if the OperationEvent object is added to the service, and if 
//   the application name is initialized. 
// Returns SUCCESS if everything is initialized.
long CryptoControlImpl::checkInitialization (bool checkEvent)
{
	if (checkEvent)
	{
		// Check if the OperationEvent has been added to this object
		if (!opEvent )
			return ERR_EVT_NO_EVNT_OBJECT;

		if (opEvent->operation_running())
			return ERR_OPEVNT__ALREADY_RUNNING;
	}
	
	// Check if the Application Name has been added (call to the initialize() 
	// must be made before calling this function)
/*	
	if (!this->getAccessManager ())
	{
		return opEvent->setError (ERR_AM_NOT_INITIALIZED, CRYPTO_EXECUTION);
	}
	*/
	return SUCCESS;
}

// Interface Method ***************************************************************
// Purpose: Initializes CA Public key from the Registry.
// Parameters:
//   bRID - Registered Identifier ID, a byte array containing the RID under which
//		the key is stored in the Registry
//   ridLen - Length of bRID in bytes
//   bPKI - Public Key Index, a byte array containing an index under which the key
//		is stored in the registry
//   pkiLen - Number of elements in bPKI
// Returns:
//    SUCCESS - if operation succeeds (use CryptoOperationEvent object to retreive
//				the actual value)
//    EVT_ERROR - if operation failed (use CnfgOperationEvent object to get more
//				details about the error.
// ***********************************************************************************
int CryptoControlImpl::initKeyCA(const byte *bRID, unsigned int ridLen, 
						         const byte *bPKI, unsigned int pkiLen,
								 unsigned int *modulus)
{
	long res = checkInitialization (false);
	if (res != SUCCESS)
		return res;

	// Convert bRID from a byte array to a string
	char *pRID = hex2ascii(bRID, ridLen);
	// Convert bPKI from a byte array to a string 
	char *pPKI = hex2ascii(bPKI, pkiLen);

	// Read Public Key, Crypto and Hash algorithm from the registry
	res = setKeyFromRegistry(pRID, pPKI, modulus);
	delete [] pRID;
	delete [] pPKI;
	return res;
}


// Interface Method ***************************************************************
// Purpose: Initializes a Public key from the modulus and exponent values passed as
//          parameters.
// Parameters:
//   keyMod - a byte array containing the value of public key modulus
//   uiKeyModLen - Number of elements in keyMod
//   keyExp - a byte array containing the value of public key exponent
//   uiKeyExpLen - Number of elements in keyExp
//   uiCryptoAlgID - ID of the algorithm implemented encryption/ decryption
//   uiHashAlgID - ID of the algorithm implemented a HASH function
// Returns:
//    SUCCESS - if operation succeeds (use CryptoOperationEvent object to retreive
//				the actual value)
//    EVT_ERROR - if operation failed (use CnfgOperationEvent object to get more
//				details about the error.
// ***********************************************************************************
int CryptoControlImpl::initKey(const byte *keyMod, unsigned int uiKeyModLen, 
							   const byte *keyExp, unsigned int uiKeyExpLen,
							   unsigned int uiCryptoAlgID, unsigned int uiHashAlgID)
{
	long res = checkInitialization (false);
	if (res != SUCCESS)
		return res;

	byte *btKey = 0;
	UNINT size = 0;

	RSAPublicKey pubKeyObj;
	// Create a public key in a format specified by ASN.1 as RSAPublicKey type
	
	res = pubKeyObj.setKey (keyMod, uiKeyModLen, keyExp, uiKeyExpLen);
	if (res != KEY_SUCCESS)
		return ERR_INITIALIZING_PUB_KEY;

	size = pubKeyObj.getKey (&btKey);

	if ((res = setPubKey(btKey, size)) != SUCCESS)
		return res;
	
	setCryptoAlg (uiCryptoAlgID);
	setHashAlg (uiHashAlgID);

	return SUCCESS;
}

/**
Interface Method 

Purpose: Encrypts data
@param pDataBuf - a byte array containing a plain text to be encrypted
@param uiDataLen - Number of elements in pDataBuf
@return
    SUCCESS - if operation succeeds (use CryptoOperationEvent object to retreive
				the actual value)
    EVT_ERROR - if operation failed (use CnfgOperationEvent object to get more
				details about the error.

 Note: This function uses a public key initialized by call to the function initKey 
		or initKeyCA
*/
int CryptoControlImpl::encrypt(byte *pDataBuf,
						       unsigned int uiDataLen)
{
	long res = checkInitialization ();
	if (res != SUCCESS)
		return res;

	opEvent->begin_operation();

	// Cast the OperationEvent object to the appropriate type
	CryptoOperationEventImpl *pEvent = (CryptoOperationEventImpl*) opEvent;
	
	//Reset event
	pEvent->resetEvent();
	
	// Check if the public key is initialized
	if (!pubKeyArr)
	{
		return pEvent->setError (ERR_CRYPT_KEY_NOT_INITIALIZED, CRYPTO_EXECUTION);
	}

	// Select an encryption algorithm to run
	switch (uiCryptoAlgID)
	{
	case 1:
		return encrypt_cryptome(pDataBuf, uiDataLen, pEvent);
	default:
		return pEvent->setError (ERR_ALGORITHM_NOT_SUPPORTED, CRYPTO_EXECUTION);
	}
}

// Interface Method ***************************************************************
// Purpose: Decrypts data
// Parameters:
//   pDataBuf - a byte array containing a cipher text to be decrypted
//   uiDataLen - Number of elements in pDataBuf
// Returns:
//    SUCCESS - if operation succeeds (use CryptoOperationEvent object to retreive
//				the actual value)
//    EVT_ERROR - if operation failed (use CnfgOperationEvent object to get more
//				details about the error.
// Note: This function uses a public key initialized by call to the function initKey 
//		or initKeyCA
// ***********************************************************************************
int CryptoControlImpl::decrypt(byte *pCipherBuf,
							   unsigned int uiCipherLen)
{
	long res = checkInitialization ();
	if (res != SUCCESS)
		return res;

	opEvent->begin_operation();

	// Cast the OperationEvent object to the appropriate type
	CryptoOperationEventImpl *pEvent = (CryptoOperationEventImpl*) opEvent;
	
	//Reset event
	pEvent->resetEvent();
	
	// Check if the public key is initialized
	if (!pubKeyArr)
	{
		return pEvent->setError (ERR_CRYPT_KEY_NOT_INITIALIZED, CRYPTO_EXECUTION);
	}

	// Select a decryption algorithm to run
	switch (uiCryptoAlgID)
	{
	case 1:
		res = decrypt_cryptome(pCipherBuf, uiCipherLen, pEvent);
		return res;
	default:
		return pEvent->setError (ERR_ALGORITHM_NOT_SUPPORTED, CRYPTO_EXECUTION);
	}
}

// Interface Method ***************************************************************
// Purpose: calculates a hash number for a message pMsg
// Parameters:
//   pDataBuf - a byte array containing a message to be digested (input to Hash algorithm)
//   uiDataLen - Number of elements in pDataBuf
// Returns:
//    SUCCESS - if operation succeeds (use CryptoOperationEvent object to retreive
//				the actual value)
//    EVT_ERROR - if operation failed (use CnfgOperationEvent object to get more
//				details about the error.
// ***********************************************************************************
int CryptoControlImpl::calcHash(byte *pMsg,
								unsigned int uiMsgLen)
{
	long res = checkInitialization ();
	if (res != SUCCESS)
		return res;

	opEvent->begin_operation();

	// Cast the OperationEvent object to the appropriate type
	CryptoOperationEventImpl *pEvent = (CryptoOperationEventImpl*) opEvent;
	
	//Reset event
	pEvent->resetEvent();
	
	// Check an input buffer
	if (!pMsg || uiMsgLen <= 0)
	{
		// Handle a special case when an input buffer is empty
		byte buff [] = {0xDA, 0x39, 0xA3, 0xEE, 0x5E, 0x6B, 0x4B, 0x0D, 0x32, 
			0x55, 0xBF, 0xEF, 0x95, 0x60, 0x18, 0x90, 0xAF, 0xD8, 0x07, 0x09};
		res = pEvent->setByteString (buff, 20);
		return res;
	}

	// Check if the public key is initialized
	if (!pubKeyArr)
	{
		return pEvent->setError (ERR_CRYPT_KEY_NOT_INITIALIZED, CRYPTO_EXECUTION);
	}

	// Select a hashing algorithm to run
	switch (uiCryptoAlgID)
	{
	case 1:
		return hash_cryptome(pMsg, uiMsgLen, pEvent);
	default:
		return pEvent->setError (ERR_ALGORITHM_NOT_SUPPORTED, CRYPTO_EXECUTION);
	}
}

// Interface Method ***************************************************************
// Purpose: Generates a random number of size uiLength
// Parameters:
//   uiAlgID - an algorithm to be used for generated a random number
//   uiDataLen - Number of bytes in a random number
// Returns:
//    SUCCESS - if operation succeeds (use CryptoOperationEvent object to retreive
//				the actual value)
//    EVT_ERROR - if operation failed (use CnfgOperationEvent object to get more
//				details about the error.
// ***********************************************************************************
int CryptoControlImpl::randomNumber(unsigned int uiAlgID, unsigned int uiLength)
{
	long res = checkInitialization ();
	if (res != SUCCESS)
		return res;

	opEvent->begin_operation();

	// Cast the OperationEvent object to the appropriate type
	CryptoOperationEventImpl *pEvent = (CryptoOperationEventImpl*) opEvent;
	
	//Reset event
	pEvent->resetEvent();
	
	// Check if the public key is initialized
	//if (!pubKeyArr)
	//{
	//	return pEvent->setError (ERR_CRYPT_KEY_NOT_INITIALIZED, CRYPTO_EXECUTION);
	//}

	// Select a random number generation algorithm to run
	switch (uiAlgID)
	{
	case 1:
		return rand_cryptome(uiLength, pEvent);
	default:
		return pEvent->setError (ERR_ALGORITHM_NOT_SUPPORTED, CRYPTO_EXECUTION);
	}
}

byte CryptoControlImpl::getCryptoAlgID()
{
	return uiCryptoAlgID;
}

byte CryptoControlImpl::getHashAlgID()
{
	return uiHashAlgID;
}



// --------------------------------------------------------------------

// ---------------- C R Y P T O    F U N C T I O N S ------------------

// --------------------------------------------------------------------

// ----------------------------------------------------
// -------  D E C R Y P T I O N  ----------------------
// ----------------------------------------------------
int CryptoControlImpl::decrypt_cryptome (byte *pCipherBuf,
										 unsigned int uiCipherLen,
										 CryptoOperationEventImpl *opEvent)
{
	int res;
	/*
	if (LibStatus == ERR_CRYPTOCME_LIB_NOT_INITIALIZED)
	{
		if ((res = InitCriptoCME()) != R_ERROR_NONE)
		{
			// Failed to initialize CRYPTO C ME library
			return opEvent->setError(res, CRYPTO_EXECUTION);
		}
	}
*/
	// allocate a buffer to holde a plaintext  
	byte *buf = new byte [uiCipherLen];
	if (!buf)
		return opEvent->setError(ERR_MEMORY_ALLOC, CRYPTO_EXECUTION);

	unsigned int len = uiCipherLen;
	
	// Decrypt data
    if ((res = decrypt_data(pCipherBuf, uiCipherLen, buf, &len)) != R_ERROR_NONE)
    {
		// Failed to decrypt the cipher
		delete [] buf;
        return opEvent->setError(res, CRYPTO_EXECUTION);
    }

	// set OperationEvent object with a result of decryption
	res = opEvent->setByteString (buf, len);
	delete [] buf;

	return res;
}

/*
CryptoPP::RandomPool & GlobalRNG()
{
        static CryptoPP::RandomPool randomPool;
        return randomPool;
}
*/

int CryptoControlImpl::decrypt_data(byte *data, unsigned int dlen,
					byte *buf,  unsigned int *mlen)
{
    int     ret         = R_ERROR_NONE;
    R_CR    *dec_obj    = NULL;
    R_PKEY  *pkey       = NULL;

	byte   *tmp;
    size_t          consumed_len;
	byte   *dbuf       = NULL;
	long            datalen;

	if (LibStatus == ERR_CRYPTOCME_LIB_NOT_INITIALIZED)
	{
		if ((ret = InitCriptoCME()) != R_ERROR_NONE)
		{
			// Failed to initialize CRYPTO C ME library
			return opEvent->setError(ret, CRYPTO_EXECUTION);
		}
	}
	
	// Initialize local variables with the Public Key values that must have been
	//    initialized before calling this function 
	dbuf = this->pubKeyArr;
	datalen = this->pubKeyLength;
	tmp = dbuf;


	printf("PUBLIC KEY:\n");
	printHex(dbuf,datalen);
	
	printf("\n\nDATA TO DECRYPT:\n");
	printHex(data,dlen);

	
	RSAPublicKey key;
	key.setKey(this->pubKeyArr,this->pubKeyLength);

	memcpy(ucCAPubKeyMod, key.Modulus, key.ModLen);
	RSAEncrypt(CA_PUBLIC_KEY_MOD, key.ModLen-1, key.Exponent, key.ExponentLen, data, dlen);
	
	
	
	printf("\n\nucRecoveredData:\n");
	printHex(ucRecoveredData,this->pubKeyLength);
	
	
	/*
	
	CRYPT_CONTEXT contextRSA ;
	CRYPT_KEYSET cryptKeyset;
	
	cryptCreateContext( &contextRSA, CRYPT_UNUSED, CRYPT_ALGO_RSA );
	
	
	RSAPublicKey key;
	CRYPT_PKCINFO_RSA *rsaKey;
	
        if( ( rsaKey = ( CRYPT_PKCINFO_RSA * ) malloc( sizeof( CRYPT_PKCINFO_RSA ) ) ) == NULL )
                return( CRYPT_ERROR_MEMORY );

	key.setKey(this->pubKeyArr,this->pubKeyLength);
	
	ret = cryptSetAttributeString( contextRSA, CRYPT_CTXINFO_LABEL,
			"RSA public key", strlen( "RSA public key" ) );
	cryptInitComponents( rsaKey, CRYPT_KEYTYPE_PUBLIC );
	cryptSetComponent( rsaKey->n, key.Modulus, (key.ModLen) * 8 );
	
	printf("\n\nNNNNNNNNNNNNNNNNNNNNNNNNNn:\n");
	printHex(rsaKey->n,key.ModLen);
	
	cryptSetComponent( rsaKey->e, key.Exponent, key.ExponentLen * 8 );
	ret = cryptSetAttributeString( contextRSA,
			CRYPT_CTXINFO_KEY_COMPONENTS, rsaKey,
			sizeof( CRYPT_PKCINFO_RSA ) );
	//cryptDestroyComponents( rsaKey );
	
	//ret=cryptDecrypt(contextRSA, buf, *mlen);
	
	CRYPT_ENVELOPE cryptEnvelope;
	int bytesCopied;
	
	ret = cryptCreateEnvelope( &cryptEnvelope, CRYPT_UNUSED,  CRYPT_FORMAT_PKCS7 );
	
	
	ret = cryptSetAttribute( cryptEnvelope, //CRYPT_ENVINFO_PUBLICKEY,
		CRYPT_ENVINFO_SIGNATURE,
		contextRSA );
	
	// Add the enveloped data and the signature check key required to
	//   verify the signature, and pop out the recovered message 
	ret = cryptPushData( cryptEnvelope, data, dlen,
					&bytesCopied );
	ret = cryptFlushData( cryptEnvelope );
	ret = cryptPopData( cryptEnvelope, buf, *mlen, &bytesCopied);
	ret = cryptDestroyEnvelope( cryptEnvelope );
	
	*/	
	
/*	CryptoPP::StringSource ss(
"30 4c 30 0d 06 09 2a 86 "
"48 86 f7 0d 01 01 01 05 "
"00 03 3b 00 30 38 02 33 "
"00 a3 07 9a 90 df 0d fd "
"72 ac 09 0c cc 2a 78 b8 "
"74 13 13 3e 40 75 9c 98 "
"fa f8 20 4f 35 8a 0b 26 "
"3c 67 70 e7 83 a9 3b 69 "
"71 b7 37 79 d2 71 7b e8 "
"34 77 cf 02 01 03"
	,true,new CryptoPP::HexDecoder) ;*/
	/*
        CryptoPP::FileSource f("aaa", true, new CryptoPP::HexDecoder);
        //CryptoPP::FileSource f("rsa400pb.dat", true, new CryptoPP::HexDecoder);
        //CryptoPP::RSASS<CryptoPP::PSSR, CryptoPP::SHA>::Signer signer(f);
        //CryptoPP::RSASS<CryptoPP::PSSR, CryptoPP::SHA>::Verifier verifier(f);	
        CryptoPP::RSASS<CryptoPP::PSSR, CryptoPP::SHA>::Verifier verifier(f);	

	//CryptoPP::X509PublicKey myPkey;
	
        // sign
	unsigned int signatureLen = *mlen;
	unsigned int messageLen = 1024*5;
        CryptoPP::SecByteBlock signature(signatureLen);
        //CryptoPP::AutoSeededRandomPool rng;
        //unsigned int signatureLen = signer.SignMessageWithRecovery(rng, message, messageLen, NULL, 0, signature);
	
	strncpy((char *)signature.data(),(char *)buf,signatureLen);
        // verify and recover
        CryptoPP::SecByteBlock recovered(verifier.MaxRecoverableLengthFromSignatureLength(signatureLen));
        CryptoPP::DecodingResult result = verifier.RecoverMessage(recovered, NULL, 0, signature, signatureLen);
        if (!result.isValidCoding){
                //throw CryptoPP::InvalidSignature();
		printf("INVALID SIGNATURE!!!!\n");
	}
        unsigned int recoveredLen = result.messageLength;
	*/
/*	
        CryptoPP::FileSource f("rsa1024.dat", true, new CryptoPP::HexDecoder);
        CryptoPP::RSASS<CryptoPP::PSSR, CryptoPP::SHA>::Signer signer(f);
        CryptoPP::RSASS<CryptoPP::PSSR, CryptoPP::SHA>::Verifier verifier(signer);	

        // sign
        byte message[] = "test";
        unsigned int messageLen = sizeof(message);
        CryptoPP::SecByteBlock signature(signer.MaxSignatureLength(messageLen));
        CryptoPP::AutoSeededRandomPool rng;
        unsigned int signatureLen = signer.SignMessageWithRecovery(rng, message, messageLen, NULL, 0, signature);

        // verify and recover
        CryptoPP::SecByteBlock recovered(verifier.MaxRecoverableLengthFromSignatureLength(signatureLen));
        CryptoPP::DecodingResult result = verifier.RecoverMessage(recovered, NULL, 0, signature, signatureLen);
        if (!result.isValidCoding){
                //throw CryptoPP::InvalidSignature();
		printf("INVALID SIGNATURE!!!!\n");
	}
        unsigned int recoveredLen = result.messageLength;
*/

/*		
	CryptoPP::StringSource priv;// ((const byte *)dbuf,(unsigned int)datalen,true);
	priv.Put(dbuf,datalen);
	priv.MessageEnd();
	//CryptoPP::StringSource privString("/home/ntufar/EMV2/kernel/aaa.dat", true, new CryptoPP::HexDecoder);
	CryptoPP::RSAES_OAEP_SHA_Decryptor rsaPriv(priv);
	//rsaPriv.AccessKey().BERDecodeKey(privFile);
	CryptoPP::DecodingResult result = rsaPriv.FixedLengthDecrypt(GlobalRNG(), data, buf);
/*	
	InitCriptoCME();
	// Create a public key from the array of bytes
	if ((ret = R_PKEY_from_public_key_binary(pkey_ctx, R_PKEY_FL_BY_REFERENCE, 
        R_PKEY_TYPE_RSA, (size_t)datalen, (const byte *) tmp,
        &consumed_len, &pkey)) != R_ERROR_NONE)
	{
		// Unable to read public key from the byte array
		printf("Load of public key failed:\n");
		printHex(tmp,datalen);
				
		return ERR_CRYPTOCME_KEY_FROM_BINARY;
	}

    // 6. [Decrypt] Create a new asymmetric cryptographic object R_CR 
    if ((ret = R_CR_new(ctx, R_CR_TYPE_ASYM, R_CR_ID_RSA,
        R_CR_SUB_PUB_DEC, &dec_obj)) != R_ERROR_NONE)
    {
        //Unable to create crypto object
		R_PKEY_free(pkey);
        return ERR_CRYPTOCME_CRYPTO_OBJECT;
    }

    // 7. [Decrypt] Initialize object with the key. 
    if ((ret = R_CR_asym_decrypt_init(dec_obj, pkey)) != R_ERROR_NONE)
    {
        // Unable to initialize crypto object
		R_CR_free(dec_obj);
		R_PKEY_free(pkey);
        return ERR_CRYPTOCME_INIT_OBJ_W_KEY;
    }

    // 8.  [Decrypt] Decrypt the data 
    if ((ret = R_CR_asym_decrypt(dec_obj, data, dlen, buf, mlen)) !=
        R_ERROR_NONE)
    {
		// Unable to decrypt data
		R_CR_free(dec_obj);
		R_PKEY_free(pkey);
        return ERR_CRYPTOCME_DECRYPT_FAILED;
    }
*/


	

    	printf("Decrypted!!!!!!!!!!\n");
	printHex(buf,*mlen);
	// Free resources
	R_CR_free(dec_obj);
	R_PKEY_free(pkey);
    
	return(ret);
}


// -------------------------------------------------------------------
//       E N C R Y P T I O N 
//--------------------------------------------------------------------
int CryptoControlImpl::encrypt_cryptome (byte *pPlainTextBuf,
										 unsigned int uiPlainTextLen,
										 CryptoOperationEventImpl *opEvent)
{
	int res;
	if (LibStatus == ERR_CRYPTOCME_LIB_NOT_INITIALIZED)
	{
		if ((res = InitCriptoCME()) != R_ERROR_NONE)
		{
			// Failed to initialize CRYPTO C ME library
			return opEvent->setError(res, CRYPTO_EXECUTION);
		}
	}

	// Allocate a buffer to hold a ciphertex
	byte *buf = new byte [uiPlainTextLen];
	if (!buf)
		return opEvent->setError(ERR_MEMORY_ALLOC, CRYPTO_EXECUTION);

	unsigned int len = uiPlainTextLen;
    if ((res = encrypt_data(pPlainTextBuf, uiPlainTextLen, 
							buf, &len)) != R_ERROR_NONE)
    {
		// Failed to decrypt the cipher
		delete [] buf;
        return opEvent->setError(res, CRYPTO_EXECUTION);
    }

	// set OperationEvent object with a result of decryption
	res = opEvent->setByteString (buf, len);
	delete [] buf;
	return res;
}

int CryptoControlImpl::encrypt_data(byte *data, unsigned int dlen,
									byte *buf,  unsigned int *mlen)
{
    int     ret         = R_ERROR_NONE;
    R_CR    *dec_obj    = NULL;
    R_PKEY  *pkey       = NULL;

	byte   *tmp;
    size_t          consumed_len;
	byte   *dbuf       = NULL;
	long            datalen;

	// Initialize local variables with the Public Key values that must have been
	//    initialized before calling this function 
	dbuf = this->pubKeyArr;
	datalen = this->pubKeyLength;
	
	tmp = dbuf;
	// Create a public key
	if ((ret = R_PKEY_from_public_key_binary(pkey_ctx, R_PKEY_FL_BY_REFERENCE, 
        R_PKEY_TYPE_RSA, (size_t)datalen, (const byte *) tmp,
        &consumed_len, &pkey)) != R_ERROR_NONE)
	{
		// Unable to read public key from the byte array
		return ERR_CRYPTOCME_KEY_FROM_BINARY;
	}

    // 1.  Create a new asymmetric cryptographic object R_CR 
    if ((ret = R_CR_new(ctx, R_CR_TYPE_ASYM, R_CR_ID_RSA,
        R_CR_SUB_PUB_ENC, &dec_obj)) != R_ERROR_NONE)
    {
        //Unable to create crypto object
		R_PKEY_free(pkey);
        return ERR_CRYPTOCME_CRYPTO_OBJECT;
    }

    // 2. Initialize object with the key. 
    if ((ret = R_CR_asym_encrypt_init(dec_obj, pkey)) != R_ERROR_NONE)
    {
        // Unable to initialize crypto object
		R_CR_free(dec_obj);
		R_PKEY_free(pkey);
        return ERR_CRYPTOCME_INIT_OBJ_W_KEY;
    }

    // 3.  Encrypt the data 
    if ((ret = R_CR_asym_encrypt(dec_obj, data, dlen, buf, mlen)) !=
        R_ERROR_NONE)
    {
        // Unable to encrypt data
		R_CR_free(dec_obj);
		R_PKEY_free(pkey);
        return ERR_CRYPTOCME_ENCRYPT_FAILED;
    }
	
	// Free resources
	R_CR_free(dec_obj);
	R_PKEY_free(pkey);
    
	return(ret);
}
//---------------------------------------------------------------------

// -----------   H A S H       H A S H  -------------------------------
//---------------------------------------------------------------------
int CryptoControlImpl::hash_cryptome (byte *pData,
						        	  unsigned int uiDataLen,
									  CryptoOperationEventImpl *opEvent)
{
	int res;
	if (LibStatus == ERR_CRYPTOCME_LIB_NOT_INITIALIZED)
	{
		if ((res = InitCriptoCME()) != R_ERROR_NONE)
		{
			// Failed to initialize CRYPTO C ME library
			return opEvent->setError(res, CRYPTO_EXECUTION);
		}
	}

	// Allocate a buffer to hold a Hash number of size R_CR_DIGEST_MAX_LEN
	byte *buf = new byte [R_CR_DIGEST_SHA1_LEN];
	if (!buf)
		return opEvent->setError(ERR_MEMORY_ALLOC, CRYPTO_EXECUTION);

	unsigned int len = R_CR_DIGEST_SHA1_LEN;
    if ((res = hash_data(pData, uiDataLen, buf, &len)) != R_ERROR_NONE)
    {
		// Failed to decrypt the cipher
		delete [] buf;
        return opEvent->setError(res, CRYPTO_EXECUTION);
    }

	// set OperationEvent object with a result of decryption
	res = opEvent->setByteString (buf, len);
	delete [] buf;
	return res;
}

int CryptoControlImpl::hash_data(byte *data, unsigned int dlen,
								 byte *buf,  unsigned int *mlen)
{
    int     ret         = R_ERROR_NONE;
    R_CR    *dec_obj    = NULL;

	// Initialize local variables with the Public Key values that must have been
	//    initialized before calling this function 

    // 1. Create a new asymmetric cryptographic object R_CR 
    if ((ret = R_CR_new(ctx, R_CR_TYPE_DIGEST, R_CR_ID_SHA1,
        R_CR_SUB_NONE, &dec_obj)) != R_ERROR_NONE)
    {
        //Unable to create crypto object
        return ERR_CRYPTOCME_CRYPTO_OBJECT;
    }

    // 2. Initialize the digest object 
    if ((ret = R_CR_digest_init(dec_obj)) != R_ERROR_NONE)
    {
        // Unable to initialize crypto object
		R_CR_free(dec_obj);
        return ERR_CRYPTOCME_INIT_OBJ_W_KEY;
    }

    // 5. Digest the data. 
    // - There is only 1 message so only a single digest update call 
    // needs to be made. If the message were to consist of mulitiple 
    // parts then mulitiple calls to R_CR_digest_update can be made. 
    if ((ret = R_CR_digest(dec_obj, data, dlen, buf, mlen)) !=
        R_ERROR_NONE)
    {
        // Unable to calculate a hash number
		R_CR_free(dec_obj);
        return ERR_CRYPTOCME_HASH_FAILED;
    }

	
	// Free resources
	R_CR_free(dec_obj);
	return(ret);
}

//---------------------------------------------------------------------
// -----------   R A N D O M   N U M B E R   --------------------------
//---------------------------------------------------------------------
int CryptoControlImpl::rand_cryptome (unsigned int uiDataLen,
									  CryptoOperationEventImpl *opEvent)
{
	int res;
	if (LibStatus == ERR_CRYPTOCME_LIB_NOT_INITIALIZED)
	{
		if ((res = InitCriptoCME()) != R_ERROR_NONE)
		{
			// Failed to initialize CRYPTO C ME library
			return opEvent->setError(res, CRYPTO_EXECUTION);
		}
	}

	// Allocate a buffer to hold a random number 
	byte *buf = new byte [uiDataLen];
	if (!buf)
		return opEvent->setError(ERR_MEMORY_ALLOC, CRYPTO_EXECUTION);

	unsigned int len = uiDataLen;
    if ((res = rand_data(uiDataLen, buf, &len)) != R_ERROR_NONE)
    {
		// Failed to generate a random number
		delete [] buf;
        return opEvent->setError(res, CRYPTO_EXECUTION);
    }

	// set OperationEvent object with a result of decryption
	res = opEvent->setByteString (buf, len);
	delete [] buf;
	return res;
}

int CryptoControlImpl::rand_data(unsigned int dlen,
								 byte *buf,  unsigned int *mlen)
{
    int  ret         = R_ERROR_NONE;
    R_CR *dec_obj    = NULL;

    // 1. Create a new cryptographic object R_CR 
    if ((ret = R_CR_new(ctx, R_CR_TYPE_RANDOM, R_CR_ID_RANDOM,
        R_CR_SUB_NONE, &dec_obj)) != R_ERROR_NONE)
    {
        //Unable to create crypto object
        return ERR_CRYPTOCME_CRYPTO_OBJECT;
    }

    // 2. Generate a random seed using a standard library
	srand( (unsigned)time( NULL ) );
	int iSeed = rand();
	
	// 3. Convert an Integer seed to a bynary seed
	BYTE bSeed[4];
	for (int i = 3; i >= 0; i--)
	{
		bSeed[i] = (BYTE)(iSeed & 0x000000ff);
		iSeed >>= 8;
	}

	// 4. Seed the random object 
    if ((ret = R_CR_random_seed(dec_obj, bSeed, 4)) != R_ERROR_NONE)
    {
        // Unable to obtain a seed 
		R_CR_free(dec_obj);
        return ERR_CRYPTOCME_RAND_FAILED;
    }

	// 5. Generate the random data
    if ((ret = R_CR_random_bytes(dec_obj, dlen, buf, mlen)) != R_ERROR_NONE)
    {
        // Unable to generate a random number 
		R_CR_free(dec_obj);
        return ERR_CRYPTOCME_RAND_FAILED;
    }

	// Free resources
	R_CR_free(dec_obj);
	return(ret);
}



// --------------------------------------------------------------------
// --------------------     Utility functions   -----------------------
// --------------------------------------------------------------------


char* CryptoControlImpl::hex2ascii(const byte *bRID, unsigned int ridLen)
{
	char *pTemp = new char [ridLen * 2 + 1];
	if (!pTemp)
		return NULL;

	unsigned int i, j;
	for (i = 0, j = 0; i < ridLen; i++)
	{
		if ((bRID[i] & 0xf0) != 0)
			break;
		if ((bRID[i] & 0x0f) != 0)
		{
			pTemp[j++] = hexChar2AsciiChar(bRID[i++] & 0x0f);
			break;
		}
	}
			
	for (; i < ridLen; i++)
	{
		pTemp[j++] = hexChar2AsciiChar((bRID[i] >> 4) & 0x0f);
		pTemp[j++] = hexChar2AsciiChar(bRID[i] & 0x0f);
	}
	pTemp[j] = '\0';
	return pTemp;
}

char CryptoControlImpl::hexChar2AsciiChar(byte btVal)
{
	if ((char)btVal >= 0x00 && (char)btVal <= 0x09)
		return btVal + '0';
	else // (btVal >= 0x0A && btVal <= 0x0F)
		return btVal + 'A' - 10;
}

int CryptoControlImpl::setKeyFromRegistry(const char *pRID, const char *pPKI, 
										  unsigned int *modulus)
{
	char key[16];
	byte *pKey;
	int key_len;
	int res;
	
	strcpy(key, "Key");
	strncpy(key+3, pRID, 10);
	strncpy(key+13, pPKI, 3);    // including trailning 0x00

	// Extract Encoded key from the Registry
	res = getValFromReg("", key,  &pKey, &key_len);
	if (res != SUCCESS)
	{
		return res;
	}

	byte Key_hex_val[1024];
	byte *pKey_hex_val = Key_hex_val;
	int Key_hex_len;
	res = AsciiStrWithSpace2HexByte((const char*)pKey, key_len, pKey_hex_val, &Key_hex_len);
	delete [] pKey;
	
		
	// Check if the read value is in ASN.1's RSAPublicKey format
	RSAPublicKey pubKeyObj;
	res = pubKeyObj.setKey(pKey_hex_val, Key_hex_len);
	if (res != KEY_SUCCESS)
	{
		return ERR_CRYPT_CA_KEY_UNRECOGNIZED_FORMAT;
	}
	
	int CryptAlg;
	CryptAlg = 0x1;		// book 2 Chapter 5.1: Keys and Ceritifcates


	int HashAlg;
	HashAlg = 0x1;		// book 2 Chapter 5.1: Keys and Ceritifcates


	pubKeyObj.getKey (&pKey);
	res = setPubKey(pKey, pubKeyObj.getKeyLength ());
	if (res != SUCCESS)
	{
		// Handle Error
		return res;
	}

	setCryptoAlg (CryptAlg);
	setHashAlg (HashAlg);
	*modulus = pubKeyObj.getModulusLength ();
	
			
	/*
	pubKeyObj.getKey (&buff);
	res = setPubKey(buff, pubKeyObj.getKeyLength ());
	if (res != SUCCESS)
	{
		// Handle Error
		return res;
	}

	setCryptoAlg (CryptAlg);
	setHashAlg (HashAlg);
	*modulus = pubKeyObj.getModulusLength ();
	*/		
	
	
/*xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
	HKEY hKey = 0;
	HKEY hRID = 0;
	HKEY hPKI = 0;

	long res;

	// Open Crypto Root registry key
	res = RegOpenKeyEx(HKEY_LOCAL_MACHINE, CRYPTO_REG_KEY, 0, KEY_READ, &hKey);
	if (res != ERROR_SUCCESS)
	{
		return ERR_CRYPT_ROOT_OPEN_FAILED;
	}

	// Open key for the specified RID
	res = RegOpenKeyEx(hKey, pRID, 0, KEY_READ, &hRID);
	if (res != ERROR_SUCCESS)
	{
		closeRegKeys(hKey, 0,0);
		return ERR_CRYPT_RID_KEY_OPEN_FAILED;
	}

	// Open a key for the specified PKI (Public Key Index)
	res = RegOpenKeyEx(hRID, pPKI, 0, KEY_READ, &hPKI);
	if (res != ERROR_SUCCESS)
	{
		closeRegKeys(hKey, hRID, 0);
		return ERR_CRYPT_PKI_KEY_OPEN_FAILED;
	}

	// Read key value from the registry
	byte *buff = NULL;
	DWORD size;
	res = GetRegistryBinValue (hPKI, "Key", REG_BINARY, &buff, &size);
	if (res != ERROR_SUCCESS)
	{
		closeRegKeys(hKey, hRID, hPKI);
		return ERR_CRYPT_KEY_READ_FAILED;
	}

	// Check if the read value is in ASN.1's RSAPublicKey format
	RSAPublicKey pubKeyObj;
	res = pubKeyObj.setKey(buff, size);
	delete [] buff;
	if (res != KEY_SUCCESS)
	{
		closeRegKeys(hKey, hRID, hPKI);
		return ERR_CRYPT_CA_KEY_UNRECOGNIZED_FORMAT;
	}

	// Read crypto algorithm id
	int CryptAlg;
	DWORD dwSize = sizeof (CryptAlg);
	DWORD dwType;
	res = RegQueryValueEx(hPKI, "CryptoAlgorithm", 0, &dwType, 
						 (byte*)(&CryptAlg), &dwSize);
	if (res != ERROR_SUCCESS || dwType != REG_DWORD) 
	{
		closeRegKeys(hKey, hRID, hPKI);
		if (res == ERROR_SUCCESS)
			return ERR_CRYPT_UNEXPECTED_REG_TYPE;
		else
			return ERR_CRYPT_CRYPTOALG_READ_FAILED;
	}

	// Read hash algorithm id
	int HashAlg;
	dwSize = sizeof (HashAlg);
	res = RegQueryValueEx(hPKI, "HashAlgorithm", 0, &dwType, 
						(byte*)(&HashAlg), &dwSize);
	if (res != ERROR_SUCCESS || dwType != REG_DWORD)
	{
		closeRegKeys(hKey, hRID, hPKI);
		if (res == ERROR_SUCCESS )
			return ERR_CRYPT_UNEXPECTED_REG_TYPE;
		else
			return ERR_CRYPT_HASHALG_READ_FAILED;
	}
	
	closeRegKeys(hKey, hRID, hPKI);
	pubKeyObj.getKey (&buff);
	res = setPubKey(buff, pubKeyObj.getKeyLength ());
	if (res != SUCCESS)
	{
		// Handle Error
		return res;
	}

	setCryptoAlg (CryptAlg);
	setHashAlg (HashAlg);
	*modulus = pubKeyObj.getModulusLength ();
*/
	return SUCCESS;
}



int CryptoControlImpl::GetRegistryBinValue (HKEY hKey, 
										const char *subKey, 
										DWORD dwType, 
										BYTE **ppbBuff, 
										DWORD *dwSize)
{
/*xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
	BYTE *btData;
	DWORD dwDataSize;
	DWORD dwRegType;
	long res;

	// Read key into the pKey
	// First, find the size of the buffer to allocate:
	res = RegQueryValueEx(
		hKey,            // handle to key
		subKey,  // value name
		0,      // reserved
		&dwRegType,       // type buffer
		NULL,        // data buffer
		&dwDataSize      // size of data buffer
		);
	if (res != ERROR_SUCCESS)
	{
		return res;
	}
	if (dwRegType != dwType)
	{
		return ERR_CRYPT_UNEXPECTED_REG_TYPE;
	}

	// allocate space for the data
	btData = new BYTE [dwDataSize];
	if (!btData)
		return ERR_MEMORY_ALLOC;

	res = RegQueryValueEx(
		hKey,            // handle to key
		subKey,  // value name
		0,      // reserved
		&dwRegType,       // type buffer
		btData,        // data buffer
		&dwDataSize      // size of data buffer
		);
	if (res != ERROR_SUCCESS)
	{
		// Handle error
		delete [] btData;	
		return ERR_CRYPT_READING_DATA_FROM_REG;
	}
	*ppbBuff = btData;
	*dwSize = dwDataSize;
*/
	//return ERROR_SUCCESS;  -- NEDIR BU?
	return 666;
}

void CryptoControlImpl::closeRegKeys(HKEY hKey, HKEY hRID, HKEY hPKI)
{
	if (hKey != 0)
		//RegCloseKey(hKey);
	if (hRID != 0)
		//RegCloseKey (hRID);
	if (hPKI != 0)
		//RegCloseKey (hPKI);
	hKey = 0;
	hRID = 0;
	hPKI = 0;
}

void CryptoControlImpl::clean()
{
	if (pubKeyArr)
	{
		delete [] pubKeyArr;
		pubKeyArr = 0;
	}
	pubKeyLength = 0;
	uiCryptoAlgID = 0;
	uiHashAlgID = 0;
}

int CryptoControlImpl::setPubKey (const byte *key, unsigned int szKey)
{
	if (pubKeyArr)
		delete [] pubKeyArr;
	pubKeyArr = new byte [szKey];
	if (!pubKeyArr)
	{
		return ERR_MEMORY_ALLOC;
	}
	for (unsigned int i = 0; i < szKey; i++)
		pubKeyArr[i] = key[i];
	pubKeyLength = szKey;
	return SUCCESS;
}

void CryptoControlImpl::setCryptoAlg (unsigned int id)
{
	this->uiCryptoAlgID = id;
}

void CryptoControlImpl::setHashAlg (unsigned int id)
{
	this->uiHashAlgID = id;
}

int CryptoControlImpl::checkCAIntegrity(int *keysVerified)
{
	*keysVerified = 0;
	long res = checkInitialization (false);
	if (res != SUCCESS)
		return res;

	// Open 'Crypto' Registry key
	CnfgControlImpl cnfg;
	CnfgOperationEventImpl cnfgOpEvnt;
/*	
	AccessManager *pAM = getAccessManager();
	if (!pAM)
		return ERR_AM_NOT_INITIALIZED;

	if ((res = pAM->open (&cnfg)) != SUCCESS)
		return res;
*/
	if ((res = cnfg.addOperationEvent (&cnfgOpEvnt)) != SUCCESS)
	{
		return res;
	}
	
	//res = cnfg.enumKeys (CNFG_TERMINAL, "Crypto");
	res = cnfg.enumKeys (CNFG_TERMINAL, "Key");
	
	cnfg.removeEvent ();
	if (res != SUCCESS)
	{
		res = cnfgOpEvnt.getError ();
		return res;
	}
	
	if (cnfgOpEvnt.getValueType () != CNFG_ENUM_KEYS)
	{
		return ERR_UNEXPECTED_TYPE;
	}

	int num_skeys = cnfgOpEvnt.getLength();
	char **ppRIDs = 0;
	cnfgOpEvnt.getStringArray (&ppRIDs);
	
	// for Each RID
	int indxs;
	int total_indxs = 0;
	for (int i = 0; i < num_skeys; i++)
	{
		//printf ("%s\n", ppRIDs[i]);
		indxs = 0;
		res = verifyRID(ppRIDs[i], &indxs);

		total_indxs += indxs;
		if (res != SUCCESS)
			break;
	}
	
	*keysVerified = total_indxs;
	return res;
}

int CryptoControlImpl::verifyRID(const char *pRID,  int *indxs)
{
	int res = SUCCESS;

	// Check if the pRID ponints to a valid RID
	*indxs = 1;
	byte rid[5];
	char realRID[10+1];  // Registered Application Provider Identificator. Like: A000000003
			     // The value passwed to us is like: KeyA00000000370
	
	strncpy(realRID, pRID + 3, 10);
	realRID[10]= '\0';
	if (!checkRID(realRID, rid))
		return SUCCESS;

	// Enumerate Sub Keys
	CnfgControlImpl cnfg;
	CnfgOperationEventImpl opEvent;
	
	if ((res = cnfg.addOperationEvent (&opEvent)) != SUCCESS)
		return res;
	res = cnfg.enumKeys (CNFG_TERMINAL, pRID);
	
	cnfg.removeEvent ();

	int num = opEvent.getLength ();
	char **indxArr = 0;
	opEvent.getStringArray (&indxArr);
	if (!indxArr)
		return SUCCESS;
	int i = 0;
	for (; i < num; i++)
	{
		//printf ("The following index is read: %s\n", indxArr[i]);
 		if ((res = VerifyKey(indxArr[i], realRID, rid)) != SUCCESS)
			break;
	}
	*indxs = i;
	
	return res;
}

int CryptoControlImpl::VerifyKey(const char *ascIndx, 
								 const char *ascRID, 
								 const byte rid[]
								 )
{
	int res;
	byte *pKey;
	byte *pChkSum;
	byte *pIndx;
	byte *tmp;
	int key_len;
	int chksum_len;
	int indx_len;
	char CheckSum[100];

	// Create a path to the current index
	strcpy(CheckSum, "CheckSum");
	strncat(CheckSum, ascIndx + 3, strlen(ascIndx) - 2);
	CheckSum[8+strlen(ascIndx)-2] = 0;
	char *path = CreateKeyPath (ascRID, ascIndx);
	if (!path)
		return ERR_MEMORY_ALLOC;

	// Extract Encoded key from the Registry
	res = getValFromReg(path, ascIndx,  &pKey, &key_len);
	if (res != SUCCESS)
	{
		delete path;
		return res;
	}

	// Extract a Check Sum from the Registry
	res = getValFromReg(path, CheckSum, &pChkSum, &chksum_len);
	if (res != SUCCESS)
	{
		delete path;
		delete [] pKey;
		return res;
	}
	// Verify that the length of the the CheckSum is equal to 
	// R_CR_DIGEST_SHA1_LEN (which is equal to 20 bytes)
	byte Checksum_hex_val[1024];
	byte *pChecksum_hex_val = Checksum_hex_val;
	int Checksum_hex_len = R_CR_DIGEST_SHA1_LEN;
	
	res = AsciiStrWithSpace2HexByte((const char*)pChkSum, chksum_len, pChecksum_hex_val, &Checksum_hex_len);
	if (res != SUCCESS)
	{
		delete path;
		delete [] pKey;
		return res;
	}
	
	if (Checksum_hex_len != R_CR_DIGEST_SHA1_LEN)
	{
		delete [] path;
		delete [] pKey;
		delete [] pChkSum;
		return ERR_CRYPT_CHK_SUM_CORRUPTED;
	}

	// Extract Public Key Index from the Registry
	// the '70' part from KeyA00000000370
	AsciiStrWithSpace2HexByte((const char*)ascIndx + 3 + 10, 2, pIndx, &indx_len);
	
	/*
	res = getValFromReg(path, "9F22",  &pIndx, &indx_len);
	if (res != SUCCESS)
	{
		delete path;
		delete [] pKey;
		delete [] pChkSum;
		return res;
	}
	*/
	// release a buffer pointed to by a 'path' pointer
	delete path;
	

	// Check the Index stored under the tag '9F22', to the index used
	// in the path (they must be the same)
	/*
	char *tmp_indx = HexByte2AsciiStr (pIndx, indx_len);
	if (!tmp_indx)
	{
		delete [] pKey;
		delete [] pChkSum;
		delete [] pIndx;
		return ERR_MEMORY_ALLOC;
	}
	if (strlen(tmp_indx) != strlen(ascIndx) ||
		strcmp(tmp_indx, ascIndx) != 0)
	{
		delete tmp_indx;
		delete [] pKey;
		delete [] pChkSum;
		delete [] pIndx;
		return ERR_CRYPT_CA_INDX_CORRUPTED;
	}
	delete tmp_indx;
	*/
	
	byte Key_hex_val[1024];
	byte *pKey_hex_val = Key_hex_val;
	int Key_hex_len;
	res = AsciiStrWithSpace2HexByte((const char*)pKey, key_len, pKey_hex_val, &Key_hex_len);
	
	
	// Parse encoded Key
	RSAPublicKey rsaKey;
	res = rsaKey.setKey (Key_hex_val, Key_hex_len);
	if (res != KEY_SUCCESS)
	{
		delete [] pKey;
		delete [] pChkSum;
		delete [] pIndx;
		return res;
	}
	
	// Create an input for the hash algorithm
	// (RID + INDX + KEY_MODULUS + KEY_EXPONENT)
	UNINT input_len = 5 + indx_len + rsaKey.getModulusLength() +
						   rsaKey.getExpLength ();
	byte *input = new byte [input_len];
	if (!input)
	{
		delete [] pKey;
		delete [] pChkSum;
		delete [] pIndx;
		return ERR_MEMORY_ALLOC;
	}

	memcpy(input, rid, 5);
	memcpy(input + 5, pIndx, indx_len);
	rsaKey.getModulus(&tmp);
	memcpy(input + 5 + indx_len, tmp + (rsaKey.getModulusBufferLength () -
		rsaKey.getModulusLength()), 
		rsaKey.getModulusLength());
	rsaKey.getExponent(&tmp);
	memcpy(input + 5 + indx_len + rsaKey.getModulusLength(),
		tmp, rsaKey.getExpLength ());

	// Release previously allocated buffers
	delete [] pKey;
	delete [] pIndx;

	// Allocate a buffer to hold a Hash number of size R_CR_DIGEST_SHA1_LEN
	byte *buf = new byte [R_CR_DIGEST_SHA1_LEN];
	if (!buf)
	{
		delete [] pChkSum;
		delete [] input;
		return ERR_MEMORY_ALLOC;
	}	

	bool localInit = false;
	if (LibStatus == ERR_CRYPTOCME_LIB_NOT_INITIALIZED)
	{
		if ((res = InitCriptoCME()) != R_ERROR_NONE)
		{
			// Failed to initialize CRYPTO C ME library
			delete [] pChkSum;
			delete [] input;
			delete [] buf;
			return res;
		}
		localInit = true;
	}
	// Calculate a Hash number
	unsigned int len = R_CR_DIGEST_SHA1_LEN;
	//printf("Input:\n");
	//printHex(input,input_len);
	//CryptoPP::HashWordType x;
/*    	CryptoPP::SHA sha;
	sha.InitState( NULL);
    	sha.Transform((CryptoPP::word32 *)buf, (const CryptoPP::word32 *)input);*/

	
	/*
	CRYPT_CONTEXT sigKeyContext, hashContext;
	void *signature;
	int signatureLength;
	
	// Create a hash context 
	cryptCreateContext( &hashContext, CRYPT_UNUSED, CRYPT_ALGO_SHA );

	// Hash the data 
	printf("Input:\n");
	printHex(input,input_len );
	res = cryptEncrypt( hashContext, input, input_len );
	printf("Input:\n");
	printHex(input,input_len );
	res = cryptEncrypt( hashContext, input, 0 );
	printf("Input:\n");
	printHex(input,input_len );
	
	// Allocate memory for the signature 
	
	// Sign the hash to create a signature blob 
	cryptCreateSignature( buf, R_CR_DIGEST_SHA1_LEN, &signatureLength,
		sigKeyContext, hashContext );
	cryptDestroyContext( hashContext );
	
	
	*/
	/*
	CRYPT_ENVELOPE cryptEnvelope;
	CRYPT_CONTEXT cryptContext;
	int bytesCopied;
	
	cryptCreateEnvelope( &cryptEnvelope, CRYPT_UNUSED, CRYPT_FORMAT_CRYPTLIB);
	cryptCreateContext( &cryptContext, CRYPT_UNUSED, CRYPT_ALGO_SHA );
	cryptSetAttribute( cryptEnvelope, CRYPT_ENVINFO_KEY, cryptContext );
	cryptDestroyContext( cryptContext );
	
	printf("Original Hash (from cryptolib):\n");
	printHex(input,input_len);
	cryptSetAttribute( cryptEnvelope, CRYPT_ENVINFO_DATASIZE, input_len );
	cryptPushData( cryptEnvelope, input, input_len, &bytesCopied );
	cryptFlushData( cryptEnvelope );
	cryptPopData( cryptEnvelope, buf, len, &bytesCopied );
	printf("RSA Hash (from cryptolib):\n");
	printHex(buf,len);
	*/
	
	CRYPT_CONTEXT cryptContext;
	int hlen;
	
	cryptCreateContext( &cryptContext, CRYPT_UNUSED, CRYPT_ALGO_SHA );

	res = cryptEncrypt(cryptContext, input, input_len);
	res = cryptEncrypt(cryptContext, input, 0);
	
	cryptGetAttributeString( cryptContext, CRYPT_CTXINFO_HASHVALUE, buf, &hlen );
	//printf("RSA Hash (from cryptolib):\n");
	//printHex(buf,hlen);

	cryptDestroyContext( cryptContext );
	
		
	//CryptoPP::SHA().CalculateDigest(buf, input, input_len);
	//printf("RSA Hash:\n");
	//printHex(buf,len);
	if (memcmp(buf, pChecksum_hex_val, R_CR_DIGEST_SHA1_LEN) == 0)
		res = SUCCESS;
	else
		res = ERR_CRYPT_KEY_CORRUPTED;
    /*if ((res = hash_data(input, input_len, buf, &len)) == R_ERROR_NONE)
    {
		printf("RSA Hash:\n");
		printHex(buf,len);
		// Verify that the caclulated hash is equal to the CheckSum read from
		// the registry
		if (memcmp(buf, pChecksum_hex_val, R_CR_DIGEST_SHA1_LEN) == 0)
			res = SUCCESS;
		else
			res = ERR_CRYPT_KEY_CORRUPTED;
    }*/

	if (localInit)
	{
		// Crypto CME library was locally initialized, therefore 
		// release it
		releaseCryptoCME();
	}
	delete [] buf;
	delete [] input;
	delete [] pChkSum;
	
	return res;
}

int CryptoControlImpl::getValFromReg (const char *path, 
									  const char* val_name, 
									  byte **pVal, int *val_len)
{
	CnfgOperationEventImpl opEvent;
	*pVal = 0;
	*val_len = 0;
	int res;

	CnfgControlImpl cnfg;
	if ((res = cnfg.addOperationEvent (&opEvent)) != SUCCESS)
		return res;

	res = cnfg.getValue (CNFG_TERMINAL, val_name, path);
	cnfg.removeEvent ();

	
	if (opEvent.getValueType () != CNFG_BINARY)
		return ERR_UNEXPECTED_TYPE;

	int len = opEvent.getLength ();
	if (len <= 0)
		return ERR_INVALID_TERMINAL_DATA;

	byte *val;
	opEvent.getByteString (&val);
	byte *temp = new byte [len];
	if (!temp)
		return ERR_MEMORY_ALLOC;

	memcpy (temp, val, len);
	*pVal = temp;
	*val_len = len;
	return SUCCESS;
}

char* CryptoControlImpl::CreateKeyPath (const char* ascRID, 
										const char* ascIndx)
{
	char *path;
	int rid_len = (int)strlen(ascRID);
	int indx_len = (int)strlen(ascIndx);
	int crypt_len = 6; // = strlen ("Crypto")

	// Create a path to the key
	path = new char [crypt_len + rid_len +  indx_len + 3];
	if (!path)
		return NULL;

	sprintf (path, "Crypto\\%s\\%s", ascRID, ascIndx);
	return path;
}

bool CryptoControlImpl::checkRID(const char *pRID, byte rid[])
{
	if (!pRID)
		return false;
	int len = (int)strlen(pRID);
	if (len != 10)
		return false;
	
	int k = 4;

	for (int i = len - 1; i >= 0, k >= 0; i--, k--)
	{
		if (pRID[i] >= '0' && pRID[i] <= '9')
			rid[k] = pRID[i] - '0';
		else if (pRID[i] >= 'A' && pRID[i] <= 'F')
			rid[k] = pRID[i] - 'A' + 10;
		else if (pRID[i] >= 'a' && pRID[i] <= 'f')
			rid[k] = pRID[i] - 'a' + 10;
		else
			return false;

		
		i--;
		byte tmp = 0x00;

		if (pRID[i] >= '0' && pRID[i] <= '9')
			tmp = (pRID[i] - '0');
		else if (pRID[i] >= 'A' && pRID[i] <= 'F')
			tmp = (pRID[i] - 'A' + 10);
		else if (pRID[i] >= 'a' && pRID[i] <= 'f')
			tmp = (pRID[i] - 'a' + 10);
		else
			return false;
		rid[k] |= (tmp << 4);
	}
	return true;
}


void memrev(unsigned char *indata, unsigned char *outdata, int inlen)
{
	int i;

	for (i=0;i<inlen;i++)
		outdata[i] = indata[inlen-i-1];
}



char * strrev(char * string)
{
	char *p;
	int len,i;
	short sRet;

	len = strlen(string);

        /*    sRet = sMemAlloc(len, (char **)&p);
  MGG*/
            //p=(unsigned char *)malloc(len);
            p=(char *)malloc(len);
	for (i=0;i<len;i++)
	{
		p[len-i-1] = string[i];
	}
	p[len] = 0;
	strcpy(string, p);
	return string;
}


#define isEven(x) ((x & 0x01)==0)
#define isOdd(x)  (x & 0x01)

int FindModulus( unsigned char *, unsigned char *, unsigned  char * );
void MD5TestSuite( void );
void MDFile (char *);
void MDTimeTrial( void );

unsigned char pucVersion[]="0.01";


int FastExpMP( unsigned char *pucX, unsigned char *pucY,

			   unsigned char *pucN, unsigned char *pucResult ) {

	static unsigned char pucTemp[XDIGIT];
	int m,n;

	if (mpCompare(pucY,pucOne)==0) {
            mpMod( pucX, pucN, pucResult );
            return 0;
	}

	if ((pucY[0]&1)==0) {
		mpDivShort( pucY,2,pucDum1,pucDum2 );
		FastExpMP( pucX,pucDum1,pucN,pucTemp );
		mpMul( pucTemp,pucTemp,pucDum2 );
		mpMod( pucDum2,pucN,pucResult );
		return(0);
	}
	else {
		mpSubShort( pucY,1,pucDum1 );
		mpDivShort( pucDum1,2,pucDum1,pucDum2 );
		FastExpMP( pucX,pucDum1,pucN,pucTemp );
		mpMul( pucTemp,pucTemp,pucDum2 );
		mpMod( pucDum2,pucN,pucTemp);
		mpMul( pucTemp,pucX,pucDum2 );
		mpMod( pucDum2,pucN,pucResult);
		return(0);
	}
}


void burnfree( unsigned char *pucA, int nSize ) {
// Safe Free, zero memory block before freeing....
	memset( pucA, 0, nSize );
	free ((char *)pucA);
}

void swap( unsigned char *pucX, unsigned char *pucY ) {
	int i;
	unsigned char ucTemp;

	for (i=0; i<IDIGIT; i++) {
		ucTemp=pucX[i];
		pucX[i]=pucY[i];
		pucY[i]=ucTemp;
	}
    ucTemp=0;
}


/*char *fgets(char * pReadData, int iMaxLen, HFILE hFile)
{
	int i=0;
	ulong ReadByte;

	do
	{
		sFileRead (hFile, &pReadData[i], 2, &ReadByte);
		i+=2;
	}while (pReadData[i] != 0x0D);
	return pReadData;
}
*/

uchar RSAEncrypt(unsigned char ucPubKeyType, uchar PubKeyLen, unsigned char *exp, int expLen, unsigned char *mesaj, uchar mesajLen)
{
	static unsigned char pucM[IDIGIT], pucMR[IDIGIT], pucNR[IDIGIT];
	static unsigned char pucN[IDIGIT],  pucR[IDIGIT], pucRR[IDIGIT];
	static unsigned char pucE[IDIGIT];
        short sRet;
	int m,n;

	memset( pucOne,0,XDIGIT );
	pucOne[0]=1;

/*vPrintMsg("ucPubKeyType", &ucPubKeyType, 1);
vPrintMsg("PubKeyLen", &PubKeyLen, 1);
vPrintMsg("exp", exp, 3);
vPrintMsg("mesaj", mesaj, mesajLen);*/
	memset(pucN,0,sizeof(pucN));
	switch (ucPubKeyType)
	{
		case CA_PUBLIC_KEY_MOD:
			memcpy(pucN, ucCAPubKeyMod, PubKeyLen);
			break;
		case ICC_PUBLIC_KEY_MOD:
			memcpy(pucN, ucICCPubKeyMod, PubKeyLen);
			break;
		case ISS_PUBLIC_KEY_MOD:
			memcpy(pucN, ucIssPubKeyMod, PubKeyLen);
			break;
		case ICC_PIN_ENC_PUBLIC_KEY_MOD:
			memcpy(pucN, ucICCPINEncPubKeyMod, PubKeyLen);
			break;
	}

//vPrintMsg("pucN", pucN, PubKeyLen);
	memset(pucE,0,sizeof(pucE));
	memcpy(pucE, exp, expLen);
	memset(pucM,0,sizeof(pucM));
	memcpy(pucM, mesaj, mesajLen);
	memset(pucNR,0,sizeof(pucNR));
	memset(pucMR,0,sizeof(pucMR));
	memset(pucRR,0,sizeof(pucRR));
	memrev(pucM, pucMR, mesajLen);
	memrev(pucN, pucNR, PubKeyLen);
	FastExpMP( pucMR,pucE,pucNR,pucR );
	memrev(pucR, pucRR, PubKeyLen);

	memcpy(ucRecoveredData, pucRR, PubKeyLen);
//vPrintMsg("ucRecoveredData", ucRecoveredData, PubKeyLen);

	return TRUE;

}

/*
int RSADecrypt(void)

{
	static unsigned char *pucS, *pucM;
	static unsigned char *pucN,  *pucR;
	static unsigned char *pucE;
	static unsigned char pucFii[1024];
	SHORT sRet;
	CHAR str[1024];
     HFILE fpi,fpo;
	int i,j,nBlockSize,nModulusSize;

//	printf("Verisoft RSA Library %s (c) RSA Decryption Executing.\n",pucVersion );
	memset( pucOne,0,XDIGIT );
	pucOne[0]=1;
	memset(pucDum1,0,XDIGIT);
	memset(pucDum2,0,XDIGIT);


	sRet = sMemAlloc(ADIGIT, (char **)&pucS);
	if (sRet != ERR_NONE)
		return FALSE;

	sRet = sMemAlloc(XDIGIT, (char **)&pucM);
	if (sRet != ERR_NONE)
		return FALSE;

	sRet = sMemAlloc(XDIGIT, (char **)&pucN);
	if (sRet != ERR_NONE)
		return FALSE;

	sRet = sMemAlloc(XDIGIT, (char **)&pucE);
	if (sRet != ERR_NONE)
		return FALSE;

	sRet = sMemAlloc(XDIGIT, (char **)&pucR);
	if (sRet != ERR_NONE)
		return FALSE;


	if (sFileOpen ("private.key", FILE_READ, &fpi) == ERR_NONE)
	{
	// Read key values
//		printf("Reading Private Keys\n");

		fgets((char *)pucS,ADIGIT,fpi);
		PackASCIINumber((char *)pucS);
		nBlockSize=atoi((char *)pucS);

		fgets((char *)pucS,ADIGIT,fpi);
		PackASCIINumber((char *)pucS);
		ConvertASCIItoMP( pucN, pucS );
		nModulusSize=strlen((char *)pucS );

//		printf("Modulus Size:%d decimal digits.\n",nModulusSize);

		fgets((char *)pucS,ADIGIT,fpi); // ignore
		fgets((char *)pucS,ADIGIT,fpi);
		PackASCIINumber((char *)pucS);

		ConvertASCIItoMP( pucE, pucS );
//		printf("Public Key succesfully read.\n");
		sFileClose (fpi);
	}

	// Run the RSA procedure M^e mod n
if (sFileOpen ("gmesaj.txt", FILE_READ, &fpi) == ERR_NONE)
{
//	printf("Input file opened.\n");
	if (sFileOpen ("amesaj.txt", FILE_WRITE, &fpo) == ERR_NONE)
	{
//		printf("Output file opened.\n");
//		MDFile("gmesaj.txt");
			while ( fgets( (char *)pucFii, sizeof(pucFii), fpi )!=NULL )
				for ( i=0; i<(int)strlen((char *)pucFii); i+=nModulusSize ) {
					memset(pucDum1,0,sizeof(pucDum1));
					strncpy((char *)pucDum1,(char *)pucFii+i,nModulusSize);
					if (!isdigit(pucDum1[0])) continue;
//					printf("Encrypted Block:\n[%s]\nDecrytpted Block:\n",pucDum1);
					ConvertASCIItoMP( pucM,pucDum1);
					FastExpMP( pucM,pucE,pucN,pucR );
					ConvertMPtoASCII( pucS,pucR );
					for (j=strlen((char *)pucS); j<nBlockSize; j++ )
						sFileWrite (fpo, "0",1);
					sprintf(str,"%s",pucS);
					sFileWrite (fpo, str, strlen(str));
//					printf("[%s]\n",pucS);
				}
			sFileClose (fpo);
			sFileClose (fpi);

//    MDFile("amesaj.txt");
//	printf("Verisoft RSA Library %s (c) RSA Decryption Done.\n",pucVersion );
	}
}

// Make sure nothing remains in memory
	burnfree(pucS ,ADIGIT);
	burnfree(pucN ,XDIGIT);
	burnfree(pucE ,XDIGIT);
	burnfree(pucM ,XDIGIT);
	burnfree(pucR ,XDIGIT);
	return TRUE;
}
*/
/*
**  test Mersenne numbers for primality using the
**  optimized Lucas test
*/



int MostSignificantDigit( unsigned char x[],
                          unsigned int  m)
{
  if (m)
  {
    m--;
    while (m && (x[m] == 0)) m--;
  }

  return(m);
}


/*
** mpCopy - copy one multiple precision integer to another
**
** return value: *to = base address of 'to' MP integer
*/

unsigned char *mpCopy( unsigned char to[],
                       unsigned char fr[])
{
  memcpy( to, fr, IDIGIT);
  return( to );
}
/*
** ConvertASCIItoMP -- convert from ASCII number string to
**                     a multiple precision integer
**
** return value: base address of MP integer
*/



int mpAddShort( unsigned char augend[],
                unsigned char addend,
                unsigned char sum[])
{
  int i, m;

  m = MostSignificantDigit(augend, IDIGIT);

  sum[0] = a.x = augend[0] + addend;

  for ( i = 1; i <= m; i++)
    sum[i] = a.x = a.b[1] + augend[i];

  if (i < IDIGIT)
    sum[i] = a.x = a.b[1];

  for ( i++; i < IDIGIT; i++) sum[i] = 0;

  return(a.b[1]);
}



/*
** mpAdd      -- add two multiple precision integers
** mpAddShort -- add a short integer to a multiple precision integer
**
** return value: 0 if ok; not 0 if overflow
*/

int mpAdd( unsigned char augend[],
           unsigned char addend[],
           unsigned char sum[])
{
  int i, m;
  unsigned char t[XDIGIT];              /* temporary result */

  if ((m = MostSignificantDigit(addend, IDIGIT)) == 0)
    return( mpAddShort(augend, addend[0], sum));

  m = MAX(m, MostSignificantDigit(augend, IDIGIT));

  memset(t, 0, IDIGIT);                 /* clear sum */

  for ( i = 0, a.x = 0; i <= m; i++)
    t[i] = a.x = a.b[1] + augend[i] + addend[i];

  if (i < IDIGIT)
    t[i] = a.x = a.b[1];

  mpCopy(sum, t);

  return(a.b[1]);
}


/*
** mpAnd - bitwise AND between two multiple precision integers
**
** return value: always 0
*/

int mpAnd( unsigned char and1[],
           unsigned char and2[],
           unsigned char result[])
{
  int i, m;

  m = MAX(MostSignificantDigit(and1, IDIGIT),
          MostSignificantDigit(and2, IDIGIT));

  for ( i = 0; i <= m; i++)
    result[i] = and1[i] & and2[i];

  if (i < IDIGIT)
    result[i] = 0;

  return(0);
}


/*
** mpCompare -- compare one multiple precision integer to another
**
** return value: 1 if x > y
**               0 if x == y
**              -1 if x < y
*/

int mpCompare( unsigned char x[], unsigned char y[])
{
  int m;
  char str[32];

  m = MAX(MostSignificantDigit(x, IDIGIT),
          MostSignificantDigit(y, IDIGIT));

  while (m >= 0)
  {
    if (x[m] > y[m]) return(1);
    if (x[m] < y[m]) return(-1);
    m--;
  }

  return(0);
}

/*
** mpDiv      -- divide two multiple precision integers
** mpDivShort -- divide a multiple precision integer with a short integer
**
** return value: 0 if ok; 1 if division by zero (overflow)
*/

int mpDiv( unsigned char dividend[],
           unsigned char divisor[],
           unsigned char quotient[],
           unsigned char remainder[])
{
  unsigned char u[XDIGIT], v[XDIGIT], w[XDIGIT], d, q;
  int  j, m, n;
  int  st;                               /* status */
  char str[64];

  if ((n = MostSignificantDigit(divisor, IDIGIT)) == 0)
    return( mpDivShort(dividend, divisor[0], quotient, remainder));

  mpCopy(u, dividend);                  /* dividend - do not destroy */
  mpCopy(v, divisor);                   /* divisor  - do not destroy */

  memset( quotient, 0, IDIGIT);         /* clear quotient */
  memset( remainder, 0, IDIGIT);        /* clear remainder */

  if ((st = mpCompare(u, v)) < 0) {
    mpCopy(remainder, u);
    return(0);
  } else if (st == 0) {
    quotient[0] = 1;
    return(0);
  }

  if ((d = 256L / (v[n] + 1)) > 1)      /* normalize */
  {
    mpMulShort(u, d, u);
    mpMulShort(v, d, v);
  }
  n = MostSignificantDigit(v, IDIGIT);
  m = MostSignificantDigit(u, IDIGIT);

  for (j = m - n; j >= 0; j--)
  {
    q = MIN( (u[m] * 256L + u[m-1]) / v[n], 255);
    do
    {
      mpMulShort(v, q, w);
      mpLeftByteShift(w, j);
      if (st = mpSub(u, w, w)) q--;
    } while (st);
    quotient[j] = q;
    if (mpCompare(w, v) < 0) break;
    mpCopy(u, w);
    m = MostSignificantDigit(u, IDIGIT);
  }

  mpDivShort(w, d, remainder, w);

  return(0);
}


int mpDivShort( unsigned char dividend[],
                unsigned char divisor,
                unsigned char quotient[],
                unsigned char remainder[])
{
  int i,m;
  char str[32];

  if (divisor == 0) return(1);

  m = MostSignificantDigit(dividend, IDIGIT);

  for (i = m, a.x = 0; i >= 0; i--)
  {
    a.b[0] = dividend[i];
    quotient[i] = a.x / divisor;
    a.b[1] = a.x % divisor;
  }
  remainder[0] = a.b[1];

  for ( i = 1; i < IDIGIT; i++)
  {
    if (i > m)
      quotient[i] = 0;
    remainder[i] = 0;
  }

  return(0);
}

int mpMulShort( unsigned char multiplicand[],
                unsigned char multiplier,
                unsigned char product[])
{
  int i, m;

  m = MostSignificantDigit(multiplicand, IDIGIT);

  for ( i = 0, a.x = 0; i <= m; i++)
    product[i] = a.x = a.b[1] + multiplicand[i] * multiplier;


  if (i < IDIGIT)
    product[i] = a.x = a.b[1];

  for ( i++; i < IDIGIT; i++) product[i] = 0;

  return(a.b[1]);
}

/*
** mpLeftByteShift - left byte shift a multiple precision integer
** mpLeftBitShift  - left bit shift a multiple precision integer
**
** input:        x[]    MP integer
**               s      number of BYTES or BITS to shift
**
** return value: base address of MP integer
*/

unsigned char *mpLeftByteShift( unsigned char x[],
                                unsigned int  s)
{
  int i, j;

  if (s > 0)
    for ( i = IDIGIT; i >= 0; i--)
      if ((j = i - s) >= 0)
        x[i] = x[j];
      else
        x[i] = 0;

  return(x);
}


unsigned char *mpLeftBitShift( unsigned char x[],
                               unsigned int  s)
{
  int i, j,  ls, rs;

  ls = s & 7;           /* left shift amount per byte */
  rs = 8 - ls;          /* right shift amount per byte */

  if (s > 0)
    for ( i = IDIGIT; i >= 0; i--)
      if ((j = i - (s / 8)) > 0)
        x[i] = (x[j] << ls) | (x[j-1] >> rs);
      else if (j == 0)
        x[i] = (x[0] << ls);
      else
        x[i] = 0;

  return(x);
}


/*
** mpSub      -- subtract one multiple precision integer from another
** mpSubShort -- subtract a multiple precision integer from a short integer
**
** return value: 0 if ok; not 0 if overflow
*/

int mpSub( unsigned char minuend[],
           unsigned char subtrahend[],
           unsigned char difference[])
{
  int i, m;
  unsigned char t[XDIGIT];              /* temporary result */

  if ((m = MostSignificantDigit(subtrahend, IDIGIT)) == 0)
    return( mpSubShort(minuend, subtrahend[0], difference));

  m = MAX(m, MostSignificantDigit(minuend, IDIGIT));

  memset(t, 0, IDIGIT);                 /* clear difference */

  for ( i = 0, a.x = 256; i <= m; i++)
    t[i] = a.x = 255 + a.b[1] + minuend[i] - subtrahend[i];

  if (i < IDIGIT) t[i] = a.x = 255 + a.b[1];

  mpCopy(difference, t);

  return(a.b[1]-1);
}


int mpSubShort( unsigned char minuend[],
                unsigned char subtrahend,
                unsigned char difference[])
{
  int i, m;

  m = MostSignificantDigit(minuend, IDIGIT);

  difference[0] = a.x = 256 + minuend[0] - subtrahend;

  for ( i = 1; i <= m; i++)
    difference[i] = a.x = 255 + a.b[1] + minuend[i];

  if (i < IDIGIT) difference[i] = a.x = 255 + a.b[1];

  for ( i++; i < IDIGIT; i++)
    difference[i] = 0;

  return(a.b[1]-1);
}


/*
** mpMod     -- find the remainder of two integers, calls Div
** return value: 0 if ok; 1 if division by zero (overflow)
*/

int mpMod( unsigned char dividend[],
           unsigned char divisor[],
           unsigned char remainder[])
{
  unsigned char quotient[XDIGIT];
  return mpDiv( dividend,divisor,quotient,remainder);
}

/*
** mpIsZero -- tests if a multiple precision integer is zero
**
** return value: 1 if x == 0, 0 otherwise
*/

int mpIsZero( unsigned char x[])
{
  int m;

  m = MostSignificantDigit(x, IDIGIT);

  while (m >= 0)
  {
    if (x[m]) return(0);
    m--;
  }

  return(1);
}


/*
** mpMul      -- multiply two multiple precision integers
** mpMulShort -- multiply a multiple precision integer by a short integer
**
** return value: 0 if ok; not 0 if overflow
*/

int mpMul( unsigned char multiplicand[],
           unsigned char multiplier[],
           unsigned char product[])
{
  int i, j, m, n;
  unsigned char t[XDIGIT];              /* temporary result */

  if ((n = MostSignificantDigit(multiplier, IDIGIT)) == 0)
    return( mpMulShort(multiplicand, multiplier[0], product));

  m = MostSignificantDigit(multiplicand, IDIGIT);
  if ((m + n) >= IDIGIT) return(1);

  memset(t, 0, IDIGIT);                 /* clear product */

  for ( i = 0; i <= m; i++)
  {
    for ( j = 0, a.x = 0; j <= n; j++)
      t[i+j] = a.x = a.b[1] + t[i+j] + multiplicand[i] * multiplier[j];
    for ( ; a.b[1] > 0; j++)
      t[i+j] = a.x = a.b[1] + t[i+j];
  }

  mpCopy(product, t);

  return(a.b[1]);
}


/*
** PackASCIINumber -- squeeze all of the non-digits out of a number string
**
** return value: base address of packed string
*/

char *PackASCIINumber( char *str)
{
  char *pi = str;
  char *pj = str;

  while ( *pi ) {
    if ( isdigit(*pi) )
      *pj++ = *pi;
    pi++;
  }
  *pj = '\0';

  return(str);
}


/*
** mpRightByteShift - right byte shift a multiple precision integer
** mpRightBitShift -  right bit shift a multiple precision integer
**
** input:        x[]    MP integer
**               s      number of BYTES or BITS to shift
**
** return value: base address of MP integer
*/

unsigned char *mpRightByteShift( unsigned char x[],
                                 unsigned int  s)
{
  int i, j;

  if (s > 0)
    for ( i = 0; i <= IDIGIT; i++)
      if ((j = i + s) <= IDIGIT)
        x[i] = x[j];
      else
        x[i] = 0;

  return(x);
}

unsigned char *mpRightBitShift( unsigned char x[],
                                unsigned int  s)
{
  int i, j, ls, rs;

  rs = s & 7;           /* right shift amount per byte */
  ls = 8 - rs;          /* left shift amount per byte */

  if (s > 0)
    for (i = 0; i <= IDIGIT; i++)
      if ((j = i + (s >> 3)) < IDIGIT)
        x[i] = (x[j] >> rs) | (x[j+1] << ls);
      else if (j == IDIGIT)
        x[i] = (x[j] >> rs);
      else
        x[i] = 0;

  return(x);
}
/*
** mpScan - read an ASCII number and convert it to a
**          multiple precision integer
**
** return value: base address of input ASCII string
*/

int mpSquareRoot(unsigned char a[],
                 unsigned char q[],
                 unsigned char r[])
{
  unsigned char t[XDIGIT];      /* temp MP integer */

  mpCopy(q, a);

  if (! mpIsZero(a))
  {
    mpDiv(a, q, r, t);
    while (mpCompare(q, r) > 0)
    {
      mpAdd(q, r, q);
      mpDivShort(q, 2, q, r);
      mpDiv(a, q, r, t);
    }
  }

  mpMul(q, q, t);
  mpSub(a, t, r);

  return(0);
}
