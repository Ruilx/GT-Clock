#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import os, sys

from PIL import Image
import numpy
import struct
from InfoPacker import InfoPacker

LICENSE = os.path.join(os.path.dirname(sys.argv[0]), "LICENSE")

MSB_AT_TOP = 1
LSB_AT_TOP = -1
def getModeStr(mode):
	return {MSB_AT_TOP: "MSB_AT_TOP", LSB_AT_TOP: "LSB_AT_TOP"}[mode]

POSITIVE_LOGIC = 1
NEGATIVE_LOGIC = 0
def getLogicStr(logic):
	return {POSITIVE_LOGIC: "POSITIVE_LOGIC", NEGATIVE_LOGIC: "NEGATIVE_LOGIC"}[logic]

WIDTH_DETECT_AT_TOP = 1
WIDTH_DETECT_AT_BOTTOM = -1
def getWidthDetectLocStr(widthDetectLoc):
	return {WIDTH_DETECT_AT_TOP: "WIDTH_DETECT_AT_TOP", WIDTH_DETECT_AT_BOTTOM: "WIDTH_DETECT_AT_BOTTOM"}[widthDetectLoc]

WIDTH_DETECT_MODE_SAME_COLOR = 1
def getWidthDetectModeStr(widthDetectMode):
	return {WIDTH_DETECT_MODE_SAME_COLOR: "WIDTH_DETECT_MODE_SAME_COLOR"}[widthDetectMode]

OUTPUT_MODE_DATA_PLAIN = 0b00000001
OUTPUT_MODE_DATA_FUNC = 0b00000011
OUTPUT_MODE_INDEX_PLAIN = 0b00000100
OUTPUT_MODE_INDEX_FUNC = 0b00001100
OUTPUT_MODE_STATUS_COMMENT = 0b00010000
OUTPUT_MODE_SETUP_FUNC = 0b00100000
OUTPUT_MODE_IMG_STATS = 0b01000000
OUTPUT_MODE_PLAIN = OUTPUT_MODE_DATA_PLAIN | OUTPUT_MODE_INDEX_PLAIN
OUTPUT_MODE_FUNC = OUTPUT_MODE_DATA_FUNC | OUTPUT_MODE_INDEX_FUNC
OUTPUT_MODE_ALL_FUNC = OUTPUT_MODE_FUNC | OUTPUT_MODE_SETUP_FUNC
OUTPUT_MODE_PREFER = OUTPUT_MODE_ALL_FUNC | OUTPUT_MODE_STATUS_COMMENT | OUTPUT_MODE_IMG_STATS

class ImageBinaryzation(object):
	def __init__(self, img: str, fontIndex: int, packer: InfoPacker, startUnicode: int, endUnicode: int):
		if endUnicode < startUnicode:
			raise ValueError("endUnicode must larger than startUnicode.")
		self.conf = {
			'img': img,
			'mode': MSB_AT_TOP,
			'logic': NEGATIVE_LOGIC,
			'widthDetectLoc': WIDTH_DETECT_AT_TOP,
			'widthDetectMode': WIDTH_DETECT_MODE_SAME_COLOR,
			'startUnicode': startUnicode,
			'endUnicode': endUnicode,
			'binaryzationGrayscaleThreshold': 0x7F
		}
		self.currentUnicode = startUnicode
		self.imageHandle = Image.open(img).convert("L")
		if not self.imageHandle:
			raise RuntimeError("Image cannot be open: '{}'".format(img))
		if self.imageHandle.size[1] < 9:
			raise RuntimeError("Image height must be 9, but access {}".format(self.imageHandle.size[1]))
		self.imgArr = numpy.array(self.imageHandle)
		if self.conf['widthDetectLoc'] == WIDTH_DETECT_AT_TOP:
			self.widthSign = self.imgArr[0]
			self.lineHandle = [
				self.imgArr[1],
				self.imgArr[2],
				self.imgArr[3],
				self.imgArr[4],
				self.imgArr[5],
				self.imgArr[6],
				self.imgArr[7],
				self.imgArr[8]
			]
		elif self.conf['widthDetectLoc'] == WIDTH_DETECT_AT_BOTTOM:
			self.widthSign = self.imgArr[8]
			self.lineHandle = [
				self.imgArr[0],
				self.imgArr[1],
				self.imgArr[2],
				self.imgArr[3],
				self.imgArr[4],
				self.imgArr[5],
				self.imgArr[6],
				self.imgArr[7]
			]
		else:
			raise RuntimeError("Invalid mode, Please check WIDTH_DETECT_MODE")
		self.currentColumn = 0

		self.packer = packer
		if len(self.lineHandle) != 8:
			raise RuntimeError("Invalid lineHandle length, array length must be 8")

		self.fontIndex = fontIndex
		self.indexBytes = 0
		self.indexStr = ""
		self.dataBytes = 0
		self.dataStr = ""
		self.dataSize = 0
		self.statusStr = ""

		self.ready = False

	def __setitem__(self, key, value):
		if key not in self.conf:
			raise KeyError("Key '{}' not found in configure.".format(key))
		self.conf[key] = value

	def __getitem__(self, item):
		if item not in self.conf:
			raise KeyError("Key '{}' not found in configure.".format(item))
		return self.conf[item]

	def __checkThreshold(self, pixel):
		return 1 if pixel > self.conf['binaryzationGrayscaleThreshold'] else 0

	def __getherColumn(self):
		byte = 0x00
		if self.conf['mode'] == MSB_AT_TOP:
			shiftLevel = lambda bit: ((byte << 1) | bit)
		elif self.conf['mode'] == LSB_AT_TOP:
			shiftLevel = lambda bit: ((byte >> 1) | (bit << 7))
		else:
			raise RuntimeError("Invalid mode, Please check mode is one of MSB_AT_TOP, LSB_AT_TOP")
		if self.currentColumn < self.imageHandle.size[0]:
			for i in self.lineHandle:
				byte = shiftLevel(self.__checkThreshold(i[self.currentColumn]))
			self.currentColumn += 1

		return byte if self.conf['logic'] == NEGATIVE_LOGIC else (~byte) & 0xFF

	def __getherChar(self, tab=1):
		if self.currentColumn >= self.imageHandle.size[0]:
			return False
		sign = self.widthSign[self.currentColumn]
		byteStr = b""
		while self.currentColumn < self.imageHandle.size[0] and \
				self.widthSign[self.currentColumn] == sign:
			byteStr += struct.pack("B", self.__getherColumn())
		index, data, status, dataBytes, dataSize = self.packer.transaction(self.currentUnicode, byteStr, tab)
		self.indexStr += index
		self.indexBytes += 2
		self.dataStr += data
		self.dataBytes += dataBytes
		self.dataSize += dataSize
		self.statusStr += status
		self.currentUnicode += 1
		return True

	def resetResult(self):
		self.currentUnicode = startUnicode
		self.currentColumn = 0
		self.indexBytes = 0
		self.indexStr = ""
		self.dataBytes = 0
		self.dataStr = ""
		self.dataSize = 0
		self.statusStr = ""

	def __generateStatusInfo(self):
		return "Using image: '{ImgPath}' size {ImgSize}, unicode from {UnicodeFrom} to {UnicodeTo}/{UnicodePlan}, grayscale threshold: {Threshold}, converted by ImageBinaryzation.\n".format(
				ImgPath=self.conf['img'],
				ImgSize=str(self.imageHandle.size),
				UnicodeFrom=self.conf['startUnicode'],
				UnicodeTo=self.currentUnicode -1,
				UnicodePlan=self.conf['endUnicode'],
				Threshold=self.conf['binaryzationGrayscaleThreshold']
			) + \
		"With options: {}".format(
			", ".join([getModeStr(self.conf['mode']), getLogicStr(self.conf['logic']),
					   getWidthDetectLocStr(self.conf['widthDetectLoc']),
					   getWidthDetectModeStr(self.conf['widthDetectMode'])])
		)

	def binaryzationPrint(self, output=OUTPUT_MODE_ALL_FUNC, **kwargs):
		if not self.ready:
			tab = 1
			if "tab" in kwargs:
				tab = int(kwargs['tab'])
			while self.__getherChar(tab=tab):
				pass
			self.ready = True

		fd = sys.stdout
		if "file" in kwargs and kwargs['file'] != sys.stdin:
			fd = kwargs['file']

		if output & OUTPUT_MODE_INDEX_PLAIN:
			if output & OUTPUT_MODE_INDEX_FUNC == OUTPUT_MODE_INDEX_FUNC:
				attrPre = ""
				attrSuf = ""
				if 'attributePrefix' in kwargs:
					attrPre = kwargs['attributePrefix']
				if 'attributeSuffix' in kwargs:
					attrSuf = kwargs['attributeSuffix']
				print(self.packer.cStyleIndexGenerator(attrPre, attrSuf).format(self.indexStr), file=fd)
			else:
				print(self.indexStr, file=fd)
			print(file=fd)
		if output & OUTPUT_MODE_DATA_PLAIN:
			if output & OUTPUT_MODE_DATA_FUNC == OUTPUT_MODE_DATA_FUNC:
				attrPre = ""
				attrSuf = ""
				if 'attributePrefix' in kwargs:
					attrPre = kwargs['attributePrefix']
				if 'attributeSuffix' in kwargs:
					attrSuf = kwargs['attributeSuffix']
				print(self.packer.cStyleDataGenerator(attrPre, attrSuf).format(self.dataStr), file=fd)
			else:
				print(self.dataStr, file=fd)
			print(file=fd)
		if output & OUTPUT_MODE_SETUP_FUNC:
			self.packer['fontInfo']['logicality'] = "true" if self.conf['logic'] == POSITIVE_LOGIC else "false"
			print(self.packer.cStyleSetupFuncGenerator(self.fontIndex, self.conf['startUnicode'], self.currentUnicode), file=fd)
		if output & OUTPUT_MODE_STATUS_COMMENT:
			print("/*", file=fd)
			print("%7s %5s %+6s %5s %8s" % ("Char", "Width", "Adjust", "Index", "Total"), file=fd)
			print(self.statusStr, end="", file=fd)
			print("Font Size Status: Index section %d byte%s, Pure Data %d byte%s, Data section %d byte%s, Total %d byte%s." % (
				self.indexBytes, "s" if self.indexBytes != 1 else "",
				self.dataBytes, "s" if self.dataBytes != 1 else "",
				self.dataSize, "s" if self.dataSize != 1 else "",
				self.indexBytes + self.dataSize, "s" if self.indexBytes + self.dataSize != 1 else ""
			), file=fd)
			if not output & OUTPUT_MODE_IMG_STATS:
				print("*/", file=fd)
		if output & OUTPUT_MODE_IMG_STATS:
			if not output & OUTPUT_MODE_STATUS_COMMENT:
				print("/*", file=fd)
			print(self.__generateStatusInfo(), file=fd)
			print("*/", file=fd)
		if "file" in kwargs and kwargs['file'] not in (sys.stdout, sys.stderr):
			print(self.__generateStatusInfo())

def commitGPL3():
	content = ""
	fd = open(LICENSE, "r")
	if not fd:
		raise RuntimeError("License not found, please read and agreed GPL3 license first.")
	try:
		while True:
			line = fd.readline()
			if (not line) or line == "<LineSeparator>\n":
				break
			content += line
	finally:
		fd.close()
	return content

import argparse

if __name__ == "__main__":
	parser = argparse.ArgumentParser(sys.argv[0], description="An 8-bit-height pixel font image to bit map converter")
	parser.add_argument("-i", "--img", type=str, required=True, dest="img", help="Image file path to convert")
	parser.add_argument("-x", "--index", type=int, required=True, dest="index", help="Font index number")
	parser.add_argument("-b", "--block", type=int, required=True, dest="block", help="Block length in index set")
	parser.add_argument("-d", "--display-name", type=str, required=True, dest="displayName", help="Font display name")
	parser.add_argument("-n", "--name", type=str, default="", dest="name", help="font name in variable. Default: eraseSpace(lower(displayName))")
	parser.add_argument("-f", "--fill-byte", type=int, default=0, dest="fillByte", help="binary index block size alignment padding byte. Defatult: 0x00")
	parser.add_argument("-s", "--start-unicode", type=int, default=0x20, dest="startUnicode", help="First character in image. Default: 0x20(<space>)")
	parser.add_argument("-e", "--end-unicode", type=int, default=0x7F, dest="endUnicode", help="Last character in image. Default: 0x7F(<del>)")
	parser.add_argument("-c", "--comment", type=bool, default=True, dest="commentShown", help="show C style output comment. Default: true")
	parser.add_argument("--comment-start", type=str, default="/*", dest="commentStart", help="C style output comment prefix(start) string. Default: /*")
	parser.add_argument("--comment-end", type=str, default="*/", dest="commentEnd", help="C style output comment suffix(end) string. Default: */")
	parser.add_argument("--comment-location", type=int, default=1, choices={-1, 1}, dest="commentLocation", help="C style output comment location. -1: before data, 1: after data. Default: 1")
	parser.add_argument("--comment-alignment", type=int, default=1, choices={-1, 1}, dest="commentAlignment", help="C style output comment alignment(only after data location. -1: align left, 1: align right. Default: 1")
	parser.add_argument("--c-style-array-type", type=str, default="const unsigned char", dest="cStyleArrayType", help="C style variable type. Default=\"const unsigned char\"")
	parser.add_argument("--font-index-suffix", type=str, default="_index", dest="fontIndexSuffix", help="C style index variable name suffix. Default=\"_index\"")
	parser.add_argument("--font-data-suffix", type=str, default="_data", dest="fontDataSuffix", help="C style data variable name suffix. Default=\"_data\"")
	parser.add_argument("--font-setup-prefix", type=str, default="setupFont", dest="fontSetupPrefix", help="C style font setup function name prefix. Default=\"setupFont\"")
	parser.add_argument("--font-info-mono-numbers", type=str, default="true", choices={"true", "false"}, dest="fontInfoMonoNumbers", help="C style font info struct property 'monoNumbers'. True: numbers are monospaced. Default: true")
	parser.add_argument("--font-info-mono-mask", type=str, default="0xFF", dest="fontInfoMonoMask", help="C style font info struct property 'monoMask'. Default: 0xFF, means 8 Column all selected.")
	parser.add_argument("-m", "--mode", type=str, choices={"MSB_AT_TOP", "LSB_AT_TOP"}, default="MSB_AT_TOP", dest="mode", help="Binary bit order specified in image. Default: MSB_AT_TOP")
	parser.add_argument("-l", "--logic", type=str, choices={"POSITIVE_LOGIC", "NEGATIVE_LOGIC"}, default="NEGATIVE_LOGIC", dest="logic", help="Binary bit logic for pixel 'ON'. Default: NEGATIVE_LOGIC")
	parser.add_argument("--width-detect-location", type=str, choices={"WIDTH_DETECT_AT_TOP", "WIDTH_DETECT_AT_BOTTOM"}, default="WIDTH_DETECT_AT_TOP", dest="widthDetectLocation", help="Character width detect line position in image. Default: WIDTH_DETECT_AT_TOP")
	parser.add_argument("--width-detect-mode", type=str, choices={"WIDTH_DETECT_MODE_SAME_COLOR"}, default="WIDTH_DETECT_MODE_SAME_COLOR", dest="widthDetectMode", help="Character width detect mode. Default: WIDTH_DETECT_MODE_SAME_COLOR, same character with continuously same color.")
	parser.add_argument("-o", "--output", type=str, default="", dest="output", help="Specific result output. Default: stdout, Support: stdout, stderr, FILE_NAME")
	parser.add_argument("--output-mode", type=str, default="PREFER", choices={"DATA_PLAIN", "DATA_FUNC", "INDEX_PLAIN", "INDEX_FUNC", "STATUS_COMMENT", "SETUP_FUNC", "IMG_STATS", "PLAIN", "FUNC", "ALL_FUNC", "PREFER"}, dest="outputMode", help="Specific result output content.")
	parser.add_argument("-t", "--threshold", type=int, default=0x7F, dest="threshold", help="Grayscale pixel 'ON'/'OFF' threshold value. Default: 0x7F(127).")
	parser.add_argument("--special", action="append", dest="special", choices=["gpl3", "preprocessflag"], help="Special operation. gpl3: adding GPL3 license at top. preprocessflag: adding C-style preprocess sign around data")

	args = parser.parse_args()
	name = None
	if args.name != "":
		name = args.name

	ip = InfoPacker(args.displayName, args.block, args.fillByte, name)
	ip['commentShown'] = args.commentShown
	ip['commentStart'] = args.commentStart
	ip['commentEnd'] = args.commentEnd
	ip['commentLocation'] = args.commentLocation
	ip['commentAlignment'] = args.commentAlignment
	ip['cStyleArrayType'] = args.cStyleArrayType
	ip['fontIndexSuffix'] = args.fontIndexSuffix
	ip['fontDataSuffix'] = args.fontDataSuffix
	ip['fontSetupPrefix'] = args.fontSetupPrefix
	ip['fontInfo']['monoNumbers'] = args.fontInfoMonoNumbers
	ip['fontInfo']['monoMask'] = args.fontInfoMonoMask

	mode = {'MSB_AT_TOP': MSB_AT_TOP, 'LSB_AT_TOP': LSB_AT_TOP}[args.mode]
	logic = {'NEGATIVE_LOGIC': NEGATIVE_LOGIC, 'POSITIVE_LOGIC': POSITIVE_LOGIC}[args.logic]
	widthDetectLocation = {'WIDTH_DETECT_AT_TOP': WIDTH_DETECT_AT_TOP, 'WIDTH_DETECT_AT_BOTTOM': WIDTH_DETECT_AT_BOTTOM}[args.widthDetectLocation]
	widthDetectMode = {'WIDTH_DETECT_MODE_SAME_COLOR': WIDTH_DETECT_MODE_SAME_COLOR}[args.widthDetectMode]

	b = ImageBinaryzation(args.img, args.index, ip, args.startUnicode, args.endUnicode)
	b['mode'] = mode
	b['logic'] = logic
	b['widthDetectLoc'] = widthDetectLocation
	b['widthDetectMode'] = widthDetectMode
	b['binaryzationGrayscaleThreshold'] = args.threshold

	fd = sys.stdout
	fileFlag = False
	if args.output != "":
		if args.output == "stderr":
			fd = sys.stderr
		elif args.output == "stdout":
			fd = sys.stdout
		else:
			fd = open(args.output, "w", encoding='utf-8')
			if fd:
				fileFlag = True
			else:
				raise RuntimeError("Cannot open file: '%s' to write." % args.output)

	outputMode = {'DATA_PLAIN': OUTPUT_MODE_DATA_PLAIN,
				  'DATA_FUNC': OUTPUT_MODE_DATA_FUNC,
				  'INDEX_PLAIN': OUTPUT_MODE_INDEX_PLAIN,
				  'INDEX_FUNC': OUTPUT_MODE_INDEX_FUNC,
				  'STATUS_COMMENT': OUTPUT_MODE_STATUS_COMMENT,
				  'SETUP_FUNC': OUTPUT_MODE_SETUP_FUNC,
				  'IMG_STATS': OUTPUT_MODE_IMG_STATS,
				  'PLAIN': OUTPUT_MODE_PLAIN,
				  'FUNC': OUTPUT_MODE_FUNC,
				  'ALL_FUNC': OUTPUT_MODE_ALL_FUNC,
				  'PREFER': OUTPUT_MODE_PREFER}[args.outputMode]

	if args.special and "gpl3" in args.special:
		print("/*\n" + commitGPL3() + "*/", file=fd)

	if args.special and "preprocessflag" in args.special:
		content = "#ifndef " + "_" + ip['name'].upper() + "_H_\n" + \
			"#define " + "_" + ip['name'].upper() + "_H_\n"
		print(content, file=fd)

	b.binaryzationPrint(outputMode, file=fd)

	if args.special and "preprocessflag" in args.special:
		print("#endif\n", file=fd)

	if fileFlag:
		fd.close()