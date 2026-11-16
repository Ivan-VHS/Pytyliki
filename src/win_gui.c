#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "core.h"
#include <stdio.h>
static LRESULT CALLBACK wndproc(HWND w,UINT m,WPARAM p,LPARAM l){
  (void)p;(void)l;
  if(m==WM_CREATE){
    pl_cpu_info c; char buf[512]; pl_cpu_profile(&c);
    snprintf(buf,sizeof(buf),"Pytyliki CPU Lab 0.4.0\n\nVendor: %s\nLogical processors: %u\nFamily / model: %u / %u\n\nSafe local diagnostics only. No firmware, BIOS, voltage or clock changes.",c.vendor,c.logical_cpus,c.family,c.model_id);
    SetWindowLongPtrA(w,GWLP_USERDATA,(LONG_PTR)GlobalAlloc(GPTR,strlen(buf)+1));
    char *stored=(char*)GetWindowLongPtrA(w,GWLP_USERDATA); if(stored)lstrcpyA(stored,buf);
    return 0;
  }
  if(m==WM_PAINT){PAINTSTRUCT ps;HDC dc=BeginPaint(w,&ps);char *s=(char*)GetWindowLongPtrA(w,GWLP_USERDATA);RECT r;GetClientRect(w,&r);SetBkMode(dc,TRANSPARENT);DrawTextA(dc,s?s:"Pytyliki CPU Lab", -1,&r,DT_LEFT|DT_TOP|DT_WORDBREAK);EndPaint(w,&ps);return 0;}
  if(m==WM_DESTROY){char *s=(char*)GetWindowLongPtrA(w,GWLP_USERDATA);if(s)GlobalFree(s);PostQuitMessage(0);return 0;}
  return DefWindowProcA(w,m,p,l);
}
int WINAPI WinMain(HINSTANCE h,HINSTANCE prev,LPSTR cmd,int show){
  (void)prev;(void)cmd;WNDCLASSA wc={0};wc.lpfnWndProc=wndproc;wc.hInstance=h;wc.lpszClassName="PytylikiCpuLab";wc.hCursor=LoadCursor(NULL,IDC_ARROW);wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);
  if(!RegisterClassA(&wc))return 1;HWND w=CreateWindowA(wc.lpszClassName,"Pytyliki CPU Lab 0.4.0",WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,680,360,NULL,NULL,h,NULL);if(!w)return 1;ShowWindow(w,show);MSG msg;while(GetMessageA(&msg,NULL,0,0)>0){TranslateMessage(&msg);DispatchMessageA(&msg);}return (int)msg.wParam;
}
