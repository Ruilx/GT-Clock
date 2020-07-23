#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os, sys, io
import gzip
import argparse

"""
GZip Compress & Decompress tool
@auteur: Ruilx
@date: 2020/07/06
"""

def main():
    parser = argparse.ArgumentParser(sys.argv[0], description="Use this file to compress or uncompress specific file using gz method.")
    parser.add_argument("-o", "--output", type=str, dest="output", default="", help="Specific output file")
    parser.add_argument("-i", "--input", type=str, dest="input", required=True, help="Specific input file")
    parser.add_argument("-m", "--method", type=str, dest="method", default="compress", choices={'compress', 'decompress'}, help="Specific method")
    parser.add_argument("-l", "--level", type=int, dest="level", default=9, help="compress level (0-9)")
    args = parser.parse_args()

    inputFile = args.input
    outputFile = args.output if args.output else inputFile + ".gz"

    if not os.path.isfile(inputFile):
        raise FileNotFoundError("File %s not found." % inputFile)

    fdinput = open(inputFile, 'rb')
    fdoutput = open(outputFile, "wb")
    bufLength = 1024

    if os.stat(inputFile).st_size > 1 * 1024 * 1024:
        raise ValueError("File size too large.");
    if args.method == 'compress':
        buf = io.BytesIO()
        gzf = gzip.GzipFile(fileobj=buf, mode='wb', compresslevel=args.level)
        inputBuf = fdinput.read(bufLength)
        while inputBuf:
            gzf.write(inputBuf)
            inputBuf = fdinput.read(bufLength)
        gzf.close()
        fdoutput.write(buf.getvalue())
    elif args.method == 'decompress':
        gzf = gzip.GzipFile(fileobj=io.BytesIO(inputFile.read()))
        buf = gzf.read(bufLength)
        while buf:
            outputFile.write(buf)
            buf = gzf.read(bufLength)
        gzf.close()

    # gzipper = gzip.compress(fileobj=fdoutput, mode='w')
    # bufLength = 1024
    # buf = fdinput.read(bufLength)

    # while buf:
    #     gzipper.write(buf)
    #     buf = fdinput.read(bufLength)

    fdoutput.close()
    fdinput.close()

    print("Convert finished.")

if __name__ == '__main__':
    main()