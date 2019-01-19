# btrcompressor

Compress files and directories recursively on btrfs.

To compress `/usr` with the compression algorithm `zstd` just run:

```
$ sudo btrcompressor -c zstd /usr
$ sudo btrfs file defrag -r /usr
```

Warning: read the man page of *btrfs-file defrag*! There is a note regarding reflinks.

Check the file sizes with the [compsize](https://github.com/kilobyte/compsize) tool:
```
# compsize /usr
Processed 266688 files, 140824 regular extents (141972 refs), 147753 inline.
Type       Perc     Disk Usage   Uncompressed Referenced  
TOTAL       66%      5.2G         7.7G         7.9G       
none       100%      3.7G         3.7G         3.7G       
zstd        36%      1.4G         4.0G         4.1G       
```

Not all files are compressed, because forced compression is turned off.

## Usage

```
Usage: btrcompressor [-v] [-j <threads>] <-c compression | -d>
       -c <compression> ... choose the compression to set (zlib,lzo,zstd)
       -d               ... disable compression
       -j <threads>     ... number of threads to use (not implemented)
       -v               ... verbose output
```

## Install

```
git clone https://bitbucket.org/chtisgit/btrcompressor
cd btrcompressor
git submodule update --init
make
cp btrcompressor /usr/local/bin
```


