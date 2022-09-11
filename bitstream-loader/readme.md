# BITSTREAM LOADER

Functions for loading and parsing bitstream and .hwh files from Vivado

---

```c
int load_bitstream(const char* bitstream_name)
```
Loads a bitstream by calling a python script

---

```c
int c_load_bitstream(const char * bitsteam_name, int partial, bitstream_header_t** header)
```
loads a bitstream by using the fpga manager provided in xilinx linux

---

```c
typedef struct {
    char* design_name;
    char* part_name;
    char* date;
    char* time;
    uint32_t bitstream_length;
} bitstream_header_t ;

/* print in stdout */
void print_header(bitstream_header_t* header);

void free_header(bitstream_header_t* header);
```
the header structure is allocated and set by `c_load_bitstream` but mus tbe freed by the user

---

```c
uint64_t get_dma_base(const char *hwh_filename);

uint64_t get_axi_ctrl_base(const char *hwh_filename);
```
naive parse of hwh files to get axi dma base address and ip ctrl base address (not using any xml parsing libs)



