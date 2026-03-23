
#ifndef __NGX_FUNC_H__
#define __NGX_FUNC_H__


//设置可执行程序标题相关函数
void   ngx_init_setproctitle();
void   ngx_setproctitle(const char *title);


//和信号/主流程相关相关
int    ngx_init_signals();
void   ngx_master_process_cycle();
int    ngx_daemon();
void   ngx_process_events_and_timers();


#endif  