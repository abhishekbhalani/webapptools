/*
 * iCrypto.cxx
 * Copyright (C) Andrew Abramov 2007 <stinger@lab18.net>
 *
 * I-Gate is free software. But sources of main part are closed.
 * Plugins for iGate are in open-source domain.
 *
 * You may redistribute it but not modify it under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * main.cc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with main.cc.  If not, write to:
 * 	The Free Software Foundation, Inc.,
 * 	51 Franklin Street, Fifth Floor
 * 	Boston, MA  02110-1301, USA.
 */

#include "Crypto.h"
#if defined(__WXMSW__)
    #include <Windows.h>
#else
    #include <sys/utsname.h>
    #include <unistd.h>

    typedef wxChar  TCHAR;
#endif
#include "Sha1.hpp"

#ifdef MSVC
  #define ASM __asm
  #define INLINE __forceinline
#endif

#ifdef GCC
  #define ASM __asm__
  #define INLINE inline
#endif

#ifdef INTEL
  #define ASM __asm
  #define INLINE inline
#endif

INLINE byte GetCode(int offset, byte *key)
{
    int     bytes;
    word    res;

    bytes = offset >> 3;
    offset &= 7;
    while (bytes >= KEY_BYTES) {
        bytes -= KEY_BYTES;
    }
    res = key[bytes++];
    if (bytes >= KEY_BYTES) {
        bytes -= KEY_BYTES;
    }
    res |= key[bytes++] << 8;
    res >>= offset;

    return (byte)(res & 0xFF);
}

void EncodeData(byte* from, byte* to, int size, byte *key)
{
    int         idx, cIdx, kLen;
    byte        ci;

    cIdx = size;
    kLen = (sizeof(key) * 8) - 1;
    while(cIdx >= KEY_BITS) {
        cIdx = (cIdx & (KEY_BITS - 1)) ^ (cIdx >> KEY_MASK);
    }

    for(idx = 0; idx < size; idx++) {
        ci = GetCode(cIdx, key);
        to[idx] = from[idx] ^ ci;
        cIdx = ci + idx;
    }
}

CEasyCrypto::CEasyCrypto()
{
	cryptoBuff = NULL;
	binHexBuff = NULL;
	cryptoBuffSize = 0;
	binHexBuffSize = 0;
}

CEasyCrypto::~CEasyCrypto()
{
	if (cryptoBuff) {
		delete cryptoBuff;
	}
	if (binHexBuff) {
		delete binHexBuff;
	}
	cryptoBuffSize = 0;
	binHexBuffSize = 0;
}

void CEasyCrypto::SetPassword()
{
	BYTE hash[SHA1_DIGEST];
    DWORD uReturn = MAX_PATH;
    byte oemText[MAX_PATH * sizeof(TCHAR)];
	int i;

#if defined(__WXMSW__)
    HKEY rKey;

    if(RegOpenKey(HKEY_LOCAL_MACHINE, TEXT("SYSTEM\\CurrentControlSet\\Control\\IDConfigDB\\Hardware Profiles\\0001"),&rKey) == ERROR_SUCCESS) {
        RegQueryValueEx(rKey, TEXT("HwProfileGuid"), NULL, NULL, (LPBYTE)(oemText), (LPDWORD)&uReturn);
        RegCloseKey(rKey);
    }
	else {
		GetUserName((TCHAR*)oemText, &uReturn);
	}
#else
	memset(oemText, 0, MAX_PATH);
    *((long*)oemText) = gethostid();
	uname(( struct utsname* )&oemText[4]);
    uReturn = sizeof(struct utsname) + 4;
    char *login = getlogin();
    if (login) {
        strncpy( (char*)&oemText[uReturn], login, (MAX_PATH - uReturn - 1) );
    }
    uReturn = MAX_PATH;
#endif
	// make rollup of the string
	SHA_160(oemText, uReturn, (PDWORD)hash);
	for (i = 0; i < (SHA1_DIGEST / 2); i++) {
		hash[i] += hash[i + (SHA1_DIGEST / 2)];
	}
	for (i = 0; i < (KEY_BYTES / 2); i++) {
		gKeyData[i*2] = ((hash[i] >> 4) & 0x0f)+ '0';
		if(gKeyData[i*2] > '9') {
			gKeyData[i*2] += 7;
		}
		gKeyData[i*2+1] = (hash[i] & 0x0f) + '0';
		if(gKeyData[i*2+1] > '9') {
			gKeyData[i*2+1] += 7;
		}
	}
}


wxString CEasyCrypto::Encrypt(wxString data)
{
	byte *lData = (byte*)strdup(data.ToUTF8().data());
	byte *bRes;
	int len = strlen((const char*)lData);

	bRes = Encrypt(lData, len);
	// convert bRes to BinHex
	AllocBinHex(len*2+2);
	for (int i = 0; i < len; i++) {
		binHexBuff[i*2] = ((bRes[i] >> 4) & 0x0f)+ '0';
		if (binHexBuff[i*2] > '9') {
			binHexBuff[i*2] += 7;
		}
		binHexBuff[i*2+1] = (bRes[i] & 0x0f) + '0';
		if (binHexBuff[i*2+1] > '9') {
			binHexBuff[i*2+1] += 7;
		}
	}
	binHexBuff[len*2] = '\0';
	binHexBuff[len*2+1] = '\0';
	delete lData;
	return wxString::From8BitData((const char*)binHexBuff);
}

wxString CEasyCrypto::Decrypt(wxString data)
{
	byte *lData = (byte*)strdup(data.ToUTF8().data());
	byte *bRes;
	int len = strlen((const char*)lData);

	// convert lData from BinHex
	AllocBinHex(len);
	memset(binHexBuff, 0, len);
	for (int i = 0; i < len; i += 2) {
		byte ch;
		ch = lData[i] - '0';
		if (ch > 9) {
			ch -= 7;
		}
		ch &= 0x0F;
		ch <<= 4;
		binHexBuff[i/2] = ch;
		ch = lData[i+1] - '0';
		if (ch > 9) {
			ch -= 7;
		}
		ch &= 0x0F;
		binHexBuff[i/2] |= ch;
	}
	bRes = Decrypt(binHexBuff, len / 2);
	delete lData;
	return wxString::From8BitData((const char*)bRes);
}

byte* CEasyCrypto::Encrypt(byte *data, int dataLen)
{
	AllocCrypto(dataLen+1);
	SetPassword();

	memset(cryptoBuff, 0, dataLen+1);
	EncodeData(data, cryptoBuff, dataLen, gKeyData);
	return cryptoBuff;
}

byte* CEasyCrypto::Decrypt(byte *data, int dataLen)
{
	AllocCrypto(dataLen+1);
	SetPassword();

	memset(cryptoBuff, 0, dataLen+1);
	EncodeData(data, cryptoBuff, dataLen, gKeyData);
	return cryptoBuff;
}

void CEasyCrypto::AllocCrypto(int newSize)
{
	if (newSize > cryptoBuffSize) {
		if (cryptoBuff) {
			delete cryptoBuff;
		}
		cryptoBuffSize = newSize;
		cryptoBuff = new byte [cryptoBuffSize];
	}
}

void CEasyCrypto::AllocBinHex(int newSize)
{
	if (newSize > binHexBuffSize) {
		if (binHexBuff) {
			delete binHexBuff;
		}
		binHexBuffSize = newSize;
		binHexBuff = new byte [binHexBuffSize];
	}
}

wxString CEasyCrypto::Hash(wxString data)
{
    BYTE hash[SHA1_DIGEST];
    byte *lData = (byte*)strdup(data.ToUTF8().data());
    int len = strlen((const char*)lData);

    AllocBinHex(SHA1_DIGEST+2);
    SHA_160(lData, len, (PDWORD)hash);
	for (int i = 0; i < SHA1_DIGEST; i++) {
		binHexBuff[i] = ((hash[i] >> 4) & 0x0f)+ '0';
		if (binHexBuff[i] > '9') {
			binHexBuff[i] += 7;
		}
	}
	binHexBuff[SHA1_DIGEST] = '\0';
	binHexBuff[SHA1_DIGEST+1] = '\0';
	delete lData;
    return wxString::From8BitData((const char*)binHexBuff);
}
