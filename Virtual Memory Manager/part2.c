
/**
 * virtmem.c
 */

#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TLB_SIZE 16
#define PAGES 1024
#define PAGE_MASK 1047552 /* TODO  1047552 */

#define PAGE_SIZE 1024
#define OFFSET_BITS 10
#define OFFSET_MASK 1023 /* TODO */

#define MEMORY_SIZE PAGES *PAGE_SIZE

// Max number of characters per line of input file to read.
#define BUFFER_SIZE 10

#define PAGE_FRAMES 256

#define FIFO 0
#define LRU 1

struct tlbentry
{
    unsigned char logical;
    unsigned char physical;
    time_t timestamp;
};

struct pageframe
{
    unsigned char logical;
    time_t timestamp;
};

// TLB is kept track of as a circular array, with the oldest element being overwritten once the TLB is full.
struct tlbentry tlb[TLB_SIZE];

// number of inserts into TLB that have been completed. Use as tlbindex % TLB_SIZE for the index of the next TLB line to use.
int tlbindex = 0;

// pagetable[logical_page] is the physical page number for logical page. Value is -1 if that logical page isn't yet in the table.
int pagetable[PAGES];

signed char main_memory[PAGE_FRAMES * PAGE_SIZE];

// Pointer to memory mapped backing file
signed char *backing;

int next_free_frame = 0;
int replacement_policy = FIFO;

struct pageframe pageframes[PAGE_FRAMES];

int max(int a, int b)
{
    if (a > b)
        return a;
    return b;
}

/* Returns the physical address from TLB or -1 if not present. */
int search_tlb(unsigned char logical_page)
{
    int i;
    for (i = 0; i < TLB_SIZE; i++)
    {
        if (tlb[i].logical == logical_page)
        {
            tlb[i].timestamp = time(NULL);
            return tlb[i].physical;
        }
    }
    return -1;
}

/* Adds the specified mapping to the TLB, replacing the oldest mapping (FIFO replacement). */
void add_to_tlb(unsigned char logical, unsigned char physical)
{
    tlb[tlbindex % TLB_SIZE].logical = logical;
    tlb[tlbindex % TLB_SIZE].physical = physical;
    tlb[tlbindex % TLB_SIZE].timestamp = time(NULL);
    tlbindex++;
}

int search_pageframe(unsigned char logical_page)
{
    int i;
    for (i = 0; i < PAGE_FRAMES; i++)
    {
        if (pageframes[i].logical == logical_page)
        {
            return i;
        }
    }
    return -1;
}

int select_victim_frame()
{
    if (replacement_policy == FIFO)
    {
        int i;
        int victim_frame = -1;
        time_t oldest_timestamp = time(NULL);
        for (i = 0; i < PAGE_FRAMES; i++)
        {
            if ((int)(pageframes[i].logical) != -1)
            {
                if (pageframes[i].timestamp < oldest_timestamp)
                {
                    oldest_timestamp = pageframes[i].timestamp;
                    victim_frame = i;
                }
            }
        }
        return victim_frame;
    }
    else if (replacement_policy == LRU)
    {
        int i;
        int victim_frame = -1;
        time_t oldest_timestamp = time(NULL);
        for (i = 0; i < PAGE_FRAMES; i++)
        {
            if ((int)(pageframes[i].logical) != -1)
            {
                if (pageframes[i].timestamp < oldest_timestamp)
                {
                    oldest_timestamp = pageframes[i].timestamp;
                    victim_frame = i;
                }
            }
        }
        return victim_frame;
    }
    else
    {
        fprintf(stderr, "Invalid replacement policy selected\n");
        exit(1);
    }
}

int main(int argc, const char *argv[])
{
    if (argc != 4)
    {
        fprintf(stderr, "Usage: ./virtmem -p [policy] backingstore input\n");
        exit(1);
    }
    replacement_policy = atoi(argv[1]);
    if (replacement_policy != FIFO && replacement_policy != LRU)
    {
        fprintf(stderr, "Invalid replacement policy selected\n");
        exit(1);
    }

    const char *backing_filename = argv[2];
    int backing_fd = open(backing_filename, O_RDONLY);
    backing = mmap(0, MEMORY_SIZE, PROT_READ, MAP_PRIVATE, backing_fd, 0);

    const char *input_filename = argv[3];
    FILE *input_fp = fopen(input_filename, "r");

    // Fill page table entries with -1 for initially empty table.
    int i;
    for (i = 0; i < PAGES; i++)
    {
        pagetable[i] = -1;
    }

    // Initialize pageframes
    for (i = 0; i < PAGE_FRAMES; i++)
    {
        pageframes[i].logical = -1;
    }

    // Character buffer for reading lines of input file.
    char buffer[BUFFER_SIZE];

    // Data we need to keep track of to compute stats at end.
    int total_addresses = 0;
    int tlb_hits = 0;
    int page_faults = 0;

    // Read lines from input file.
    while (fgets(buffer, BUFFER_SIZE, input_fp))
    {
        // Extract virtual address from input line.
        int virtual_address = atoi(buffer);

        // Extract page number and offset from virtual address.
        int page_number = (virtual_address & PAGE_MASK) >> OFFSET_BITS;
        int page_offset = virtual_address & OFFSET_MASK;

        if((int)next_free_frame == 256) {
            int c = 0;
        }
        
        // Check TLB for page number.
        int physical_page = search_tlb(page_number);

        if (physical_page == -1)
        {

            // Check page table for page number.
            physical_page = pagetable[page_number];
            if (physical_page == -1)
            {
                // Page fault.
                page_faults++;

                // Find next free frame or victim frame to replace.
                if (next_free_frame < PAGE_FRAMES)
                {
                    physical_page = next_free_frame;
                    next_free_frame++;
                }
                else
                {
                    physical_page = select_victim_frame();
                    // Update page table
                    pagetable[pageframes[physical_page].logical] = -1;
                }
                // Copy page from backing store to memory
                memcpy(main_memory + physical_page * PAGE_SIZE, backing + page_number * PAGE_SIZE, PAGE_SIZE);
                // Add to page table
                pagetable[page_number] = physical_page;
                // Update pageframe
                pageframes[physical_page].logical = page_number;
                pageframes[physical_page].timestamp = time(NULL);
            }
            // Add page number to TLB.
            add_to_tlb(page_number, physical_page);
        }
        else
        {
            tlb_hits++;
        }
        // Compute physical address from page number and offset.
        int physical_address = (physical_page * PAGE_SIZE) + page_offset;
        // Retrieve value from memory at computed physical address.
        signed char value = main_memory[physical_address];
        printf("Virtual address: %d Physical address: %d Value: %d\n",
               virtual_address, physical_address, value);
        
        // Page not found in TLB.
        total_addresses++;
    }
    printf("Number of Translated Addresses = %d\n", total_addresses);
    printf("Page Faults = %d\n", page_faults);
    printf("Page Fault Rate = %.3f\n", (double)page_faults / total_addresses);
    printf("TLB Hits = %d\n", tlb_hits);
    printf("TLB Hit Rate = %.3f\n", (double)tlb_hits / total_addresses);
    fclose(input_fp);
    munmap(backing, MEMORY_SIZE);
    return 0;
}
