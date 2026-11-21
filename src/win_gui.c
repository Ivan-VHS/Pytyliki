#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "core.h"
#include "processor.h"
#include "dsp.h"
#include "metric_geometry.h"
#include "metric_registry.h"
#include "embedded.h"

#ifndef WM_DPICHANGED
#define WM_DPICHANGED 0x02E0
#endif

#define ID_PROFILE 1001
#define ID_SUITE 1002
#define ID_REVIEW 1003
#define ID_LOAD_WAV 1004
#define ID_TONE 1005
#define ID_LOWPASS 1006
#define ID_COMPRESS 1007
#define ID_SATURATE 1008
#define ID_MEASURE 1009
#define ID_EXPORT 1010
#define ID_GAIN 1011
#define ID_CLEAR 1012
#define ID_SAVE_REPORT 1013
#define ID_METRIC_LAB 1014
#define ID_EMBEDDED 1015
#define ID_THEME 1016
#define ID_CUTOFF 1101
#define ID_THRESHOLD 1102
#define ID_RATIO 1103
#define ID_DRIVE 1104
#define ID_GAIN_VALUE 1105

static HWND g_log;
static HWND g_main;
static HFONT g_font;
static HFONT g_heading_font;
static HFONT g_log_font;
static HBRUSH g_page_brush, g_input_brush, g_log_brush;
static int g_dark_mode = 0;

static COLORREF page_color(void) { return g_dark_mode ? RGB(10,30,70) : RGB(244,246,250); }
static COLORREF header_color(void) { return RGB(0,57,166); }
static COLORREF text_color(void) { return g_dark_mode ? RGB(255,255,255) : RGB(10,30,70); }
static COLORREF input_color(void) { return g_dark_mode ? RGB(0,57,166) : RGB(255,255,255); }
static COLORREF log_color(void) { return RGB(10,30,70); }
static COLORREF blue_color(void) { return RGB(0,57,166); }
static COLORREF blue_hover_color(void) { return RGB(10,30,70); }
static COLORREF red_color(void) { return RGB(181,26,16); }
static COLORREF red_hover_color(void) { return RGB(213,43,30); }
static COLORREF border_color(void) { return g_dark_mode ? RGB(255,255,255) : RGB(0,57,166); }
static COLORREF button_ink(int primary, int danger) {
    if (primary || danger) return RGB(255,255,255);
    return text_color();
}
static int primary_button(UINT id) {
    return id==ID_PROFILE || id==ID_SUITE || id==ID_REVIEW || id==ID_SAVE_REPORT;
}
static void request_dpi_awareness(void) {
    HMODULE user32=GetModuleHandleA("user32.dll");
    if (user32) {
        typedef BOOL (WINAPI *dpi_context_fn)(HANDLE);
        dpi_context_fn set_context=(dpi_context_fn)GetProcAddress(user32,"SetProcessDpiAwarenessContext");
        if (set_context) { (void)set_context((HANDLE)(LONG_PTR)-4); return; }
    }
    SetProcessDPIAware();
}
static void rebuild_brushes(void) {
    if (g_page_brush) DeleteObject(g_page_brush);
    if (g_input_brush) DeleteObject(g_input_brush);
    if (g_log_brush) DeleteObject(g_log_brush);
    g_page_brush = CreateSolidBrush(page_color());
    g_input_brush = CreateSolidBrush(input_color());
    g_log_brush = CreateSolidBrush(log_color());
}
static pl_audio_buffer g_audio = {0};

static void log_text(const char *text) {
    if (!g_log || !text) return;
    SendMessageA(g_log, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
    SendMessageA(g_log, EM_REPLACESEL, FALSE, (LPARAM)text);
    SendMessageA(g_log, EM_SCROLLCARET, 0, 0);
}
static void log_format(const char *fmt, ...) {
    char buffer[2048]; va_list args; va_start(args, fmt);
    (void)_vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, fmt, args); buffer[sizeof(buffer)-1] = '\0';
    va_end(args); log_text(buffer);
}
static void apply_font(HWND control) { if (g_font) SendMessageA(control, WM_SETFONT, (WPARAM)g_font, TRUE); }
static HWND add_label(HWND parent, const char *label, int id) {
    HWND h=CreateWindowExA(0,"STATIC",label,WS_CHILD|WS_VISIBLE|SS_LEFT,0,0,10,10,parent,(HMENU)(INT_PTR)id,GetModuleHandleA(NULL),NULL);
    apply_font(h); return h;
}
static HWND add_edit(HWND parent, const char *value, int id) {
    HWND h=CreateWindowExA(WS_EX_CLIENTEDGE,"EDIT",value,WS_CHILD|WS_VISIBLE|WS_TABSTOP|ES_AUTOHSCROLL|ES_LEFT,0,0,10,10,parent,(HMENU)(INT_PTR)id,GetModuleHandleA(NULL),NULL);
    apply_font(h); return h;
}
static double read_parameter(int id, double fallback) {
    char text[64], *end=NULL; GetWindowTextA(GetDlgItem(g_main,id),text,(int)sizeof(text));
    double value=strtod(text,&end);
    if (!text[0] || !end || end==text || *end || !isfinite(value)) return fallback;
    return value;
}
static HWND add_button(HWND parent, const char *label, int id) {
    HWND h=CreateWindowExA(0,"BUTTON",label,WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_OWNERDRAW|BS_NOTIFY,0,0,10,10,parent,(HMENU)(INT_PTR)id,GetModuleHandleA(NULL),NULL);
    apply_font(h); return h;
}
static void show_metric_lab(void) {
    const double profile[] = {3.0, 4.0, 12.0};
    const double weights[] = {1.0, 2.0, 0.5};
    const double sample[] = {2.0, 4.0, 6.0, 8.0};
    double l2 = 0.0, l1 = 0.0, weighted = 0.0, mean = 0.0, variance = 0.0;
    if (pl_metric_norm(profile, 3u, &l2) != 0 ||
        pl_metric_l1_norm(profile, 3u, &l1) != 0 ||
        pl_metric_weighted_l2_norm(profile, weights, 3u, &weighted) != 0 ||
        pl_metric_mean_variance(sample, 4u, 0, &mean, &variance) != 0) {
        log_text("[METRIC LAB] Numerical operation failed validation.\r\n");
        return;
    }
    log_format("\r\n[METRIC LAB] L2=%.6g | L1=%.6g | weighted L2=%.6g\r\n", l2, l1, weighted);
    log_format("[METRIC LAB] population mean=%.6g | variance=%.6g | registered operations=%lu\r\n",
        mean, variance, (unsigned long)pl_metric_registry_count());
    log_text("Metric Lab calls the shared C11 kernel; it does not duplicate formulas in the GUI.\r\n");
}
static void show_profile(void) {
    pl_processor_snapshot p;
    if (pl_processor_scan(&p)!=0) { log_text("[CPU SCAN] Processor capability scan failed.\r\n"); return; }
    log_format("\r\n[PROCESSOR PROFILE] %s / %s\r\nVendor: %s | logical threads: %u | family/model: %u/%u\r\nPhysical memory: %llu MiB | available: %llu MiB\r\n",
         p.architecture,p.base.model,p.base.vendor,p.base.logical_cpus,p.base.family,p.base.model_id,
         (unsigned long long)p.physical_memory_mib,(unsigned long long)p.available_memory_mib);
    log_format("ISA: SSE2=%u SSE4.2=%u AVX-HW=%u OSXSAVE=%u AVX-OS=%u AVX-usable=%u AVX2-usable=%u AVX512F-usable=%u AES=%u FMA-usable=%u | virtualization=%u\r\n",
         p.sse2,p.sse42,p.avx_hw,p.osxsave,p.avx_os_state,p.avx,p.avx2,p.avx512f,p.aes,p.fma,p.virtualization);
    log_text("AVX-family usable flags include OS-managed XCR0 state validation. Capability reporting does not execute SIMD instructions or modify firmware/clock settings.\r\n");
}
static void run_suite(void) {
    pl_suite_result r;
    int rc=pl_processor_benchmark_suite(UINT64_C(12000000),8,4,&r);
    if(rc!=0) { log_format("[BENCHMARK] Suite failed (%d).\r\n",rc); return; }
    log_format("\r\n[PROCESSOR SUITE] integer %.2f ms | memory %.2f ms (%u MiB x %u passes) | matrix %.2f ms\r\nchecksum=%llu | integrity=%s\r\n",
         r.integer_ms,r.memory_ms,r.memory_mib,r.passes,r.matrix_ms,(unsigned long long)r.checksum,r.integrity_ok?"PASS":"CHECK");
    log_text("Scores are local comparative measurements, not processor certification. Close other high-load applications for more repeatable runs.\r\n");
}
static int choose_file(char *path,size_t cap,int save) {
    OPENFILENAMEA ofn; memset(&ofn,0,sizeof(ofn)); path[0]='\0';
    ofn.lStructSize=sizeof(ofn); ofn.hwndOwner=GetActiveWindow(); ofn.lpstrFile=path; ofn.nMaxFile=(DWORD)cap;
    ofn.lpstrFilter="WAV audio (*.wav)\0*.wav\0C/C++/Rust source (*.c;*.h;*.cpp;*.rs)\0*.c;*.h;*.cpp;*.rs\0All files (*.*)\0*.*\0";
    ofn.nFilterIndex=save?1:3; ofn.lpstrDefExt=save?"wav":NULL; ofn.Flags=OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR|(save?(OFN_OVERWRITEPROMPT|OFN_EXPLORER):(OFN_FILEMUSTEXIST|OFN_EXPLORER));
    return save?GetSaveFileNameA(&ofn):GetOpenFileNameA(&ofn);
}
static int choose_report(char *path,size_t cap) {
    OPENFILENAMEA ofn; memset(&ofn,0,sizeof(ofn)); path[0]='\0';
    ofn.lStructSize=sizeof(ofn); ofn.hwndOwner=g_main; ofn.lpstrFile=path; ofn.nMaxFile=(DWORD)cap;
    ofn.lpstrFilter="JSON diagnostic report (*.json)\0*.json\0All files (*.*)\0*.*\0";
    ofn.nFilterIndex=1; ofn.Flags=OFN_PATHMUSTEXIST|OFN_OVERWRITEPROMPT|OFN_EXPLORER;
    return GetSaveFileNameA(&ofn);
}
static void save_report(void) {
    char path[1024]; if(!choose_report(path,sizeof(path))) return;
    pl_cpu_info cpu; pl_benchmark bench={0}; pl_analysis analysis={0};
    if(pl_cpu_profile(&cpu)!=0 || pl_run_benchmark(UINT64_C(10000000),&bench)!=0 || pl_write_json(path,&cpu,&bench,&analysis)!=0) {
        log_format("[REPORT] Could not create report: %s\r\n",path); return;
    }
    log_format("[REPORT] Saved CPU snapshot and bounded benchmark JSON: %s\r\n",path);
}
static void review_source(void) {
    char path[1024]; if(!choose_file(path,sizeof(path),0)) return;
    pl_analysis a={0}; char details[8192]={0};
    if(pl_analyze_file(path,&a,details,sizeof(details))!=0) { log_format("[SOURCE REVIEW] Could not read: %s\r\n",path); return; }
    log_format("\r\n[SOURCE REVIEW] %s\r\nFiles: %u | review flags: %u | read errors: %u\r\n%s",
         path,a.files_scanned,a.findings,a.read_errors,details[0]?details:"No heuristic flags found.\r\n");
    log_text("Flags are review prompts, not vulnerability verdicts. Source files are never executed or modified.\r\n");
}
static void load_wav(void) {
    char path[1024]; if(!choose_file(path,sizeof(path),0)) return;
    int rc=pl_audio_load_wav(path,&g_audio);
    if(rc!=0) { log_format("[AUDIO IMPORT] WAV load failed (%d). Expected PCM 16-bit mono/stereo WAV, 8–192 kHz, bounded file size.\r\n",rc); return; }
    log_format("\r\n[DSP IMPORT] %s | %u Hz | %u channel(s) | %lu frames\r\n",path,g_audio.sample_rate,g_audio.channels,(unsigned long)g_audio.frames);
    pl_audio_stats s; if(pl_audio_measure(&g_audio,&s)==0) log_format("Peak %.4f | RMS %.4f | crest %.2f dB | clipped samples %lu\r\n",s.peak,s.rms,s.crest_db,(unsigned long)s.clipped_samples);
}
static void generate_tone(void) {
    int rc=pl_audio_generate_tone(&g_audio,48000,2,440.0,3.0);
    if(rc!=0) { log_format("[DSP GENERATOR] Tone generation failed (%d).\r\n",rc); return; }
    log_text("\r\n[DSP GENERATOR] Created 440 Hz stereo test tone, 48 kHz, 3 seconds, -3.1 dBFS peak.\r\n");
}
static void audio_effect(int id) {
    if(!g_audio.samples) { log_text("[DSP] Load a PCM WAV or generate a test tone first.\r\n"); return; }
    int rc=0; const char *name="";
    if(id==ID_LOWPASS) { double cutoff=read_parameter(ID_CUTOFF,5000.0); rc=pl_audio_apply_lowpass(&g_audio,cutoff); log_format("[DSP PROCESS] One-pole low-pass %.1f Hz: %s\r\n",cutoff,rc==0?"applied":"invalid parameter"); return; }
    else if(id==ID_COMPRESS) { double threshold=read_parameter(ID_THRESHOLD,-12.0), ratio=read_parameter(ID_RATIO,4.0); rc=pl_audio_apply_compressor(&g_audio,threshold,ratio); log_format("[DSP PROCESS] Compressor %.1f dB / %.2f:1: %s\r\n",threshold,ratio,rc==0?"applied":"invalid parameter"); return; }
    else if(id==ID_SATURATE) { double drive=read_parameter(ID_DRIVE,1.8); rc=pl_audio_apply_saturation(&g_audio,drive); log_format("[DSP PROCESS] Soft saturation %.2fx: %s\r\n",drive,rc==0?"applied":"invalid parameter"); return; }
    else if(id==ID_GAIN) { double gain=read_parameter(ID_GAIN_VALUE,-3.0); rc=pl_audio_apply_gain(&g_audio,gain); log_format("[DSP PROCESS] Gain %.1f dB: %s\r\n",gain,rc==0?"applied":"invalid parameter"); return; }
    if(rc==0) log_format("[DSP PROCESS] Applied %s.\r\n",name); else log_format("[DSP PROCESS] %s rejected the current buffer.\r\n",name);
}
static void measure_audio(void) {
    pl_audio_stats s; if(pl_audio_measure(&g_audio,&s)!=0) { log_text("[DSP METER] No valid audio buffer.\r\n"); return; }
    log_format("\r\n[DSP METER] Peak %.5f | RMS %.5f | DC %.6f | crest %.2f dB | clipped %lu\r\n",
         s.peak,s.rms,s.dc,s.crest_db,(unsigned long)s.clipped_samples);
}
static void export_wav(void) {
    if(!g_audio.samples) { log_text("[DSP EXPORT] No audio buffer to export.\r\n"); return; }
    char path[1024]; if(!choose_file(path,sizeof(path),1)) return;
    if(pl_audio_write_wav(path,&g_audio)==0) log_format("[DSP EXPORT] Wrote PCM16 WAV: %s\r\n",path);
    else log_format("[DSP EXPORT] Write failed: %s\r\n",path);
}

static void show_embedded_profiles(void) {
    size_t i;
    log_text("\r\n[EMBEDDED C] Native MCU profile catalogue\r\n");
    for (i = 0u; i < pl_embedded_profile_count(); ++i) {
        const pl_embedded_profile *p = pl_embedded_profile_at(i);
        if (!p) continue;
        log_format("%s | %s | %s | %llu Hz | FLASH %llu B | SRAM %llu B | stack %llu B%s\r\n",
            p->id, p->architecture, p->compiler,
            (unsigned long long)p->clock_hz,
            (unsigned long long)p->flash.size_bytes,
            (unsigned long long)p->sram.size_bytes,
            (unsigned long long)p->stack_reserve_bytes,
            p->is_reference_profile ? " | reference" : "");
    }
    log_text("Profile catalogue is advisory. Verify exact MCU memory, startup and pin mapping before flashing.\r\n");
}
static void layout(HWND w) {
    RECT r; GetClientRect(w,&r);
    int width=r.right-r.left, height=r.bottom-r.top;
    int margin=width<900?16:22, gap=8, button_height=44, y=98;
    int top_ids[]={ID_PROFILE,ID_SUITE,ID_REVIEW,ID_METRIC_LAB,ID_EMBEDDED,ID_THEME};
    int top_columns=width>=1480?6:3;
    int top_rows=(6+top_columns-1)/top_columns;
    int top_width=(width-2*margin-(top_columns-1)*gap)/top_columns;
    for(int i=0;i<6;i++) {
        int row=i/top_columns, column=i%top_columns;
        MoveWindow(GetDlgItem(w,top_ids[i]),margin+column*(top_width+gap),y+row*(button_height+gap),top_width,button_height,TRUE);
    }
    y+=top_rows*(button_height+gap)+8;

    int audio_ids[]={ID_LOAD_WAV,ID_TONE,ID_LOWPASS,ID_COMPRESS,ID_SATURATE,ID_GAIN,ID_MEASURE,ID_EXPORT};
    int audio_columns=width>=740?4:2;
    int audio_rows=(8+audio_columns-1)/audio_columns;
    int audio_width=(width-2*margin-(audio_columns-1)*gap)/audio_columns;
    for(int i=0;i<8;i++) {
        int row=i/audio_columns, column=i%audio_columns;
        MoveWindow(GetDlgItem(w,audio_ids[i]),margin+column*(audio_width+gap),y+row*(button_height+gap),audio_width,button_height,TRUE);
    }
    y+=audio_rows*(button_height+gap)+8;

    int labels[]={1201,1202,1203,1204,1205};
    int edits[]={ID_CUTOFF,ID_THRESHOLD,ID_RATIO,ID_DRIVE,ID_GAIN_VALUE};
    int parameter_columns=width>=1280?5:3;
    int parameter_rows=(5+parameter_columns-1)/parameter_columns;
    int group=(width-2*margin-(parameter_columns-1)*gap)/parameter_columns;
    for(int i=0;i<5;i++) {
        int row=i/parameter_columns, column=i%parameter_columns;
        int x=margin+column*(group+gap), py=y+row*60;
        MoveWindow(GetDlgItem(w,labels[i]),x,py,group-4,18,TRUE);
        MoveWindow(GetDlgItem(w,edits[i]),x,py+20,group-4,30,TRUE);
    }
    y+=parameter_rows*60+6;
    MoveWindow(GetDlgItem(w,ID_SAVE_REPORT),width-margin-244,y,124,44,TRUE);
    MoveWindow(GetDlgItem(w,ID_CLEAR),width-margin-112,y,112,44,TRUE);
    y+=52;
    int log_height=height-y-margin;
    if(log_height<80) log_height=80;
    MoveWindow(g_log,margin,y,width-2*margin,log_height,TRUE);
}
static LRESULT CALLBACK wndproc(HWND w,UINT m,WPARAM p,LPARAM l) {
    switch(m) {
    case WM_CREATE: {
        g_main=w;
        g_font=CreateFontA(-16,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH|FF_DONTCARE,"Segoe UI");
        g_heading_font=CreateFontA(-23,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH|FF_DONTCARE,"Georgia");
        rebuild_brushes();
        add_button(w,"CPU CAPABILITY SCAN",ID_PROFILE); add_button(w,"RUN PROCESSOR SUITE",ID_SUITE); add_button(w,"REVIEW SOURCE FILE",ID_REVIEW); add_button(w,"METRIC LAB",ID_METRIC_LAB); add_button(w,"EMBEDDED C",ID_EMBEDDED); add_button(w,"DARK THEME",ID_THEME);
        add_button(w,"LOAD PCM WAV",ID_LOAD_WAV); add_button(w,"GENERATE TEST TONE",ID_TONE); add_button(w,"LOW-PASS 5 kHz",ID_LOWPASS); add_button(w,"COMPRESS 4:1",ID_COMPRESS);
        add_button(w,"SOFT SATURATION",ID_SATURATE); add_button(w,"GAIN",ID_GAIN); add_button(w,"MEASURE AUDIO",ID_MEASURE); add_button(w,"EXPORT PCM WAV",ID_EXPORT);
        add_label(w,"LP cutoff Hz",1201); add_label(w,"Comp threshold dB",1202); add_label(w,"Comp ratio",1203); add_label(w,"Saturation drive",1204); add_label(w,"Gain dB",1205);
        add_edit(w,"5000",ID_CUTOFF); add_edit(w,"-12",ID_THRESHOLD); add_edit(w,"4",ID_RATIO); add_edit(w,"1.8",ID_DRIVE); add_edit(w,"-3",ID_GAIN_VALUE);
        add_button(w,"SAVE JSON REPORT",ID_SAVE_REPORT); add_button(w,"CLEAR LOG",ID_CLEAR);
        g_log=CreateWindowExA(WS_EX_CLIENTEDGE,"EDIT","Pytyliki desktop workstation initialized.\r\nCPU operations are read-only and bounded. DSP actions process a local in-memory audio buffer.\r\n",
            WS_CHILD|WS_VISIBLE|WS_VSCROLL|WS_HSCROLL|ES_MULTILINE|ES_AUTOVSCROLL|ES_READONLY|ES_NOHIDESEL,0,0,10,10,w,NULL,GetModuleHandleA(NULL),NULL);
        g_log_font=CreateFontA(-14,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,FIXED_PITCH|FF_MODERN,"Consolas");
        if (g_log_font) SendMessageA(g_log,WM_SETFONT,(WPARAM)g_log_font,TRUE); else apply_font(g_log);
        return 0;
    }
    case WM_GETMINMAXINFO: {
        MINMAXINFO *limits=(MINMAXINFO *)l;
        limits->ptMinTrackSize.x=760; limits->ptMinTrackSize.y=680;
        return 0;
    }
    case WM_SIZE: layout(w); return 0;
    case WM_CTLCOLORSTATIC: {
        HDC dc=(HDC)p; SetBkMode(dc,TRANSPARENT); SetTextColor(dc,text_color());
        return (LRESULT)g_page_brush;
    }
    case WM_CTLCOLOREDIT: {
        HDC dc=(HDC)p; HWND control=(HWND)l;
        if (control==g_log) {
            SetTextColor(dc,RGB(255,255,255));
            SetBkColor(dc,log_color()); return (LRESULT)g_log_brush;
        }
        SetTextColor(dc,text_color()); SetBkColor(dc,input_color());
        return (LRESULT)g_input_brush;
    }
    case WM_DPICHANGED: {
        RECT *suggested=(RECT*)l;
        SetWindowPos(w,NULL,suggested->left,suggested->top,suggested->right-suggested->left,
            suggested->bottom-suggested->top,SWP_NOZORDER|SWP_NOACTIVATE);
        layout(w); InvalidateRect(w,NULL,TRUE); return 0;
    }
    case WM_DRAWITEM: {
        DRAWITEMSTRUCT *item=(DRAWITEMSTRUCT*)l;
        if(item && item->CtlType==ODT_BUTTON) {
            UINT id=(UINT)item->CtlID;
            int selected=(item->itemState & ODS_SELECTED)!=0;
            int hot=(item->itemState & ODS_HOTLIGHT)!=0;
            int danger=(id==ID_CLEAR), primary=primary_button(id);
            COLORREF fill=danger?((selected||hot)?red_hover_color():red_color()):
                primary?((selected||hot)?blue_hover_color():blue_color()):
                (selected||hot?(g_dark_mode?RGB(10,30,70):RGB(244,246,250)):input_color());
            COLORREF ink=button_ink(primary,danger);
            HBRUSH brush=CreateSolidBrush(fill);
            HPEN pen=CreatePen(PS_SOLID,1,danger?red_color():primary?blue_color():border_color());
            HGDIOBJ old_brush=SelectObject(item->hDC,brush), old_pen=SelectObject(item->hDC,pen);
            RECT rc=item->rcItem;
            RoundRect(item->hDC,rc.left+1,rc.top+1,rc.right-1,rc.bottom-1,4,4);
            SetBkMode(item->hDC,TRANSPARENT); SetTextColor(item->hDC,ink);
            char label[256]={0}; GetWindowTextA(item->hwndItem,label,(int)sizeof(label));
            InflateRect(&rc,-7,-3);
            DrawTextA(item->hDC,label,-1,&rc,DT_CENTER|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);
            if(item->itemState & ODS_FOCUS) { InflateRect(&rc,-3,-3); DrawFocusRect(item->hDC,&rc); }
            SelectObject(item->hDC,old_brush); SelectObject(item->hDC,old_pen);
            DeleteObject(brush); DeleteObject(pen); return TRUE;
        }
        break;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps; HDC dc=BeginPaint(w,&ps); RECT r; GetClientRect(w,&r);
        FillRect(dc,&r,g_page_brush);
        RECT header={0,0,r.right,82}; HBRUSH hb=CreateSolidBrush(header_color()); FillRect(dc,&header,hb); DeleteObject(hb);
        SetBkMode(dc,TRANSPARENT); SetTextColor(dc,RGB(255,255,255));
        HFONT old=(HFONT)SelectObject(dc,g_heading_font ? g_heading_font : g_font); RECT title={22,10,r.right-20,42}; DrawTextA(dc,"PYTYLIKI  /  PROCESSOR SYSTEMS WORKSTATION",-1,&title,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
        SelectObject(dc,g_font); SetTextColor(dc,RGB(255,255,255)); RECT sub={23,43,r.right-20,70}; DrawTextA(dc,"WHITE / BLUE / RED    CPU diagnostics | metrics | source review | local DSP",-1,&sub,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
        SelectObject(dc,old); HBRUSH accent=CreateSolidBrush(red_color()); RECT line={0,82,r.right,86}; FillRect(dc,&line,accent); DeleteObject(accent);
        EndPaint(w,&ps); return 0;
    }
    case WM_COMMAND:
        switch(LOWORD(p)) {
            case ID_PROFILE: show_profile(); break;
            case ID_SUITE: run_suite(); break;
            case ID_REVIEW: review_source(); break;
            case ID_METRIC_LAB: show_metric_lab(); break;
            case ID_EMBEDDED: show_embedded_profiles(); break;
            case ID_LOAD_WAV: load_wav(); break;
            case ID_TONE: generate_tone(); break;
            case ID_LOWPASS: case ID_COMPRESS: case ID_SATURATE: case ID_GAIN: audio_effect(LOWORD(p)); break;
            case ID_MEASURE: measure_audio(); break;
            case ID_EXPORT: export_wav(); break;
            case ID_SAVE_REPORT: save_report(); break;
            case ID_CLEAR: SetWindowTextA(g_log,"Session log cleared.\r\n"); break;
            case ID_THEME:
                g_dark_mode = !g_dark_mode;
                rebuild_brushes();
                SetWindowTextA(GetDlgItem(w,ID_THEME),g_dark_mode?"LIGHT THEME":"DARK THEME");
                RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_ALLCHILDREN|RDW_ERASE);
                break;
        }
        return 0;
    case WM_DESTROY:
        pl_audio_free(&g_audio);
        if (g_font) DeleteObject(g_font);
        if (g_heading_font) DeleteObject(g_heading_font);
        if (g_log_font) DeleteObject(g_log_font);
        if (g_page_brush) DeleteObject(g_page_brush);
        if (g_input_brush) DeleteObject(g_input_brush);
        if (g_log_brush) DeleteObject(g_log_brush);
        PostQuitMessage(0); return 0;
    }
    return DefWindowProcA(w,m,p,l);
}
int WINAPI WinMain(HINSTANCE h,HINSTANCE prev,LPSTR cmd,int show) {
    (void)prev; (void)cmd;
    request_dpi_awareness();
    WNDCLASSA wc; memset(&wc,0,sizeof(wc)); wc.lpfnWndProc=wndproc; wc.hInstance=h;
    wc.lpszClassName="PytylikiProcessorWorkstation"; wc.hCursor=LoadCursor(NULL,IDC_ARROW); wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);
    if(!RegisterClassA(&wc)) return 1;
    HWND w=CreateWindowA(wc.lpszClassName,"Pytyliki - Processor Systems Workstation",WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN,
        CW_USEDEFAULT,CW_USEDEFAULT,1180,840,NULL,NULL,h,NULL); if(!w)return 1;
    BOOL animations_enabled=TRUE;
    char reduced_motion[8]={0};
    if(GetEnvironmentVariableA("UI_REDUCED_MOTION",reduced_motion,(DWORD)sizeof(reduced_motion))>0 &&
       (reduced_motion[0]=='1' || reduced_motion[0]=='t' || reduced_motion[0]=='T' ||
        reduced_motion[0]=='y' || reduced_motion[0]=='Y')) animations_enabled=FALSE;
#ifndef SPI_GETCLIENTAREAANIMATION
#define SPI_GETCLIENTAREAANIMATION 0x1042
#endif
    if(animations_enabled &&
       SystemParametersInfoA(SPI_GETCLIENTAREAANIMATION,0,&animations_enabled,0) &&
       animations_enabled) {
        if(!AnimateWindow(w,140,AW_BLEND|AW_ACTIVATE)) ShowWindow(w,show);
    } else {
        ShowWindow(w,show);
    }
    UpdateWindow(w); MSG msg; while(GetMessageA(&msg,NULL,0,0)>0){TranslateMessage(&msg);DispatchMessageA(&msg);} return (int)msg.wParam;
}
