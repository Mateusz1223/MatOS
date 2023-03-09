import os
import subprocess

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
		
	buff = bytearray(b'')
	for fn in files_to_img:
		with open(fn, "rb") as f:
			buff += bytearray(f.read())
			if len(buff) % 512 == 0:
				continue
			padding_size = 512 - len(d) % 512				
			buff += bytearray(b"\0" * padding_size)

	if len(buff) / 512 - 3 > 127:
		print("Kernel is too big. Changes in stage1 of the bootloader required!")
		exit(1)

	# allign to 64MB
	padding_size = 67108864 - len(buff)
	buff += bytearray(b"\0" * padding_size)

	try:
		os.remove("build\\matos.img")
	except:
		print("")

	with open("build\\matos.img", "ab") as f:
		f.write(buff)

if __name__ == '__main__':
	main()