#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define PAGE_SIZE 256
#define PAGE_MASK 0xFF
#define PAGE_SHIFT 8
#define NUM_FRAMES 256
#define NUM_PAGES 256
#define TLB_SIZE 16

typedef struct {
    int frame_number;
    int valid;
} PageTableEntry;

typedef struct {
    int page_number;
    int frame_number;
    int valid;
} TLBEntry;

unsigned char physical_memory[NUM_FRAMES * PAGE_SIZE];
PageTableEntry page_table[NUM_PAGES];
TLBEntry tlb[TLB_SIZE];
FILE* backing_store;
int page_faults = 0;
int tlb_hits = 0;
int total_addresses = 0;
int tlb_index = 0;

void init_tlb() {
    for (int i = 0; i < TLB_SIZE; i++) {
        tlb[i].valid = 0;
    }
}

void init_page_table() {
    for (int i = 0; i < NUM_PAGES; i++) {
        page_table[i].valid = 0;
        page_table[i].frame_number = -1;
    }
}

int search_tlb(int page_number, int* frame_number) {
    for (int i = 0; i < TLB_SIZE; i++) {
        if (tlb[i].valid && tlb[i].page_number == page_number) {
            *frame_number = tlb[i].frame_number;
            return 1;
        }
    }
    return 0;
}

void update_tlb_fifo(int page_number, int frame_number) {
    tlb[tlb_index].page_number = page_number;
    tlb[tlb_index].frame_number = frame_number;
    tlb[tlb_index].valid = 1;
    tlb_index = (tlb_index + 1) % TLB_SIZE;
}

void handle_page_fault(int page_number, int frame_number) {
    page_faults++;
    fseek(backing_store, page_number * PAGE_SIZE, SEEK_SET);
    fread(&physical_memory[frame_number * PAGE_SIZE], 1, PAGE_SIZE, backing_store);
    page_table[page_number].frame_number = frame_number;
    page_table[page_number].valid = 1;
}

int get_free_frame() {
    static int next_frame = 0;
    int frame = next_frame;
    next_frame = (next_frame + 1) % NUM_FRAMES;
    return frame;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s addresses.txt\n", argv[0]);
        return 1;
    }
    
    backing_store = fopen("BACKING_STORE.bin", "rb");
    if (!backing_store) {
        fprintf(stderr, "Error: Cannot open BACKING_STORE.bin\n");
        return 1;
    }
    
    init_tlb();
    init_page_table();
    
    FILE* address_file = fopen(argv[1], "r");
    if (!address_file) {
        fprintf(stderr, "Error: Cannot open %s\n", argv[1]);
        fclose(backing_store);
        return 1;
    }
    
    uint32_t raw_address;
    while (fscanf(address_file, "%u", &raw_address) == 1) {
        total_addresses++;
        uint16_t logical_address = raw_address & 0xFFFF;
        
        int page_number = (logical_address >> PAGE_SHIFT) & PAGE_MASK;
        int offset = logical_address & PAGE_MASK;
        int frame_number;
        
        if (search_tlb(page_number, &frame_number)) {
            tlb_hits++;
        } else {
            if (page_table[page_number].valid) {
                frame_number = page_table[page_number].frame_number;
            } else {
                frame_number = get_free_frame();
                handle_page_fault(page_number, frame_number);
            }
            update_tlb_fifo(page_number, frame_number);
        }
        
        int physical_address = frame_number * PAGE_SIZE + offset;
        signed char value = (signed char)physical_memory[physical_address];
        
        printf("Virtual address: %u Physical address: %d Value: %d\n", 
               logical_address, physical_address, value);
    }
    
    printf("\nStatistics:\n");
    printf("Total addresses: %d\n", total_addresses);
    printf("Page faults: %d (%.2f%%)\n", page_faults, 
           (double)page_faults / total_addresses * 100);
    printf("TLB hits: %d (%.2f%%)\n", tlb_hits, 
           (double)tlb_hits / total_addresses * 100);
    
    fclose(address_file);
    fclose(backing_store);
    return 0;
}