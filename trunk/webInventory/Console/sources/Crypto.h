/*
 * This program is free software; you can redistribute it but not modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */

#ifndef _ICRYPTO_H
#define _ICRYPTO_H

#include <wx/wx.h>

#ifndef byte
typedef unsigned char byte;
#endif
#ifndef word
typedef unsigned short word;
#endif

#if !defined(__WXMSW__)
	#define MAX_PATH 1024
#endif

#define KEY_BYTES   16
#define KEY_BITS    128
#define KEY_MASK    7

class CEasyCrypto {
private:
	byte	*cryptoBuff;
	int		cryptoBuffSize;
	byte	*binHexBuff;
	int		binHexBuffSize;
	byte	gKeyData[KEY_BYTES];

	void AllocCrypto(int newSize);
	void AllocBinHex(int newSize);
	void SetPassword();

public:
	CEasyCrypto();
	~CEasyCrypto();

	wxString Hash(wxString data);
	wxString Encrypt(wxString data);
	wxString Decrypt(wxString data);

	byte *Encrypt(byte *data, int dataLen);
	byte *Decrypt(byte *data, int dataLen);
};

#endif /* _ICRYPTO_H */
