# Double Indirect Node

This repository contains modifications to the inode (i.e., dinode) structure to support a double indirect pointer for blocks. After these modifications, a file can be composed of 11 + 128 + 128 * 128 blocks. A thorough investigation of the file system has been conducted to ensure its proper functioning in all scenarios. The mkfs tool does not require any changes.

## Encryption and Decryption

For encryption purposes, a Caesar cipher is used. This cipher requires setting an integer as the encryption and decryption key, which should work for all ASCII characters. The key adjustment is done through the setkey system call, and the key is global across the entire operating system. The key is always a non-negative number, including zero. Any user program can make this call to change the global key.

Once the key is set, an existing file on the disk can be encrypted or decrypted using the encr and decr system calls. Both system calls expect the file to be already opened for both reading and writing.

Furthermore, the behavior of the read and write system calls needs to be modified to automatically perform decryption/encryption during reading/writing. Files will have information about their encryption status in the major attribute of the inode structure: 0 means the file is not encrypted, and 1 means it is encrypted. These operations use the globally set key for decryption/encryption. If an incorrect key is used during file reading, the result should be unintelligible text.

## System Calls

System calls must not print anything on the screen. All output, whether the system call was successful or not, should be done by user programs.

### `int setecho(int do_echo);`

This system call turns on or off the echo functionality on the console. If the parameter passed is 1, the system works normally. If the parameter is 0, all characters except '\n' will be displayed on the console as '*'. The system call returns -1 if an error occurs during its execution and 0 otherwise.

### `int setkey(int key);`

The passed parameter is the key used for encryption and decryption. This system call returns -1 if an error occurs during its execution and 0 otherwise.

### `int encr(int fd);`

This system call encrypts the file using the currently set global encryption key. The parameter passed is the file descriptor of the file to be encrypted, and it is expected that the file is already opened for reading and writing. Files of type T_DEV cannot be encrypted. The return values can be as follows:
- -1: The key is not set.
- -2: The file is of type T_DEV.
- -3: The file is already encrypted.
- 0: The system call was successful.

### `int decr(int fd);`

This system call decrypts the file using the currently set global decryption key. The parameter passed is the file descriptor of the file to be decrypted, and it is expected that the file is already opened for reading and writing. Files of type T_DEV cannot be decrypted. The return values can be as follows:
- -1: The key is not set.
- -2: The file is of type T_DEV.
- -3: The file is not encrypted.
- 0: The system call was successful.

## User Programs

If an error occurs during the execution of any system call, the user program must print a meaningful error message to inform the user of the issue.

### Program `setkey`

The `setkey` program allows setting the encryption and decryption key. The `encr` and `decr` programs enable the encryption and decryption of existing files on the disk, respectively. Finally, the `blockwriter` program allows creating a file that occupies a variable number of blocks and contains simple and predictable text.

### Program `blockwriter`

The `blockwriter` program creates a new file in the current directory with a specified size in blocks and a given name. The default values are 150 blocks and the filename `long.txt`. Users of this program can modify these values using command-line arguments.

**Options:**
- `--help (-h)`: Show the help menu.
- `--output-file (-o) FILENAME`: Set the name for the newly created file to `FILENAME`.
- `--blocks (-b) BLOCKS`: Set the number of blocks to be written to `BLOCKS`.

### Program `setkey`

The `setkey` program sets the system key to a specified value using either command-line parameters or STDIN.

**Options:**
- `--help (-h)`: Show the help menu.
- `--secret (-s)`: Take the key from STDIN while hiding the key using the setecho system call.

### Program `encr`

The `encr` program encrypts one or more files using the `encr` system call. If no file name for encryption is provided, the help menu is displayed.

**Options:**
- `--help (-h)`: Show the help menu.
- `--encrypt-all (-a)`: Encrypt all non-encrypted files in the current directory.

### Program `decr`

The `decr` program decrypts one or more files using the `decr` system call. If no file name for decryption is provided, the help menu is displayed.

**Options:**
- `--help (-h)`: Show the help menu.
- `--decrypt-all (-a)`: Decrypt all encrypted files in the current directory.
