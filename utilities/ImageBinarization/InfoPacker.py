#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
GT-Clock Utilities
Font info packer tools
@author: Ruilx
@2020/07/01 18:36
"""

class InfoPacker(object):
	def __init__(self, displayName: str, blockSize: int, fillByte: int = 0, name = None):
		if fillByte > 0xFF:
			raise ValueError("fillByte argument cannot larger than 1 byte.")
		if len(displayName) > 15:
			raise ValueError("font display name length must less than 15")
		if blockSize > 0xFF:
			raise ValueError("blockSize not support above 256")
		self.conf = {
			'displayName': displayName,
			'fillByte': fillByte,
			'blockSize': blockSize,
			'name': name if name else displayName.lower().replace(" ", ""),
			'specialChars': {
				0x00: "<nul>",
				0x01: "<soh>",
				0x02: "<stx>",
				0x03: "<etx>",
				0x04: "<eot>",
				0x05: "<enq>",
				0x06: "<ack>",
				0x07: "<bel>",
				0x08: "<bs>",
				0x09: "<ht>",
				0x0A: "<lf>",
				0x0B: "<vt>",
				0x0C: "<ff>",
				0x0D: "<cr>",
				0x0E: "<so>",
				0x0F: "<si>",
				0x10: "<dle>",
				0x11: "<dc1>",
				0x12: "<dc2>",
				0x13: "<dc3>",
				0x14: "<dc4>",
				0x15: "<nak>",
				0x16: "<syn>",
				0x17: "<etb>",
				0x18: "<can>",
				0x19: "<em>",
				0x1A: "<sub>",
				0x1B: "<esc>",
				0x1C: "<fs>",
				0x1D: "<gs>",
				0x1E: "<rs>",
				0x1F: "<us>",
				0x20: "<sp>",
				0x7F: "<del>",
				ord("\\"): "\\"
			},
			'commentShown': True,
			'commentStart': "/*",
			'commentEnd': "*/",
			'commentLocation': -1,
			'commentAlignment': 1,
			'cStyleArrayType': "const unsigned char",
			'fontIndexSuffix': "_index",
			'fontDataSuffix': "_data",
			'fontIndexPrefix': "font_",
			'fontDataPrefix': "font_",
			'fontSetupPrefix': "font_",
			'fontInfo': {
				'logicality': "false",
				'numberHeight': 7,
			},
			'displayFontStatus': True,
			'cStyleNull': "nullptr"
		}
		self.currentIndex = 0

		if not ("A" <= self.conf['name'][0] <= "Z" or "a" <= self.conf['name'][0] <= "z" or self.conf['name'][0] == "_"):
			raise RuntimeError("Font name only allow English letter or underline '_' at first character.")

	def __setitem__(self, key, value):
		if key not in self.conf:
			raise KeyError("Key '{}' not found in configure.".format(key))
		self.conf[key] = value

	def __getitem__(self, item):
		if item not in self.conf:
			raise KeyError("Key '{}' not found in configure.".format(item))
		return self.conf[item]

	def __charToStr(self, char):
		if char in self.conf['specialChars']:
			return self.conf['specialChars'][char]
		else:
			return chr(char)

	def __doingComment(self, unicodeOrder, commentLocation, defineValue):
		return "%s" % ("%s %s %s%s" % (
			self.conf['commentStart'],
			self.__charToStr(unicodeOrder),
			self.conf['commentEnd'],
			" " if commentLocation == -1 else ""
		)) if commentLocation == defineValue and self.conf['commentShown'] else " "

	def __cStyleCodeGenerator(self, prefix, suffix, attributePrefix = "", attributeSuffix = ""):
		return " ".join(filter(lambda x: x != "", (attributePrefix, self.conf['cStyleArrayType'], prefix + self.conf['name'] + suffix, attributeSuffix, "[]" ,"= {{"))) + \
			   "\n{}}};"

	def cStyleIndexGenerator(self, attributePrefix = "", attributeSuffix = ""):
		return self.__cStyleCodeGenerator(self.conf['fontIndexPrefix'], self.conf['fontIndexSuffix'], attributePrefix, attributeSuffix)

	def cStyleDataGenerator(self, attributePrefix = "", attributeSuffix = ""):
		return self.__cStyleCodeGenerator(self.conf['fontDataPrefix'], self.conf['fontDataSuffix'], attributePrefix, attributeSuffix)

	def cStyleFontInfoStruct(self):
		return """#ifndef _FONT_INFO_STRUCT_
#define _FONT_INFO_STRUCT_
typedef struct FontInfo_t {
	uint8_t fontIndex;
	//char fontName[16];
	uint16_t startUnicode;
	uint16_t endUnicode;
	uint_8 blockLength;
	bool logicality;
	bool monospace;
	uint8_t numberHeight,
	const unsigned char *index;
	const unsigned char *data;
} FontInfo;
#endif
"""

	def cStyleSetupFuncGenerator(self, fontIndex, startUnicode, endUnicode, monospace, forceWriteIndex):
		return """static const FontInfo {FontSetupPrefix}{FontName} = {{
	.fontIndex = {FontIndex},
	//.fontName = "{FontDisplayName}",
	.startUnicode = {StartUnicode},
	.endUnicode = {EndUnicode},
	.blockLength = {BlockLength},
	.logicality = {Logicality},
	.monospace = {Monospace},
	.numberHeight = {NumberHeight},
	.index = {Index},
	.data = {Data},
}};
""".format(FontSetupPrefix=self.conf['fontSetupPrefix'],
		   FontName=self.conf['name'],
		   FontIndex=fontIndex,
		   FontDisplayName=self.conf['displayName'],
		   StartUnicode=startUnicode,
		   EndUnicode=endUnicode,
		   BlockLength=self.conf['blockSize'],
		   Logicality=self.conf['fontInfo']['logicality'],
		   Monospace="true" if monospace else "false",
		   NumberHeight=self.conf['fontInfo']['numberHeight'],
		   Index=(self.conf['name'] + self.conf['fontIndexSuffix']) if not monospace or forceWriteIndex else self.conf['cStyleNull'],
		   Data=self.conf['name'] + self.conf['fontDataSuffix'],)

	def transaction(self, unicodeOrder, data: bytes, tab: int = 1):
		length = len(data)
		block = self.conf['blockSize']
		indexDelta = (length / block) if (length % block == 0) else ((length // block) +1)
		currentIndex = self.currentIndex
		self.currentIndex += indexDelta
		commentLocation = self.conf['commentLocation']
		dataAdj = data + (chr(self.conf['fillByte']) * (
			(block - (length % block)) if (length % block != 0)
			else 0
		)).encode('ascii')
		return (
			"%s%s0x%02X,0x%02X,%s\n" % (
				"\t" * tab,
				self.__doingComment(unicodeOrder, commentLocation, -1),
				int(currentIndex),
				length,
				"%*s" % (
					(12 * self.conf['commentAlignment']) if commentLocation == 1 else 0,
					self.__doingComment(unicodeOrder, commentLocation, 1)
				) if commentLocation == 1 else ""
			),
			"%s%s%s%s\n" % (
				"\t" * tab,
				self.__doingComment(unicodeOrder, commentLocation, -1),
				",".join("0x%02x" % i for i in dataAdj) + ",",
				"%*s" % (
					(12 * self.conf['commentAlignment']) if commentLocation == 1 else 0,
					self.__doingComment(unicodeOrder, commentLocation, 1)
				) if commentLocation == 1 else ""

			),
			# "%s" % ("Char: '%s', width: %d, adjust: %+d, index: %d, total: %d byte%s\n" % (
			# 		self.__charToStr(unicodeOrder),
			# 		length,
			# 		len(dataAdj) - length,
			# 		currentIndex,
			# 		len(dataAdj),
			# 		"s" if len(dataAdj) != 1 else ""
			# 	) if self.conf['displayFontStatus'] else ""
			# ),
			("%7s %3d   %+4d  %5d %3d byte%s\n" % ("'" + self.__charToStr(unicodeOrder) + "'", length, len(dataAdj) - length, currentIndex, len(dataAdj), "s" if len(dataAdj) != 1 else "")) if self.conf['displayFontStatus'] else "",
			length, len(dataAdj)
		)

if __name__=="__main__":
	print("This class has no entry point.")
	pass
