#include "kernel.h"

// Kernel metadata and stack allocator
KernelMetadata kernelMeta = {0};
static StackAllocator stack_allocator;

// Initialize the kernel, including stack allocator setup
void osKernelInitialize(void)
{
	//set the priority of PendSV to almost the weakest
	SHPR3 |= 0xFE << 16; //shift the constant 0xFE 16 bits to set PendSV priority
	SHPR2 |= 0xFDU << 24; //Set the priority of SVC higher than PendSV

	// Retrieve the initial value of MSP from address 0x0 (vector table)
    uint32_t* MSP_INIT_VAL = *(uint32_t**)0x0;

    // Initialize the stack allocator
    stack_allocator.stack_start = MSP_INIT_VAL;  // Start stack pool from initial MSP
    stack_allocator.current_stack = stack_allocator.stack_start;
    stack_allocator.stack_size = 0x400;          // Example: 1KB per thread stack
    stack_allocator.stack_pool_size = 0x4000;    // Example: 16KB total stack pool size

    // Initialize kernel metadata
	kernelMeta.numThreadsRunning = 0;
	kernelMeta.currentThread = 0;
}

// Allocate stack for a new thread
uint32_t* allocateStack(void) {
    // Calculate the new stack pointer by subtracting the stack size (in bytes)
    uint32_t* new_stack = (uint32_t*)((uint32_t)stack_allocator.current_stack - stack_allocator.stack_size);

    // Check if the new stack pointer is within the stack pool bounds
    if ((uint32_t)new_stack < (uint32_t)stack_allocator.stack_start - stack_allocator.stack_pool_size) {
        return NULL; // No space left
    }

    // Update the current stack pointer and return the new stack
    stack_allocator.current_stack = new_stack;
    return new_stack;
}


// Setup stack for the task
uint32_t* setup_stack(uint32_t* stack_top, void (*thread_function)(void*), void* argument) {
	*(--stack_top) = 1 << 24; // Set Thumb bit
	*(--stack_top) = (uint32_t)thread_function;
	*(--stack_top) = 0xE;
	*(--stack_top) = 0xC;
	*(--stack_top) = 0x3;
	*(--stack_top) = 0x2;
	*(--stack_top) = 0x1;
    *(--stack_top) = (uint32_t)argument; // R0: Pass the raw argument

	*(--stack_top) = 0xE;
	*(--stack_top) = 0xE;
	*(--stack_top) = 0xE;
	*(--stack_top) = 0xE;
	*(--stack_top) = 0xE;
	*(--stack_top) = 0xE;
	*(--stack_top) = 0xE;
	*(--stack_top) = 0xE;
	return stack_top;
}

// Function to find a thread by its identifier (TID)
thread* findThreadByTID(uint32_t tid) {
    for (uint8_t i = 0; i < kernelMeta.numThreadsRunning; i++) {
        if (kernelMeta.threadArray[i].TID == tid) {
            return &kernelMeta.threadArray[i];
        }
    }
    return NULL; // TID not found
}

bool osCreateThread(void (*thread_function)(void*), void* argument)
{
	if (kernelMeta.numThreadsRunning >= MAX_THREADS) {
		printf("Error: Maximum number of threads reached\r\n");
		return false;
	}

	uint32_t* stack_top = allocateStack();
	if (stack_top == NULL) {
		printf("Error: Unable to allocate stack for thread\r\n");
		return false;
	}

	stack_top = setup_stack(stack_allocator.current_stack, thread_function, argument);

	// Add the thread to the thread array
	thread* new_thread = &kernelMeta.threadArray[kernelMeta.numThreadsRunning];
	new_thread->sp = stack_top;
	new_thread->thread_function = thread_function;
	new_thread->is_active = true;
	new_thread->TID = kernelMeta.numThreadsRunning + 1;

	// Set default timeslice and runtime
	new_thread->timeslice = 5; // Default 5 ms timeslice
	new_thread->runtime = new_thread->timeslice;

    // Increment the number of threads
	kernelMeta.numThreadsRunning++;

	return true;
}

bool osCreateThreadWithDeadline(void (*thread_function)(void*), void* argument, uint32_t deadlineMs) {

	osCreateThread(thread_function, argument);

	// Set custom timeslice and runtime
	thread* new_thread = &kernelMeta.threadArray[kernelMeta.numThreadsRunning];
	new_thread->timeslice = deadlineMs;
	new_thread->runtime = new_thread->timeslice;

	return true;
}

void osKernelStart()
{
	if (kernelMeta.numThreadsRunning == 0) {
		printf("Error: No threads to start\r\n");
		return;
	}

	__asm("SVC %0" : : "I"(SVC_RUN_FIRST_THREAD));
}

void osYield()
{
	__asm("SVC %0" : : "I"(SVC_YIELD));
}

// Scheduler function for round-robin scheduling
void osSched(void) {
    // Save the current thread's stack pointer
	kernelMeta.threadArray[kernelMeta.currentThread].sp = (uint32_t*)(__get_PSP()-8*4);

    // Move to the next active thread
    do {
        kernelMeta.currentThread = (kernelMeta.currentThread + 1) % kernelMeta.numThreadsRunning;
    } while (!kernelMeta.threadArray[kernelMeta.currentThread].is_active);

    // Set PSP to the next thread's stack pointer
    __set_PSP((uint32_t)kernelMeta.threadArray[kernelMeta.currentThread].sp);
}

void SVC_Handler_Main( unsigned int *svc_args )
{
	unsigned int svc_number;
	/*
	* Stack contains:
	* r0, r1, r2, r3, r12, r14, the return address and xPSR
	* First argument (r0) is svc_args[0]
	*/
	svc_number = ( ( char * )svc_args[ 6 ] )[ -2 ] ;
	switch( svc_number )
	{
		case SVC_RUN_FIRST_THREAD:
			__set_PSP((uint32_t)kernelMeta.threadArray[0].sp);
			runFirstThread();
			break;
		case SVC_YIELD:
			//Pend an interrupt to do the context switch
			kernelMeta.threadArray[kernelMeta.currentThread].runtime = kernelMeta.threadArray[kernelMeta.currentThread].timeslice;
			_ICSR |= 1<<28;
			__asm("isb");
			break;
		default: /* unknown SVC */
		break;
	}
}



