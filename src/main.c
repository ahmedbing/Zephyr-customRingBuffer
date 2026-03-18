#include <zephyr/kernel.h>
#include "zephyr/sys/printk.h"

int main(void)
{
        printk("Hello, World!\n");
        while(1){
                printk("Running...\n");
                k_sleep(K_SECONDS(5));
        }
        return 0;
}
