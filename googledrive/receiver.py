import os
import sys

Path="/tmp/my_program.fifo"
fifo = open(Path, "r")
for line in fifo:
	print (line)
fifo.close()
