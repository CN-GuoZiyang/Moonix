// SBI 调用号
#ifndef _SBI_H
#define _SBI_H
#define SBI_SET_TIMER               0x0
#define SBI_CONSOLE_PUTCHAR         0x1
#define SBI_CONSOLE_GETCHAR         0x2
#define SBI_CLEAR_IPI               0x3
#define SBI_SEND_IPI                0x4
#define SBI_REMOTE_FENCE_I          0x5
#define SBI_REMOTE_SFENCE_VMA       0x6
#define SBI_REMOTE_SFENCE_VMA_ASID  0x7
#define SBI_SHUTDOWN                0x8

#define SBI_ECALL(__num, __a0, __a1, __a2)                                    \
	({                                                                    \
		register unsigned long a0 asm("a0") = (unsigned long)(__a0);  \
		register unsigned long a1 asm("a1") = (unsigned long)(__a1);  \
		register unsigned long a2 asm("a2") = (unsigned long)(__a2);  \
		register unsigned long a7 asm("a7") = (unsigned long)(__num); \
		asm volatile("ecall"                                          \
			     : "+r"(a0)                                       \
			     : "r"(a1), "r"(a2), "r"(a7)                      \
			     : "memory");                                     \
		a0;                                                           \
	})

#define SBI_ECALL_0(__num) SBI_ECALL(__num, 0, 0, 0)
#define SBI_ECALL_1(__num, __a0) SBI_ECALL(__num, __a0, 0, 0)
#define SBI_ECALL_2(__num, __a0, __a1) SBI_ECALL(__num, __a0, __a1, 0)

#endif
