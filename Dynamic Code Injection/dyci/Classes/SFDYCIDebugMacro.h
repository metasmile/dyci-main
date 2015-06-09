//
//  DYCIMacro.h
//  dyci-example
//
//  Created by metasmile(cyrano905@gmail.com) on 2015. 6. 4.
//  Copyright 2011 Stanfy, LLC. All rights reserved.
//

#import <objc/runtime.h>
#import <mach/mach_time.h>

/*
A handy macro when working revert to original frequently + catch exceptions for protecting test cycle.
You can use this macro instead of overriding 'updateOnClassInjection'.

1. Define some code 'BEGIN_INJECTION ~ END_INJECTION', 'BEGIN_REJECTION ~  END_REJECTION'.
if line of 'code' is being exist, DYCI will call BEGIN_INJECTION block.

        BEGIN_INJECTION
            ... code ...
        END_INJECTION

        BEGIN_REJECTION
            ... revert to original state ...
        END_REJECTION

2. Simulator is still running. Change 'INJECTION' block to empty line if you want it to revert.
Then DYCI will call 'BEGIN_REJECTION ~ END_REJECTION' block. And repeat.

    BEGIN_INJECTION
    END_INJECTION
 */

#define BEGIN_INJECTION - (void)updateOnClassInjection{; \
    NSLog(@"----------------- Injecting ... -----------------");\
    double __exeinterval__ = DYCI_CHECK_DELAY(^{\
        int __linebegin__ = __LINE__;\
        BOOL __raised__ = NO;\
        @try{\

#define END_INJECTION \
        }@catch(NSException *iex){\
            NSLog(@"[!] Injecting error: %@",iex);\
            __raised__ = YES;\
        }\
        SEL __selector_injection__ = NSSelectorFromString(@"__updateOnClassRejection");\
        BOOL __exist_method__ = (BOOL) class_getInstanceMethod(self.class, __selector_injection__);\
        if((__raised__ || __LINE__-__linebegin__==1) && __exist_method__){\
            @try{ \
                NSLog(@"----------------- Rejecting ... -----------------");\
                ((void (*)(id, SEL))[self methodForSelector:__selector_injection__])(self, __selector_injection__);\
            }@catch(NSException *rex){\
                 NSLog(@"[!] Rejecting error: %@",rex);\
                 __raised__ = YES;\
            }\
        }\
    });\
    NSLog(@"================= Complete [%lfs] =================",__exeinterval__);\
    }

#define BEGIN_REJECTION - (void)__updateOnClassRejection{;\

#define END_REJECTION }


/*
Utils
 */
static inline double
DYCI_CHECK_DELAY(void (^block)(void))
{
    mach_timebase_info_data_t info;
    if (mach_timebase_info(&info) != KERN_SUCCESS) return -1.0;

    uint64_t start = mach_absolute_time ();
    block();
    uint64_t end = mach_absolute_time ();
    uint64_t elapsed = end - start;
    uint64_t nanos = elapsed * info.numer / info.denom;
    return (double)nanos / NSEC_PER_SEC;
}
