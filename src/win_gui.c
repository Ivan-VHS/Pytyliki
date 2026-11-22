#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winreg.h>
#include <commdlg.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "core.h"
#include "processor.h"
#include "metric_geometry.h"
#include "metric_registry.h"
#include "embedded.h"
#include "ui_ux_policy.h"

#ifndef WM_DPICHANGED
#define WM_DPICHANGED 0x02E0
#endif

#define ID_PROFILE 1001
#define ID_SUITE 1002
#define ID_REVIEW 1003
#define ID_CLEAR 1012
#define ID_SAVE_REPORT 1013
#define ID_METRIC_LAB 1014
#define ID_EMBEDDED 1015
#define ID_THEME 1016
#define ID_MOTION 1017
#define ID_MOTION_REDUCE 1301
#define ID_MOTION_ECO 1302
#define ID_MOTION_SAVE 1304
#define ID_MOTION_CANCEL 1305
#define ID_MOTION_MODE 1306

static HWND g_log;
static HWND g_main;
static HFONT g_font;
static HFONT g_log_font;
static HBRUSH g_page_brush, g_input_brush, g_log_brush;
static int g_dark_mode = 0;
static int g_reduce_motion = 0;
static int g_eco_visuals = 0;
static int g_motion_mode = 0; /* 0 Normal, 1 Expressive, 2 Reduced Motion */
static HWND g_motion_dialog = NULL;
static HWND g_motion_mode_combo = NULL;
static HWND g_motion_eco_check = NULL;

static COLORREF page_color(void) { return g_dark_mode ? RGB(7,26,45) : RGB(244,248,253); }
static COLORREF header_color(void) { return g_dark_mode ? RGB(5,14,26) : RGB(0,51,102); }
static COLORREF contrast_safe_text(COLORREF proposed, COLORREF background) {
    if (pl_ui_text_contrast_meets(
        GetRValue(proposed), GetGValue(proposed), GetBValue(proposed),
        GetRValue(background), GetGValue(background), GetBValue(background), 0))
        return proposed;
    const double black_ratio = pl_ui_contrast_ratio_rgb(
        0,0,0,GetRValue(background),GetGValue(background),GetBValue(background));
    const double white_ratio = pl_ui_contrast_ratio_rgb(
        255,255,255,GetRValue(background),GetGValue(background),GetBValue(background));
    return black_ratio >= white_ratio ? RGB(0,0,0) : RGB(255,255,255);
}
static COLORREF text_color(void) {
    return contrast_safe_text(
        g_dark_mode ? RGB(232,237,242) : RGB(18,44,69), page_color()
    );
}
static COLORREF input_color(void) { return g_dark_mode ? RGB(15,42,69) : RGB(255,255,255); }
static COLORREF log_color(void) { return g_dark_mode ? RGB(7,26,45) : RGB(15,42,69); }
static COLORREF blue_color(void) { return g_dark_mode ? RGB(112,165,255) : RGB(0,78,152); }
static COLORREF blue_hover_color(void) { return g_dark_mode ? RGB(138,182,255) : RGB(26,95,219); }
static COLORREF red_color(void) { return g_dark_mode ? RGB(255,123,134) : RGB(181,18,45); }
static COLORREF red_hover_color(void) { return g_dark_mode ? RGB(255,154,162) : RGB(156,16,38); }
static COLORREF border_color(void) { return g_dark_mode ? RGB(49,81,111) : RGB(202,216,232); }
static COLORREF button_ink(int primary, int danger) {
    if (primary) return g_dark_mode ? RGB(6,26,48) : RGB(255,255,255);
    if (danger) return g_dark_mode ? RGB(43,6,16) : RGB(255,255,255);
    return blue_color();
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

static void log_format(const char *fmt, ...);
static void apply_font(HWND control);

static void load_motion_settings(void) {
    HKEY key;
    DWORD value = 0, size = sizeof(value), type = REG_DWORD;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Ivan-VHS\\Pytyliki\\UI", 0, KEY_QUERY_VALUE, &key) != ERROR_SUCCESS)
        return;
    size = sizeof(value);
    if (RegQueryValueExA(key, "ReduceMotion", NULL, &type, (LPBYTE)&value, &size) == ERROR_SUCCESS && type == REG_DWORD)
        g_reduce_motion = value != 0;
    value = 0; size = sizeof(value); type = REG_DWORD;
    if (RegQueryValueExA(key, "EcoVisuals", NULL, &type, (LPBYTE)&value, &size) == ERROR_SUCCESS && type == REG_DWORD)
        g_eco_visuals = value != 0;
    value = 0; size = sizeof(value); type = REG_DWORD;
    if (RegQueryValueExA(key, "MotionMode", NULL, &type, (LPBYTE)&value, &size) == ERROR_SUCCESS && type == REG_DWORD)
        g_motion_mode = value > 2 ? 0 : (int)value;
    if (g_reduce_motion) g_motion_mode = 2;
    RegCloseKey(key);
}

static void save_motion_settings(void) {
    HKEY key;
    DWORD disposition = 0;
    if (RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\Ivan-VHS\\Pytyliki\\UI", 0, NULL, 0,
        KEY_SET_VALUE, NULL, &key, &disposition) != ERROR_SUCCESS)
        return;
    g_reduce_motion = (g_motion_mode == 2);
    DWORD reduce = g_reduce_motion ? 1u : 0u;
    DWORD eco = g_eco_visuals ? 1u : 0u;
    DWORD mode = (DWORD)g_motion_mode;
    RegSetValueExA(key, "ReduceMotion", 0, REG_DWORD, (const BYTE*)&reduce, sizeof(reduce));
    RegSetValueExA(key, "EcoVisuals", 0, REG_DWORD, (const BYTE*)&eco, sizeof(eco));
    RegSetValueExA(key, "MotionMode", 0, REG_DWORD, (const BYTE*)&mode, sizeof(mode));
    RegCloseKey(key);
}

static void close_motion_dialog(HWND dialog) {
    if (IsWindow(dialog)) DestroyWindow(dialog);
}

static LRESULT CALLBACK motion_dialog_proc(HWND dialog, UINT message, WPARAM wparam, LPARAM lparam) {
    (void)lparam;
    switch (message) {
    case WM_CREATE: {
        HWND mode_label = CreateWindowExA(0, "STATIC", "Motion intensity",
            WS_CHILD | WS_VISIBLE | SS_LEFT, 22, 18, 430, 22,
            dialog, NULL, GetModuleHandleA(NULL), NULL);
        g_motion_mode_combo = CreateWindowExA(0, "COMBOBOX", "",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | WS_VSCROLL,
            22, 42, 430, 120, dialog, (HMENU)(INT_PTR)ID_MOTION_MODE, GetModuleHandleA(NULL), NULL);
        SendMessageA(g_motion_mode_combo, CB_ADDSTRING, 0, (LPARAM)"Normal");
        SendMessageA(g_motion_mode_combo, CB_ADDSTRING, 0, (LPARAM)"Expressive");
        SendMessageA(g_motion_mode_combo, CB_ADDSTRING, 0, (LPARAM)"Reduced Motion");
        SendMessageA(g_motion_mode_combo, CB_SETCURSEL, (WPARAM)g_motion_mode, 0);
        g_motion_eco_check = CreateWindowExA(0, "BUTTON", "Eco / Low-CPU Visuals - suppress nonessential effects",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX, 22, 82, 430, 44,
            dialog, (HMENU)(INT_PTR)ID_MOTION_ECO, GetModuleHandleA(NULL), NULL);
        HWND note = CreateWindowExA(0, "STATIC",
            "Normal: balanced motion. Expressive: longer window entrance. Reduced Motion: no spatial transition.",
            WS_CHILD | WS_VISIBLE | SS_LEFT, 22, 132, 430, 50, dialog, NULL, GetModuleHandleA(NULL), NULL);
        HWND save = CreateWindowExA(0, "BUTTON", "Save", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
            270, 198, 82, (int)pl_ui_preferred_target_px(), dialog, (HMENU)(INT_PTR)ID_MOTION_SAVE, GetModuleHandleA(NULL), NULL);
        HWND cancel = CreateWindowExA(0, "BUTTON", "Cancel", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
            362, 198, 82, (int)pl_ui_preferred_target_px(), dialog, (HMENU)(INT_PTR)ID_MOTION_CANCEL, GetModuleHandleA(NULL), NULL);
        apply_font(mode_label); apply_font(g_motion_mode_combo); apply_font(g_motion_eco_check);
        apply_font(note); apply_font(save); apply_font(cancel);
        SendMessageA(g_motion_eco_check, BM_SETCHECK, g_eco_visuals ? BST_CHECKED : BST_UNCHECKED, 0);
        SetFocus(g_motion_mode_combo);
        return 0;
    }
    case WM_COMMAND:
        if (LOWORD(wparam) == ID_MOTION_SAVE) {
            LRESULT selected = SendMessageA(g_motion_mode_combo, CB_GETCURSEL, 0, 0);
            g_motion_mode = selected >= 0 && selected <= 2 ? (int)selected : 0;
            g_reduce_motion = (g_motion_mode == 2);
            g_eco_visuals = SendMessageA(g_motion_eco_check, BM_GETCHECK, 0, 0) == BST_CHECKED;
            save_motion_settings();
            log_format("[UI MOTION] Mode %s | Eco Visuals %s\\r\\n",
                g_motion_mode == 2 ? "REDUCED" : g_motion_mode == 1 ? "EXPRESSIVE" : "NORMAL",
                g_eco_visuals ? "ON" : "OFF");
            close_motion_dialog(dialog);
            return 0;
        }
        if (LOWORD(wparam) == ID_MOTION_CANCEL) {
            close_motion_dialog(dialog);
            return 0;
        }
        break;
    case WM_KEYDOWN:
        if (wparam == VK_ESCAPE) {
            close_motion_dialog(dialog);
            return 0;
        }
        break;
    case WM_CLOSE:
        close_motion_dialog(dialog);
        return 0;
    case WM_NCDESTROY:
        g_motion_dialog = NULL;
        g_motion_mode_combo = NULL;
        g_motion_eco_check = NULL;
        EnableWindow(g_main, TRUE);
        SetForegroundWindow(g_main);
        return 0;
    }
    return DefWindowProcA(dialog, message, wparam, lparam);
}

static void show_motion_settings(void) {
    if (g_motion_dialog && IsWindow(g_motion_dialog)) {
        SetForegroundWindow(g_motion_dialog);
        return;
    }
    EnableWindow(g_main, FALSE);
    g_motion_dialog = CreateWindowExA(WS_EX_DLGMODALFRAME | WS_EX_CONTROLPARENT, "PytylikiMotionSettings",
        "UI Motion Settings", WS_CAPTION | WS_SYSMENU | WS_POPUP,
        CW_USEDEFAULT, CW_USEDEFAULT, 490, 320, g_main, NULL, GetModuleHandleA(NULL), NULL);
    if (!g_motion_dialog) {
        EnableWindow(g_main, TRUE);
        SetForegroundWindow(g_main);
        return;
    }
    ShowWindow(g_motion_dialog, SW_SHOW);
    UpdateWindow(g_motion_dialog);
}


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
static int choose_source_file(char *path, size_t cap) {
    OPENFILENAMEA ofn;
    memset(&ofn, 0, sizeof(ofn));
    path[0] = '\0';
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = GetActiveWindow();
    ofn.lpstrFile = path;
    ofn.nMaxFile = (DWORD)cap;
    ofn.lpstrFilter = "C/C++/Rust source (*.c;*.h;*.cpp;*.rs)\0*.c;*.h;*.cpp;*.rs\0All files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR | OFN_FILEMUSTEXIST | OFN_EXPLORER;
    return GetOpenFileNameA(&ofn);
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
    char path[1024]; if(!choose_source_file(path,sizeof(path))) return;
    pl_analysis a={0}; char details[8192]={0};
    if(pl_analyze_file(path,&a,details,sizeof(details))!=0) { log_format("[SOURCE REVIEW] Could not read: %s\r\n",path); return; }
    log_format("\r\n[SOURCE REVIEW] %s\r\nFiles: %u | review flags: %u | read errors: %u\r\n%s",
         path,a.files_scanned,a.findings,a.read_errors,details[0]?details:"No heuristic flags found.\r\n");
    log_text("Flags are review prompts, not vulnerability verdicts. Source files are never executed or modified.\r\n");
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
    int top_ids[]={ID_PROFILE,ID_SUITE,ID_REVIEW,ID_METRIC_LAB,ID_EMBEDDED,ID_THEME,ID_MOTION};
    int top_columns=width>=1480?7:4;
    int top_rows=(7+top_columns-1)/top_columns;
    int top_width=(width-2*margin-(top_columns-1)*gap)/top_columns;
    for(int i=0;i<7;i++) {
        int row=i/top_columns, column=i%top_columns;
        MoveWindow(GetDlgItem(w,top_ids[i]),margin+column*(top_width+gap),y+row*(button_height+gap),top_width,button_height,TRUE);
    }
    y+=top_rows*(button_height+gap)+8;

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
        rebuild_brushes();
        add_button(w,"CPU CAPABILITY SCAN",ID_PROFILE); add_button(w,"RUN PROCESSOR SUITE",ID_SUITE); add_button(w,"REVIEW SOURCE FILE",ID_REVIEW); add_button(w,"METRIC LAB",ID_METRIC_LAB); add_button(w,"EMBEDDED C",ID_EMBEDDED); add_button(w,"DARK THEME",ID_THEME); add_button(w,"MOTION SETTINGS",ID_MOTION);
        add_button(w,"SAVE JSON REPORT",ID_SAVE_REPORT); add_button(w,"CLEAR LOG",ID_CLEAR);
        g_log=CreateWindowExA(WS_EX_CLIENTEDGE,"EDIT","Pytyliki desktop workstation initialized.\r\nDeveloper: Ivan-VHS\r\nCPU operations are read-only and bounded. Measurements and workload plans are local and bounded.\r\n",
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
            SetTextColor(dc,RGB(229,237,247));
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
                (selected||hot?(g_dark_mode?RGB(36,79,130):RGB(217,232,255)):input_color());
            COLORREF ink=button_ink(primary,danger);
            HBRUSH brush=CreateSolidBrush(fill);
            HPEN pen=CreatePen(PS_SOLID,1,danger?red_color():primary?blue_color():border_color());
            HGDIOBJ old_brush=SelectObject(item->hDC,brush), old_pen=SelectObject(item->hDC,pen);
            RECT rc=item->rcItem;
            RoundRect(item->hDC,rc.left+1,rc.top+1,rc.right-1,rc.bottom-1,16,16);
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
        HFONT old=(HFONT)SelectObject(dc,g_font); RECT title={22,10,r.right-20,42}; DrawTextA(dc,"PYTYLIKI  /  PROCESSOR SYSTEMS WORKSTATION  /  IVAN-VHS",-1,&title,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
        SetTextColor(dc,RGB(218,231,245)); RECT sub={23,43,r.right-20,70}; DrawTextA(dc,"WHITE / BLUE / RED    CPU diagnostics | metrics | source review | embedded tools",-1,&sub,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
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
            case ID_MOTION: show_motion_settings(); break;
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
        if (g_font) DeleteObject(g_font);
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
    WNDCLASSA motion_wc; memset(&motion_wc,0,sizeof(motion_wc));
    motion_wc.lpfnWndProc=motion_dialog_proc; motion_wc.hInstance=h;
    motion_wc.lpszClassName="PytylikiMotionSettings"; motion_wc.hCursor=LoadCursor(NULL,IDC_ARROW);
    motion_wc.hbrBackground=(HBRUSH)(COLOR_BTNFACE+1);
    if(!RegisterClassA(&motion_wc)) return 1;
    HWND w=CreateWindowA(wc.lpszClassName,"Pytyliki - Processor Systems Workstation",WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN,
        CW_USEDEFAULT,CW_USEDEFAULT,1180,840,NULL,NULL,h,NULL); if(!w)return 1;
    load_motion_settings();
    BOOL animations_enabled=(g_motion_mode == 2 || g_eco_visuals) ? FALSE : TRUE;
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
        const UINT intro_ms = g_motion_mode == 1 ? 240u : 140u;
        /* Expressive maps the paddle response to one native glide; no ambient loop. */
        const DWORD intro_flags = g_motion_mode == 1
            ? (AW_SLIDE | AW_VER_NEGATIVE | AW_ACTIVATE)
            : (AW_BLEND | AW_ACTIVATE);
        if(!AnimateWindow(w,intro_ms,intro_flags)) ShowWindow(w,show);
    } else {
        ShowWindow(w,show);
    }
    UpdateWindow(w); MSG msg; while(GetMessageA(&msg,NULL,0,0)>0){TranslateMessage(&msg);DispatchMessageA(&msg);} return (int)msg.wParam;
}
