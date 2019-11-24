import os
import subprocess

def fix_stage1():
	size = os.stat("build\kernel.exe").st_size

	size = (size + 511) / 512
	size = int(size)
	
	if size >= 255:
		raise Exception("kernel is too large")
	
	with open("build\stage1", "rb+") as f:
		d = f.read()
		d = bytearray(d)
		idx = d.index(b"\xb0\xcc\x90\x90")
		d[idx+1] = size
		f.seek(0)
		f.write(d)

def create_compile_kernel_cmd():
	cmd = "gcc -Ikernel -std=c99 -mno-ms-bitfields -masm=intel -m32 -nostdlib -o build\kernel kernel\src\kernel.c" #For more warnings -Wall -Wextra 

	for root, directories, filenames in os.walk("kernel"): 
		for filename in filenames:
			if (filename.endswith(".c") or filename.endswith(".s") or filename.endswith(".S")) and filename!="kernel.c":
				cmd = cmd + ' ' + os.path.join(root,filename)
				
	cmds_to_run.insert(0, cmd)

#creat list of commands to run	
cmds_to_run = [
	"strip build\kernel.exe",
	"nasm bootloader\stage1.asm -o build\stage1",
	"nasm bootloader\stage2.asm -o build\stage2"
]

files_to_img = [
	"build\stage1",
	"build\stage2",
	"build\kernel.exe"
]

def main():
	create_compile_kernel_cmd()

	for cmd in cmds_to_run:
		print("Running:" + cmd + "\n")
		print(str(subprocess.check_output(cmd, shell=False), 'utf-8'))

	fix_stage1()
		
	buf = []
	for fn in files_to_img:
		with open(fn, "rb") as f:
			d = f.read()
			buf.append(d)

			if len(d) % 512 == 0:
				continue

			padding_size = 512 - len(d) % 512				
			buf.append("\0" * padding_size);

	try:
		os.remove("build\\floppy.raw")
	except:
		print("")

	with open("build\\floppy.raw", "ab") as f:
		for b in buf:
			f.write(b)

if __name__ == '__main__':
	main()