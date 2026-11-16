#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "core.h"
static LRESULT CALLBACK window_proc(HWND w,UINT m,WPARAM p,LPARAM l){
    (void)p;(void)l;
    if(m==WM_CREATE){pl_cpu_info c;char buf[512];char *stored;(void)pl_cpu_profile(&c);
        (void)snprintf(buf,sizeof(buf),"Pytyliki CPU Lab 0.4.0\n\nVendor: %s\nModel: %s\nLogical processors: %u\nFamily/model: %u/%u\n\nSafe local diagnostics only. No firmware, BIOS, voltage or clock changes.",c.vendor,c.model,c.logical_cpus,c.family,c.model_id);
        stored=(char*)GlobalAlloc(GPTR,strlen(buf)+1u);(void)SetWindowLongPtrA(w,GWLP_USERDATA,(LONG_PTR)stored);if(stored)(void)lstrcpyA(stored,buf);return 0;}
    if(m==WM_PAINT){PAINTSTRUCT ps;HDC dc=BeginPaint(w,&ps);char*s=(char*)GetWindowLongPtrA(w,GWLP_USERDATA);RECT r;(void)GetClientRect(w,&r);(void)SetBkMode(dc,TRANSPARENT);(void)DrawTextA(dc,s?s:"Pytyliki CPU Lab",-1,&r,DT_LEFT|DT_TOP|DT_WORDBREAK);EndPaint(w,&ps);return 0;}
    if(m==WM_DESTROY){char*s=(char*)GetWindowLongPtrA(w,GWLP_USERDATA);if(s)(void)GlobalFree(s);PostQuitMessage(0);return 0;}
    return DefWindowProcA(w,m,p,l);
}
int WINAPI WinMain(HINSTANCE h,HINSTANCE prev,LPSTR cmd,int show){
    WNDCLASSA wc={0};HWND w;MSG msg;(void)prev;(void)cmd;wc.lpfnWndProc=window_proc;wc.hInstance=h;wc.lpszClassName="PytylikiCpuLab";wc.hCursor=LoadCursor(NULL,IDC_ARROW);wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);
    if(!RegisterClassA(&wc))return 1;w=CreateWindowA(wc.lpszClassName,"Pytyliki CPU Lab 0.4.0",WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,720,420,NULL,NULL,h,NULL);if(!w)return 1;
    (void)ShowWindow(w,show);while(GetMessageA(&msg,NULL,0,0)>0){(void)TranslateMessage(&msg);(void)DispatchMessageA(&msg);}return (int)msg.wParam;
}
