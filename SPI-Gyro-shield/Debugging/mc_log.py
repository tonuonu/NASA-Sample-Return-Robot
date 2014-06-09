#!/usr/bin/env python
import serial
import string
import sys,getopt
import _winreg as winreg
import itertools
import fnmatch
import math
import time

global port, log_level, defaultbaud
port = ''
log_level = 0
defaultbaud = 3000000

def enumerate_serial_ports():
    """ Uses the Win32 registry to return an
        iterator of serial (COM) ports
        existing on this computer.
    """
    path = 'HARDWARE\\DEVICEMAP\\SERIALCOMM'
    try:
        key = winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, path)
    except WindowsError:
        raise IterationError

    for i in itertools.count():
        try:
            val = winreg.EnumValue(key, i)
            yield str(val[1])
        except EnvironmentError:
            break

def log(level, s):
	global log_level
	if (level <= log_level):
		print s
	
	
def clear_buffers(ser):
	for i in range(10):
		ser.write(chr(0xff))
	ser.flush()
	i = ser.inWaiting()
	if (i>0):
		d = ser.read(i)
	log(0, "Buffers clear done. Read %d bytes from Rx queue." % i)
	return
	
	
def capture_data(ser):
	outf = open("rubeet.log","w")
	row=""
	for i in range(4):
		row = "%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s" % (row, "tspeed", "load", "dist", "pidp", "pidi", "phase", "cycles")
	outf.write("%s\n" % row[1:])
	
	prevsec = -1
	linecnt = 0
	secs = -1
	total = 0
	while True:
		newsec=int(time.time())
		if newsec>prevsec:
			prevsec=newsec
			secs = secs + 1
			total = total + linecnt
			if secs>0:
				print "%d (avg: %1.1f)" % (linecnt, float(total)/secs)
			linecnt = 0
		try:
			line = ser.read(80)
			linecnt = linecnt+1
			if len(line)==80:
				i=0
				row=""
				for item in line.split():
					if (i%4)==0:
						tspeed=int(item,16) 
						if tspeed&0x8000:
							tspeed=tspeed-65536
					elif (i%4)==1:
						load=int(item,16)>>9
						if load&0x40:
							load=load-128
						dist=int(item,16)&0x1FF
						if dist&0x100:
							dist=dist-512
					elif (i%4)==2:
						pidp=int(item,16)>>8
						if pidp&0x80:
							pidp=pidp-256
						pidi=int(item,16)&0xFF
						if pidi&0x80:
							pidi=pidi-256
					else:
						phase=int(item,16)>>13
						cycles=int(item,16)&0xFFF
						row = "%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d" % (row, tspeed, load, dist, pidp, pidi, phase, cycles)
					i=i+1
				outf.write("%s\n" % row[1:])
		except:
			outf.close()
			return

def get_num(s):
	try:
		return int(eval(s))
	except:
		raise

		
def get_num_safe(s):
	try:
		return int(eval("%s" % s))
	except:
		return 0
		
			
def usage():
	print """
Usage:
$ """+__file__+""" [command] [args]

Where:
[command] - A command to perform
[args]    - Additional arguments to the command

Commands supported:
 -p:   Port selection, "ignore" to skip HW communication. Default: """+port+"""
 -l:   Set log level [0,1,2]. Default is 0
 -c:   Capture data

 """
	sys.exit(-1)

def main(argv):
	global port, legacy_read_cmd, log_level, ini_file
	global registers
	index = 'none'
	options = {'none':[]}
	for arg in argv:                
		if arg in ['-p', '-c', '-l']:      
			index = arg
		elif arg in ['-?', '-h', '-help']:
			usage()

		if index in options:
			options[index].append(arg)
		else:
			options[index] = []

	if '-p' in options:
		port = options['-p'][0]

	if '-l' in options:
		log_level = (1 if (len(options['-l'])==0) else get_num_safe(options['-l'][0]))
		#print "Loglevel %d" % log_level

	if (port == ''):
		ports = enumerate_serial_ports()
		pcnt = 0
		print "Ports available in system:"
		for p in ports:
			print p
			pcnt = pcnt +1
		if pcnt == 1:
			port = p
			print "Using available port: " + port
		else:
			if port!='ignore':
				if pcnt > 1:
					print "\nplease select!"
				else:
					print "None"
				sys.exit(-1)
		
	try:
		if port!='ignore':
			ser = serial.Serial(port=port, baudrate=defaultbaud, timeout=1)
		else:
			ser=0
		#print ser.getSettingsDict()
	except:
		print "Cannot open " + port
		if port!='ignore':
			usage()
		else:
			ser=0
	
	if '-c' in options:
		capture_data(ser)
	
	if ser:	
		ser.close()


if __name__ == "__main__":
	main(sys.argv[1:])
else:
	usage()
