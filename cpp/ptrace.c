#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/reg.h> //#include <linux/user.h>
#include <sys/syscall.h>

/*
 * This is a ptrace example code works on x64 Linux
*/

const int long_size = sizeof(long);
 
void reverse(char *str)
{
  int i, j;
  char temp;
  for(i = 0, j = strlen(str) - 2; i <= j; ++i, --j) {
    temp = str[i];
    str[i] = str[j];
    str[j] = temp;
  }
}
 
void getdata(pid_t child, long addr, char *str, int len)
{
  char *laddr;
  int i, j;
  union u {
    long val;
    char chars[long_size];
  } data;

  i = 0;
  j = len / long_size;
  laddr = str;
  while(i < j) {
    data.val = ptrace(PTRACE_PEEKDATA, child, addr + i * 8, NULL);
    memcpy(laddr, data.chars, long_size);
    ++i;
    laddr += long_size;
  }
  j = len % long_size;
  if(j != 0) {
    data.val = ptrace(PTRACE_PEEKDATA, child, addr + i * 8, NULL);
    memcpy(laddr, data.chars, j);
  }
  str[len] = '\0';
 }
 
void putdata(pid_t child, long addr, char *str, int len)
{
  char *laddr;
  int i, j;
  union u {
    long val;
    char chars[long_size];
  } data;
 
 i = 0;
 j = len / long_size;
 laddr = str;
 while(i < j) {
  memcpy(data.chars, laddr, long_size);
  ptrace(PTRACE_POKEDATA, child, addr + i * 8, data.val); //i * 4
  ++i;
  laddr += long_size;
  }

  j = len % long_size;
  if(j != 0) {
    memcpy(data.chars, laddr, j);
    ptrace(PTRACE_POKEDATA, child, addr + i * 8, data.val); //i * 4
  }
}

int main()
{ 
  pid_t child;
  child = fork();
  if(child == 0) {
    ptrace(PTRACE_TRACEME, 0, NULL, NULL);
    execl("/bin/ls", "ls", NULL);
  }
  else {
    long orig_eax;
    long params[3];
    int status;
    char *str, *laddr;
    int toggle = 0;
    while(1) {
      wait(&status);
      if(WIFEXITED(status))
        break;
      orig_eax = ptrace(PTRACE_PEEKUSER, child, 8 * ORIG_RAX, NULL);
      if(orig_eax == SYS_write) {
        if(toggle == 0) {
          toggle = 1;
          params[0] = ptrace(PTRACE_PEEKUSER, child, 8 * RDI, NULL);
          params[1] = ptrace(PTRACE_PEEKUSER, child, 8 * RSI, NULL);
          params[2] = ptrace(PTRACE_PEEKUSER, child, 8 * RDX, NULL);
          str = (char *)calloc((params[2]+1), sizeof(char));
          getdata(child, params[1], str, params[2]);
          reverse(str);
          putdata(child, params[1], str, params[2]);
        }
        else {
          toggle = 0;
        }
      }
      ptrace(PTRACE_SYSCALL, child, NULL, NULL);
    }
  }
  return 0;
}