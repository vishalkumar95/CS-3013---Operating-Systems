#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <asm-generic/current.h>
#include <asm/uaccess.h>

unsigned long **sys_call_table;

asmlinkage long (*ref_sys_cs3013_syscall1)(void);
asmlinkage long (*ref_sys_cs3013_syscall2)(unsigned short *target_pid, unsigned short *target_uid);
asmlinkage long (*ref_sys_cs3013_syscall3)(unsigned short *target_pid, unsigned short *actual_uid);
int DFS(struct task_struct *task, unsigned short *process_id, unsigned short *target_id);
unsigned short *DFS2(struct task_struct *task, unsigned short *process_id, unsigned short *target_id);
int Handle_child(struct task_struct *task, unsigned short *target_uid);

asmlinkage long new_sys_cs3013_syscall1(void) {
  	printk(KERN_INFO "\"'Hello world?!' More like 'Goodbye, world!' EXTERMINATE!\" -- Dalek");
  	return 0;
}

// System_call_2 implementation
asmlinkage long new_sys_cs3013_syscall2(unsigned short *target_pid, unsigned short *target_uid) {
	unsigned short *u_target_pid = (unsigned short*) kmalloc(sizeof(unsigned short), GFP_KERNEL);
	unsigned short *u_target_uid = (unsigned short*) kmalloc(sizeof(unsigned short), GFP_KERNEL);
	int retvalue;
	if (copy_from_user(u_target_pid, target_pid, sizeof(unsigned short))) {
		return EFAULT;
	}

	if (copy_from_user(u_target_uid, target_uid, sizeof(unsigned short))) {
		return EFAULT;
	}
	
	retvalue = DFS(&init_task, u_target_pid, u_target_uid);
	if (retvalue == 0) {
		printk("The process associated with the process id was found \n");
	}
	else {
		printk("There is no process associated with the given process id \n");
	}
	
	return retvalue;
}

// Function to traverse through the doubly linkedlist for system_call_2
int DFS(struct task_struct *task, unsigned short *process_id, unsigned short *target_uid) {
	struct task_struct *child;
	int flag = 1;
	unsigned short child_pid;

    	list_for_each_entry(child, &(task->tasks), tasks) {
		child_pid = (unsigned short) child -> pid;
		if (child_pid == *process_id) {
			printk("Process with id %d found: \n", child_pid);
			flag = Handle_child(child, target_uid);
			break;
		}
	}
	return flag;
}

// Another linkedlist traversal function for system_call_3
unsigned short *DFS2(struct task_struct *task, unsigned short *process_id, unsigned short *target_uid) {
	struct task_struct *child;
	unsigned short *b; 

    	list_for_each_entry(child, (&task->tasks),tasks) {
		unsigned short child_pid = (unsigned short) child -> pid;
		if (child_pid == *process_id) {
			printk("Process with id %d found: \n", child_pid);
			b = (unsigned short *) kmalloc(sizeof(unsigned short), GFP_KERNEL);
			*b = child->loginuid.val;
			return b;
		}
   	 }
	return NULL;
}

// Function to handle the found child process that matched the process id provided by the user
int Handle_child(struct task_struct *task, unsigned short *target_uid) {
	int uid;
	int flag = 0;
	uid = current_uid().val;
	if (uid < 1000){
		printk("This current user is identified as the root user \n");
		(task->loginuid).val = *target_uid;
	}
	else {
		if ((task->loginuid.val) == uid || (task->loginuid.val) == (-1) || (task->loginuid.val) == 65535) {
			if(*target_uid == 1001) {
				printk("Moving the user id to 1001");
				(task->loginuid).val = *target_uid;
			}
			else {
				flag = 1;
				printk("The user id can't be moved");
			}
		}
		else {
			flag = 1;
			printk("The user is not identified");
		}
	}
	return flag;	
}			

// Implementation of system_call_3
asmlinkage long new_sys_cs3013_syscall3(unsigned short *target_pid, unsigned short *actual_uid) {
	unsigned short *loguid = (unsigned short *) kmalloc(sizeof(unsigned short), GFP_KERNEL);
	unsigned short *u_target_pid = (unsigned short*) kmalloc(sizeof(unsigned short), GFP_KERNEL);

	if (copy_from_user(u_target_pid, target_pid, sizeof(unsigned short))) {
		return EFAULT;
	}

	loguid = DFS2(&init_task, u_target_pid, actual_uid);

	if (copy_to_user(actual_uid, loguid, sizeof(unsigned short))) {
		return EFAULT;
	}
	
	return 0;
}
	

static unsigned long **find_sys_call_table(void) {
  unsigned long int offset = PAGE_OFFSET;
  unsigned long **sct;
  
  while (offset < ULLONG_MAX) {
    sct = (unsigned long **)offset;

    if (sct[__NR_close] == (unsigned long *) sys_close) {
      printk(KERN_INFO "Interceptor: Found syscall table at address: 0x%02lX",
	     (unsigned long) sct);
      return sct;
    }
    
    offset += sizeof(void *);
  }
  
  return NULL;
}

static void disable_page_protection(void) {
  /*
    Control Register 0 (cr0) governs how the CPU operates.

    Bit #16, if set, prevents the CPU from writing to memory marked as
    read only. Well, our system call table meets that description.
    But, we can simply turn off this bit in cr0 to allow us to make
    changes. We read in the current value of the register (32 or 64
    bits wide), and AND that with a value where all bits are 0 except
    the 16th bit (using a negation operation), causing the write_cr0
    value to have the 16th bit cleared (with all other bits staying
    the same. We will thus be able to write to the protected memory.

    It's good to be the kernel!
  */
  write_cr0 (read_cr0 () & (~ 0x10000));
}

static void enable_page_protection(void) {
  /*
   See the above description for cr0. Here, we use an OR to set the 
   16th bit to re-enable write protection on the CPU.
  */
  write_cr0 (read_cr0 () | 0x10000);
}

static int __init interceptor_start(void) {
  /* Find the system call table */
  if(!(sys_call_table = find_sys_call_table())) {
    /* Well, that didn't work. 
       Cancel the module loading step. */
    return -1;
  }
  
  /* Store a copy of all the existing functions */
  ref_sys_cs3013_syscall1 = (void *)sys_call_table[__NR_cs3013_syscall1];
  ref_sys_cs3013_syscall2 = (void *)sys_call_table[__NR_cs3013_syscall2];
  ref_sys_cs3013_syscall3 = (void *)sys_call_table[__NR_cs3013_syscall3];

  /* Replace the existing system calls */
  disable_page_protection();

  sys_call_table[__NR_cs3013_syscall1] = (unsigned long *)new_sys_cs3013_syscall1;
  sys_call_table[__NR_cs3013_syscall2] = (unsigned long *)new_sys_cs3013_syscall2;  
  sys_call_table[__NR_cs3013_syscall3] = (unsigned long *)new_sys_cs3013_syscall3;
  
  enable_page_protection();
  
  /* And indicate the load was successful */
  printk(KERN_INFO "Loaded interceptor!");

  return 0;
}

static void __exit interceptor_end(void) {
  /* If we don't know what the syscall table is, don't bother. */
  if(!sys_call_table)
    return;
  
  /* Revert all system calls to what they were before we began. */
  disable_page_protection();
  sys_call_table[__NR_cs3013_syscall1] = (unsigned long *)ref_sys_cs3013_syscall1;
  sys_call_table[__NR_cs3013_syscall2] = (unsigned long *)ref_sys_cs3013_syscall2;
  sys_call_table[__NR_cs3013_syscall3] = (unsigned long *)ref_sys_cs3013_syscall3;
  enable_page_protection();

  printk(KERN_INFO "Unloaded interceptor!");
}

MODULE_LICENSE("GPL");
module_init(interceptor_start);
module_exit(interceptor_end);
