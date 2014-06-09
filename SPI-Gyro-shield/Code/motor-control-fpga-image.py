#!/usr/bin/env python

line=""
outf=open("motor-control-fpga-image.hpp","w")
with open("../../../robotex/moto2/fpga/Moto2_Implmnt/sbt/outputs/bitmap/Moto2_bitmap.bin", "rb") as inpf:
	byte = inpf.read(1)
	while byte != "":
		if len(line)>=40:
			outf.write("%s\n" % line);
			line=""
		if line=="":
			line="0x%0.2x," % ord(byte)
		else:
			line="%s0x%0.2x," % (line, ord(byte))
		byte = inpf.read(1)
if len(line)>0:
	outf.write("%s\n" % line);
inpf.close()
outf.close()
