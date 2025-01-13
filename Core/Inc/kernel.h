#ifndef KERNEL_H
#define KERNEL_H

#define SHPR2 *(uint32_t*)0xE000ED1C //for setting SVC priority, bits 31-24
#define SHPR3 *(uint32_t*)0xE000ED20 // PendSV is bits 23-16
#define _ICSR *(uint32_t*)0xE000ED04 //This lets us trigger PendSV

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "main.h" // Needed for definitions like __set_PSP

// SVC Number Definitions
#define SVC_RUN_FIRST_THREAD 0x3
#define SVC_YIELD 0x4

// Maximum number of threads supported
#define MAX_THREADS 8

// External function declaration
extern void runFirstThread(void);

typedef struct k_thread{
	uint32_t* sp; //stack pointer
	void (*thread_function)(void*); //function pointer
	bool is_active;
	uint32_t TID;
    uint32_t timeslice;        // Maximum runtime in ms
    uint32_t runtime;          // Remaining runtime in ms
}thread;

// Kernel Metadata Structure
typedef struct {
    thread threadArray[MAX_THREADS];  // Array of threads
    uint8_t currentThread;           // Index of the currently running thread
    uint8_t numThreadsRunning;       // Number of threads currently created
} KernelMetadata;

extern KernelMetadata kernelMeta;

// Struct for stack allocation information
typedef struct {
    uint32_t* stack_start;    // Starting address of the global stack pool
    uint32_t* current_stack;  // Pointer to the current stack allocation
    uint32_t stack_size;      // Size of each stack in bytes
    uint32_t stack_pool_size; // Total size of the stack pool
} StackAllocator;

// Kernel API function prototypes
void osKernelInitialize(void);                         // Initialize the kernel
uint32_t* allocateStack(void);                         // Allocate memory for a thread stack
uint32_t* setup_stack(uint32_t* stack_top, void (*thread_function)(void*), void* argument); // Set up a thread's stack
bool osCreateThread(void (*thread_function)(void*), void* argument);
bool osCreateThreadWithDeadline(void (*thread_function)(void*), void* argument, uint32_t deadlineMs);
void osKernelStart(void);// Create and run a new thread
void osYield(void);

#endif // KERNEL_H
