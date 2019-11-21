//
//  my_container.c
//  
//
//  Created by Евдокимов Миша on 21.11.2019.
//
#define _GNU_SOURCE
#include <string.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#define STACK_SIZE (100 * 1024 * 1024)

static char child_stack[STACK_SIZE];

static int child_fn(void* arg) {
    
    printf("Clone: %ld\n", (long) getpid());
    pid_t child_pid = fork();
    
    if(child_pid) {
        
        printf("Clone PID: %ld\n", (long) child_pid);
        
        // here we change the privacy
        system("mount --make-rprivate -o remount /");
        system("mount -t  proc proc /proc --make-private");
        
        // here we configure ips
        system("ifconfig veth1 10.1.1.2 up");
        
        // here we create an image
        system("dd if=/dev/zero of=image.fs count=1024 bs=1048675");
        
        // here we unmount th disk device
        system("losetup -d /dev/loop2");
        
        // mount image of fs to disk device
        system("losetup /dev/loop2 image.fs");
        
        // here we crate the fs
        system("mkfs image.fs");
        
        // here we mount ext2 root node of the fs to /home dir
        system("mount -t ext2 /dev/loop2 /home --make-private");
        
        // here we create test file to test if the fs works
        system("dd if=/dev/zero of=/home/test_file.txt count=1024 bs=1048675");
        
        printf("Files of the children:\n");
        system("ls");
        
        // here you can type some commands to test
        system("bash");
        
    }
    
    return 0;
    
}


int main() {
    
    pid_t child_pid = clone(child_fn, child_stack + STACK_SIZE, CLONE_NEWPID | SIGCHLD | CLONE_NEWNS | CLONE_NEWNET, NULL);
    
    printf("clone() = %ld\n", (long) child_pid);
    
    // create ip links for parent and child
    char ip[1024*sizeof(char)];
    
    sprintf(ip, "ip link add name veth0 type veth peer name veth1 netns %ld", (long) child_pid);
    system(ip);
    
    // configure ip
    system("ifconfig veth0 10.1.1.1/24 up");
    
    // call execution of child process
    waitpid(child_pid, NULL, 0);
    
    printf("The files of the parent:\n");
    system("ls");
    
    return 0;

}
