# r0e - Execute User Code in Ring0

This small header allows executing functions in your application in ring 0, i.e., with kernel privileges. 
Use cases include executing privileged instructions without a kernel module (e.g., interacting with model-specific registers) or accessing kernel data structures. 
The header requires the [PTEditor](https://github.com/misc0110/PTEditor) kernel module to be loaded. 

## Usage

The usage is fairly simple. 
Before ring-0 calls are possible, everything needs to be initialized by calling `r0e_init()`.
From then on, functions can be executed in ring 0 by providing a function pointer to `r0e_call()`. 
The provided function must not take any parameters but can return a `size_t`. 
If no ring-0 calls are needed anymore, call `r0e_cleanup()`.

## Example

There are two examples provided that are compiled when running `make`. 

### Reading MSR
The `msr.c` example uses r0e to read a model-specific register (MSR) using the privileged instruction `rdmsr`. 

### Getting Direct-physical Map Offset
The `dpm.c` examples first extracts the address of the kernel variable `page_offset_base` from the `/proc/kallsyms` pseudo file. It then uses r0e to read the value stored at this address to get the starting address of the direct-physical map (DPM). 

## Limitations
There are a few limitations that have to be kept in mind when using this library.

* Do *not* use syscalls in your ring 0 callback function. You are already running in kernel mode, jumping to the kernel using another interrupt again typically leads to undefined behavior. It might work, but mostly it results in crashes or even complete system freezes. 
* Based on the first point: only use library functions if you know that they do not use syscalls internally. 
* Avoid exceptions in your ring 0 code. Especially accessing invalid kernel addresses will lead to system freezes. Normal page faults should work in most cases. 
* Do not use `r0e_call` while you are running in ring 0. This leads to a system freeze. 
* SMAP and SMEP are automatically disabled when calling `r0e_call`, and they are never re-enabled. This is a security risk. Well, but if you use this library and/or have PTEditor loaded, your system isn't secure anyway. 
* The library is not thread-safe. 
* If KPTI is enabled, you cannot read or write kernel memory, as the kernel is unmapped in user space.
* If UMIP is enabled, r0e has to make an educated guess for the IDT location, which might not work for all kernel versions.

## Technical Details
r0e modifies the interrupt-descriptor table (IDT) and adds an entry for (software) interrupt 45. 
The respective interrupt handler is `r0e_irq_handler` that, when called, calls the provided callback function. 
To ensure that SMAP and SMEP do not prevent the execution of this user-space interrupt handler, the page containing the interrupt handler is marked as a kernel page by clearing the user-accessible bit in the page table entry. 
The interrupt handler also disables SMAP and SMEP using the bits in the CR4 register before calling the callback function. 
After the callback function returns, the handler returns with `iret`, dropping back to ring 3.

As only the IDT of the current core is modified, r0e has to ensure that the application is not migrated to a different CPU core. 
For this, `r0e_init` fixes the affinity to the currently running core. 
Moreover, the interrupt handler is locked in memory to prevent swapping of this page. 
