# shell-xv6

Implementation of Shell for xv6 OS.

##### Install all the required packages:
```
sudo apt update && \
sudo apt -y install build-essential gdb \
     coreutils util-linux sysstat procps wget tar \
     qemu

```

##### Clone and initialize xv6:
```
wget https://www.cse.iitb.ac.in/~mythili/os/labs/xv6-public.tgz && \
tar -xf xv6-public.tgz && \
cd xv6-public && \
make
```

##### Run xv6:
```
cd xv6-public
make qemu-nox
```

Once xv6 is setup, replace files from `src` directory to `xv6-public`.
```
mv src/* xv6-public/
``` 

### References:

- xv6 printf function:
  - https://stackoverflow.com/questions/49971604/how-does-xv6-write-to-the-terminal

- To read from file:
  - https://stackoverflow.com/questions/42061824/unix-in-c-trying-to-write-tail-for-xv6