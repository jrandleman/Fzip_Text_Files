# Fz1p.c
## _Lossless Text Compression Via Substring Substitution & Bit Packing!_
---------------------------------------
## Compilation and Initialization:
\<COMPILE> `$ gcc -o fz1p fz1p.c`</br>
\<COMPRESSOR> `$ ./fz1p filename.txt`</br>
\<DECOMPRESS> `$ ./fz1p filename.txt.FZ1P`

* Enter `-l` to print compression information during run:
  * `$ ./fz1p -l ...`
* Try compressing the `Macbeth.txt` sample file provided!
  
---------------------------------------
## Compression Information:
| AVG COMPRESSION | ORIGINAL FILE | COMPRESSED FILE |
|:---------------:|:-------------:|:---------------:|
|       50%       |      TXT      |       BIN       |

## Compression Technique:
1. Replaces Chars of the Following String w/ Keys Generated Using the "Reserved Char Sequences":
   * Thus Spaces, the Lowercase Alphabet, and `.,!?-` are the Only Printable Chars Left in the File
 ```c
 "\"#$%&()*+,/0123456789:;<=>@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`{|}~\n\t"
 ```
2. All Spaces are Converted to Underscores
3. Abbreviates Commonly Repeated Substrings Found in the File
4. W/ Only 32 Different Printable Characters Left (Step 1), Each are Bit-Packed Into 5 Bits Rather Than 8
5. ***Mind = Blown***
---------------------------------------
## 3 Reserved Character Sequences for Compression:
* "`qx`", "`qy`", _&_ "`qz`" _are **NOT** to be used in any_ `.txt` _being compressed (lower **or** uppercase!)_
* `fz1p.c` _preemptively scans files to warn the client if any of the above are found_
