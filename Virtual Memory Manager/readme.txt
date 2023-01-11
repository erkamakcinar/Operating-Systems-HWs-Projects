This assignment was prepared in macOS environment. gcc is used as compiler.

For the part 1, 
The implementation assume that the physical address space is the same size as the virtual address space,
so no page-replacement policy is needed.
I will also explain the part 2 implementation in more detail there since it covers part1 as well.

We can talk more for part 2,
The virtual memory manager uses a Translation Lookaside Buffer (TLB) and a page table to map virtual memory addresses to physical memory addresses. 
The code is written in the C programming language.
Some of the key components and functionality of the code:

*   The code defines several constants that are used throughout the program,
    such as TLB_SIZE (16), PAGES (1024), PAGE_SIZE (1024), OFFSET_BITS (10), MEMORY_SIZE (1024 * 1024) which represent the size of the TLB,
    the number of pages in the virtual address space, the size of each page, the number of bits used for the offset in the virtual address,
    and the total size of the physical memory.

*   The code also defines a struct called tlbentry that is used to store entries in the TLB.
    Each entry contains a logical page number and a physical page number.

*   The code declares two global arrays: tlb and pagetable. 
    tlb is an array of tlbentry structs and is used to store the TLB entries. 
    pagetable is an integer array that is used to store the mapping of logical page numbers to physical page numbers.

*   The code declares two other global variables tlbindex and main_memory which tlbindex keeps the number of the next empty TLB line. 
    main_memory is the actual memory array which pages are being loaded into.

*   The main function of the program, which is where the program starts executing, 
    reads command line arguments passed to the program and uses them to open the specified input file and the backing store file.

*   The main function reads lines from the input file, one at a time. 
    For each line, it extracts the virtual memory address from the line and uses bit-wise operations to extract the page number and offset from the virtual address.

*   The main function then calls search_tlb() function which is used to search the TLB for the specified logical page number. 
    If the page is found, the function returns the corresponding physical page number, otherwise, it returns -1.

*   The main function then checks the pagetable if the page is present. 
    If it's not present, it increments the page fault count and 
    calls the select_victim_frame() function to select a victim frame in the physical memory, which the page will be loaded into.

*   The selected page is then copied from the backing store to the main memory,
    and the page table and page frames tables are updated accordingly.

*   The main function also calls the add_to_tlb() function to add the mapping of the logical page number to the physical page number to the TLB, 
    using FIFO replacement policy.

*   Finally, the main function computes the physical address from the physical page number and offset, 
    and retrieves the value from the main memory at the computed physical address. 
    It then prints the virtual address, physical address, and value for the current line of the input file.

*   After reading all the lines from the input file, 
    the main function calculates and prints the number of translated addresses, page faults, TLB hits, page fault rate and TLB hit rate, 
    then it closes the opened files and memory mapped areas, and exits the program.

NOTE:
For Part 2, the code starts to generate negative addresses after a while. I couldn't solve it even though I tried.

NOTE:
The "$" symbol is used to indicate that it is a terminal command. The command is the code snippet after the "$" symbol.

Terminal commands required to compile part 1:
1. $gcc -o p1.o part1.c
Required terminal commands to run part 1:
1. Run the terminal command given below from a terminal.
$./p1.o BACKING_STORE.bin addresses.txt

Terminal commands required to compile part 2:
1. $gcc -o p2.o part2.c
Required terminal commands to run part 2:
1. Run the terminal command given below from a terminal.
$./p2.o 0 BACKING_STORE.bin addresses.txt
or
$./p2.o 1 BACKING_STORE.bin addresses.txt

