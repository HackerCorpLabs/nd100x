 # 🛠️ Risc-V linux Build Setup

This project supports building to **Risc-V** for running in Linux on a tiny Risc-V device.

This is tested on the $5 Milk-V Duo product: https://milkv.io/


## ✅ Prerequisites

The build requires the Milk-V Duo toolchain.

Follow [this description](https://milkv.io/docs/duo/application-development/tdl-sdk/tdl-sdk-introduction#tdl-sdk-examples-for-v1-os-image) to download and install the toolchain.
 

You need to manually download the toolchain [host-tools](https://sophon-file.sophon.cn/sophon-prod-s3/drive/23/03/07/16/host-tools.tar.gz)  and extract it to the SDK root directory:

```bash
cd ~
wget https://sophon-file.sophon.cn/sophon-prod-s3/drive/23/03/07/16/host-tools.tar.gz
tar xvf host-tools.tar.gz
cd host tools
export PATH=$PATH:$(pwd)/gcc/riscv64-linux-musl-x86_64/bin

```

- I used /home/<your-home/>milkv/host-tools as the SDK path.
- Full path: /home/<your-hom>/milkv/host-tools/gcc/riscv64-linux-musl-x86_64/bin
- You can add it to your PATH with:

```
export PATH=$PATH:/home/<your-home>/milkv/host-tools/gcc/riscv64-linux-musl-x86_64/bin
```

## ✅  Build

To build for RISC-V:

```
make riscv
```

This will use the toolchain file `riscv64-toolchain.cmake` which is configured to use:
- The compiler at the above path
- Debug flags: `-march=rv64gc -mabi=lp64d -g -O0`

### ✅  Build Configurations

By default, the build uses these compiler flags:
- `-march=rv64gc`: RISC-V 64-bit with general extensions and compressed instructions
- `-mabi=lp64d`: Long and pointer 64-bit, double-precision floating point ABI
- `-g`: Include debug symbols 
- `-O0`: No optimization (makes debugging easier)

Other compiler toolchains that has been tested to compile but is not the correct for the Milk-V Duo board (but maybe a better choice for other devices)
 - gcc-riscv64-linux-gnu
 - gcc-riscv64-unknown-elf

### ✅  DAP Support

The RISC-V build includes DAP (Debug Adapter Protocol) support, enabling debugging with compatible tools. The build automatically:
- Links with the local cJSON library
- Creates the appropriate include directory structure
- Disables daplib test tools (debugger and mock server) for RISC-V 

### ✅  Custom Build

For a custom build with different compiler flags or configuration:

```bash
# Example: Build with optimization level 2
mkdir -p build_riscv_opt
cd build_riscv_opt
PATH="$PATH:/home/ronny/milkv/host-tools/gcc/riscv64-linux-musl-x86_64/bin" \
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_RISCV=ON \
      -DCMAKE_TOOLCHAIN_FILE=../riscv64-toolchain.cmake \
      -DCMAKE_C_FLAGS="-march=rv64gc -mabi=lp64d -O2" ..
make -j$(nproc)
```

## 📱 Milk-V Duo Deployment

Getting started: https://milkv.io/docs/duo/getting-started  
Documentation: https://milkv.io/docs  
GitHub: https://github.com/milkv-duo  
Example environment setup: https://github.com/milkv-duo/duo-examples/blob/main/envsetup.sh  

### Deploying to the Device

Connect to the device:
```bash
ssh root@192.168.42.1
mkdir -p nd
```

Copy the executable and disk images:
```bash
scp build_riscv/bin/nd100x images/SMD0.IMG FLOPPY.IMG root@192.168.42.1:/root/nd
```

Or just update the executable:
```bash
scp build_riscv/bin/nd100x root@192.168.42.1:/root/nd
```


## Memory Limits

The Milk-V Duo with 64MB RAM seems to not like that I compiled nd100x using 16MB of RAM. It was killed of withing a few seconds.

The solution was to recompile the emulator to use only 8MB of RAM.

### Debugging tips

Check DMSG
- dmesg | tail -n 50

Look for lines like
- Out of memory: Kill process 1234 (your_app) score X or sacrifice child
- Killed process 1234 (your_app)

Actual log:
- [  651.534797] Out of memory: Killed process 330 (nd100x) total-vm:18480kB, anon-rss:10492kB, file-rss:4kB, shmem-rss:0kB, UID:0 pgtables:44kB oom_score_adj:0
- [  651.555582] oom_reaper: reaped process 330 (nd100x), now anon-rss:0kB, file-rss:0kB, shmem-rss:0kB
- [  651.555648] nd100x[330]: unhandled signal 7 code 0x2 at 0x0000000000a85000 in ld-musl-riscv64.so.1[3fcc2eb000+97000]