// GBX Thumb Extractor
// Author: m4tx
// www.m4tx.2ap.pl
// 06.24.2010, 17:51:21am

#include <windows.h> // Okna
#include <commctrl2.h> // Toolbary, tooltipy, paski itd
#include <fstream> // Pliki
#include <string> // Ci¹gi znaków
#include <sstream>
#include <shlobj.h>

#define TTS_ICONINFORMATION 1

#define ID_FILE_EXIT     		100
#define ID_HELP_ABOUT			101

#define TTS_BALOON 0x40
#define TTM_SETTITLE WM_USER+32
#define MYTOOLTIPTEST 0x00DEAD00
#define ID_rThumbFromMap 201;
#define ID_rThumbToMap 202;

HINSTANCE MainInstance;

CONST CHAR MainForm[] = "MainForm";

MSG Message;
WPARAM hNormalFont = (WPARAM)(HFONT)GetStockObject(DEFAULT_GUI_FONT);

HWND MainFormHwnd; // G³ówna forma
HWND eInputEdit; // Pole edycyjne pliku wejœcia
HWND bInputButton; // Przycisk pola edycyjnego pliku wejœcia
HWND eOutputEdit; // Pole edycyjne pliku wyjœcia
HWND bOutputButton; // Przycisk pola edycyjnego pliku wyjœcia
HWND bStart; // Przycisk Start

HWND rThumbFromMap; // Miniatura od mapy
HWND rThumbToMap; // Miniatura do mapy

HWND lAuthorUrl; // Etykieta strony autora
HWND AboutHwnd; // Okno o programie

WNDPROC lAuthorUrlOldWndProc;
WNDPROC AboutHwndOldWndProc;

WNDCLASSEX AboutWc = {0};

WNDPROC MainHwndEditsOldWndProc;

struct MYTOOLTIP
{
    DWORD dwTest;
    LPSTR szTip;
};

LRESULT CALLBACK WndProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void CreateTooltip (HWND hParent, LPCSTR Text, LPCSTR Title);
LRESULT CALLBACK WndProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK MainHwndEditsWndProc (HWND hwnd, UINT mesg, WPARAM wParam, LPARAM lParam);
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSEX MainWndClass;

    MainWndClass.cbSize = sizeof (WNDCLASSEX); // Rozmiar struktury w bajtach
    MainWndClass.style = 0; // Style klasy
    MainWndClass.lpfnWndProc = WndProc; // WskaÅºnik do procedury obs³uguj¹cej okno
    MainWndClass.cbClsExtra = 0; // Dodatkowe bajty pamiêci dla klasy
    MainWndClass.cbWndExtra = 0; // Dodatkowe bajty pamiêci dla klasy
    MainWndClass.hInstance = hInstance; // 	 Identyfikator aplikacji, która ma byæ w³aœcicielem okna
    MainWndClass.hIcon = LoadIcon (hInstance, "APPICON"); // Ikona okna
    MainWndClass.hIconSm = LoadIcon (hInstance, "APPICON"); // Ikona okna
    MainWndClass.hCursor = LoadCursor (NULL, IDC_ARROW); // Kursor okna
    MainWndClass.hbrBackground = GetSysColorBrush (COLOR_3DFACE); // T³o okna
    MainWndClass.lpszMenuName = NULL; // Nazwa identyfikuj¹ca menu okna
    MainWndClass.lpszClassName = MainForm; // Nazwa klasy

    if (!RegisterClassEx (&MainWndClass))
    {
        MessageBox (NULL, "Nie mo¿na utworzyæ okna: b³¹d przy rejestrowaniu klasy!", "GBX Thumb Extractor", MB_OK | MB_ICONERROR);
        return 1;
    }

    MainInstance = hInstance;

    //==================================================================================================
    // Menu programu

    HMENU hMenu, hSubMenu; // utworzenie zmiennych hMenu i hSubMenu

    hMenu = CreateMenu(); // Zainicjowanie zmiennej hMenu

    hSubMenu = CreatePopupMenu(); // Zainicjowanie zmiennej hSubMenu
    AppendMenu(hSubMenu, MF_STRING, ID_FILE_EXIT, "&Zakoñcz"); // Dodanie pozycji Zakoñcz
    AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT)hSubMenu, "&Program"); // Dodanie menu SmsPrice

    hSubMenu = CreatePopupMenu(); // Zainicjowanie zmiennej hSubMenu
    AppendMenu(hSubMenu, MF_STRING, ID_HELP_ABOUT, "&O programie"); // Dodanie pozycji O programie
    AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT)hSubMenu, "P&omoc"); // Dodanie menu Pomoc

    //==================================================================================================
    // G³ówne okno

    MainFormHwnd = CreateWindowEx (
        NULL,
        MainForm,
        "GBX Thumb Extractor",
        WS_SYSMENU | WS_BORDER | WS_CAPTION | WS_VISIBLE | WS_MINIMIZEBOX,
        200,
        200,
        300,
        195,
        NULL,
        NULL,
        hInstance,
        NULL);

    SetMenu(MainFormHwnd, hMenu); // Ustawienie menu dla okna
    InitCommonControls();

    //==================================================================================================
    // Etykieta dla pola tekstowego pliku wejœcia

    HWND lInputLabel = CreateWindowEx (
        NULL,
        "STATIC",
        "Wybierz plik, który przetworzyæ:",
        WS_CHILD | WS_VISIBLE,
        5,
        5,
        285,
        20,
        MainFormHwnd,
        NULL,
        hInstance,
        NULL);
    SendMessage(lInputLabel, WM_SETFONT, hNormalFont, 0);

    //==================================================================================================
    // Pole tekstowe pliku wejœcia

    eInputEdit = CreateWindowEx (
        WS_EX_CLIENTEDGE,
        "EDIT",
        NULL,
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        5,
        25,
        245,
        20,
        MainFormHwnd,
        NULL,
        hInstance,
        NULL);
    SendMessage(eInputEdit, WM_SETFONT, hNormalFont, 0);
    CreateTooltip (eInputEdit,
    "Wpisz tutaj pe³n¹ œcie¿kê pliku, z którego ma zostaæ wyci¹gniêta miniatura, lub skorzystaj z przycisku po prawej stronie.",
    "Informacja");
    MainHwndEditsOldWndProc = (WNDPROC) SetWindowLong (eInputEdit, GWL_WNDPROC, (LONG)MainHwndEditsWndProc);
    SetWindowLong (eInputEdit, GWL_WNDPROC, (LONG)MainHwndEditsWndProc);

    //==================================================================================================
    // Przycisk dla pola tekstowego pliku wejœcia

    bInputButton = CreateWindowEx (
        NULL,
        "BUTTON",
        "...",
        WS_CHILD | WS_VISIBLE,
        255,
        25,
        30,
        20,
        MainFormHwnd,
        NULL,
        hInstance,
        NULL);
    SendMessage(bInputButton, WM_SETFONT, hNormalFont, 0);
    CreateTooltip (bInputButton, "Wciœnij ten przycisk, aby wybraæ plik, z którego ma zostaæ wyci¹gniêta miniatura.", "Informacja");

    //==================================================================================================
    // Etykieta dla pola tekstowego pliku wejœcia

    HWND lOutputLabel = CreateWindowEx (
        NULL,
        "STATIC",
        "Wybierz plik, do którego zapisaæ wynik:",
        WS_CHILD | WS_VISIBLE,
        5,
        55,
        285,
        20,
        MainFormHwnd,
        NULL,
        hInstance,
        NULL);
    SendMessage(lOutputLabel, WM_SETFONT, hNormalFont, 0);

    //==================================================================================================
    // Pole tekstowe pliku wyjœcia

    eOutputEdit = CreateWindowEx (
        WS_EX_CLIENTEDGE,
        "EDIT",
        NULL,
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        5,
        75,
        245,
        20,
        MainFormHwnd,
        NULL,
        hInstance,
        NULL);
    SendMessage(eOutputEdit, WM_SETFONT, hNormalFont, 0);
    CreateTooltip (eOutputEdit,
    "Wpisz tutaj pe³n¹ œcie¿kê pliku, do którego ma zostaæ zapisany wynik, lub skorzystaj z przycisku po prawej stronie. Plik nie musi istnieæ.",
    "Informacja");
    SetWindowLong (eOutputEdit, GWL_WNDPROC, (LONG)MainHwndEditsWndProc);

    //==================================================================================================
    // Przycisk dla pola tekstowego pliku wyjœcia

    bOutputButton = CreateWindowEx (
        NULL,
        "BUTTON",
        "...",
        WS_CHILD | WS_VISIBLE,
        255,
        75,
        30,
        20,
        MainFormHwnd,
        NULL,
        hInstance,
        NULL);
    SendMessage(bOutputButton, WM_SETFONT, hNormalFont, 0);
    CreateTooltip (bOutputButton, "Wciœnij ten przycisk, aby wybraæ plik, do którego ma zostaæ zapisany wynik.", "Informacja");

    //==================================================================================================
    // Przycisk "Start"

    bStart = CreateWindowEx (
        NULL,
        "BUTTON",
        "Start",
        WS_CHILD | WS_VISIBLE,
        5,
        110,
        60,
        30,
        MainFormHwnd,
        NULL,
        hInstance,
        NULL);
    SendMessage(bStart, WM_SETFONT, hNormalFont, 0);
    CreateTooltip (bStart, "Wciœnij ten przycisk, aby program rozpocz¹³ wykonywanie zadania.", "Start");

    //==================================================================================================
    // Miniatura od mapy

    rThumbFromMap = CreateWindowEx (
        NULL,
        "BUTTON",
        "Miniatura od mapy",
        WS_CHILD | WS_VISIBLE | BS_RADIOBUTTON,
        145,
        105,
        140,
        20,
        MainFormHwnd,
        (HMENU) 201,
        hInstance,
        NULL);
    SendMessage(rThumbFromMap, WM_SETFONT, hNormalFont, 0);
    CreateTooltip (rThumbFromMap, "Zaznacz to pole, aby miniatura by³a zapisana z pliku mapy na dysk.", "Miniatura od mapy");
    CheckDlgButton(MainFormHwnd, 201, BST_CHECKED);

    //==================================================================================================
    // Miniatura do mapy

    rThumbToMap = CreateWindowEx (
        NULL,
        "BUTTON",
        "Miniatura do mapy",
        WS_CHILD | WS_VISIBLE | BS_RADIOBUTTON,
        145,
        125,
        140,
        20,
        MainFormHwnd,
        (HMENU) 202,
        hInstance,
        NULL);
    SendMessage(rThumbToMap, WM_SETFONT, hNormalFont, 0);
    CreateTooltip (rThumbToMap, "Zaznacz to pole, aby miniatura by³a zapisana z dysku do pliku mapy.", "Miniatura do mapy");

    //==================================================================================================

    if (MainFormHwnd == NULL)
    {
        MessageBox (NULL, "Nie mo¿na utworzyæ okna: b³¹d przy tworzeniu!", "GBX Thumb Extractor", MB_OK | MB_ICONERROR);
        return 1;
    }

    ShowWindow (MainFormHwnd, nCmdShow);
    UpdateWindow (MainFormHwnd);

    //==================================================================================================
    // Pêtla komunikatów

    while (GetMessage (&Message, NULL, 0, 0))
    {
        TranslateMessage (&Message);
        DispatchMessage (&Message);
    }

    return Message.wParam;

    //==================================================================================================

    return 0;
}

void OpenFileDialog ();
void SaveFileDialog ();
void ProcessFile();

PSTR GetControlText(HWND hWindow);
void CreateDialogBox(HWND);
void RegisterDialogClass(HWND hwnd);
LRESULT CALLBACK WndProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
      case WM_CREATE:
          RegisterDialogClass(hwnd);
          break;
        case WM_CLOSE:
            DestroyWindow (hwnd);
            break;

        case WM_DESTROY:
            PostQuitMessage (0);
            break;

        case WM_COMMAND:
            if ((HWND) lParam == bInputButton)
                OpenFileDialog();

            if ((HWND) lParam == bOutputButton)
                SaveFileDialog();

            if ((HWND) lParam == bStart)
                ProcessFile();

            if ((HWND) lParam == rThumbFromMap)
            {
                if (!IsDlgButtonChecked(hwnd, 201))
                {
                    CheckDlgButton(hwnd, 201, BST_CHECKED);
                    CheckDlgButton(hwnd, 202, BST_UNCHECKED);
                }
            }
            if ((HWND) lParam == rThumbToMap)
            {
                if (!IsDlgButtonChecked(hwnd, 202))
                {
                    CheckDlgButton(hwnd, 202, BST_CHECKED);
                    CheckDlgButton(hwnd, 201, BST_UNCHECKED);
                }
            }

            switch(LOWORD(wParam)) {
                case ID_FILE_EXIT:
                    PostQuitMessage(0);
                break;

                case ID_HELP_ABOUT:
                    CreateDialogBox(hwnd);
                break;
            }
            break;

        case WM_NOTIFY:
            if (((LPNMHDR)lParam)->code == TTN_GETDISPINFO)
            {
                LPNMTTDISPINFO lpnmtdi = (LPNMTTDISPINFO) lParam;

                if (lpnmtdi->lParam != (LPARAM)NULL)
                {
                    DWORD dwValue;
                    memcpy(&dwValue, (const void*)lpnmtdi->lParam, 4);

                    if (dwValue == MYTOOLTIPTEST)
                    {
                        MYTOOLTIP* mtl = (MYTOOLTIP*)(lpnmtdi->lParam);

                        lpnmtdi->lpszText = mtl->szTip;
                        lpnmtdi->hinst = NULL;
                        lpnmtdi->uFlags = TTF_DI_SETITEM;

                        SendMessage (((LPNMHDR)lParam)->hwndFrom, TTM_SETMAXTIPWIDTH, 0, 300);

                        delete mtl;
                    }
                }
            }
            break;

        case WM_KEYDOWN:
            switch ((int) wParam)
            {
                case VK_ESCAPE:
                    PostQuitMessage(0);
                    break;
            }
            break;

        default:
            return DefWindowProc (hwnd, msg, wParam, lParam);
    }

    return 0;
}

LRESULT CALLBACK MainHwndEditsWndProc (HWND hwnd, UINT mesg, WPARAM wParam, LPARAM lParam)
{
    switch (mesg)
    {
        case WM_KEYDOWN:
            switch ((int) wParam)
            {
                case VK_ESCAPE:
                    PostQuitMessage(0);
                    break;
            }
            break;
    }

    return CallWindowProc (MainHwndEditsOldWndProc, hwnd, mesg, wParam, lParam);
}

void OpenFileDialog ()
{
     OPENFILENAME ofn;
     char FileName[MAX_PATH] = "";

     ZeroMemory (&ofn, sizeof(ofn));
     ofn.lStructSize = sizeof(ofn);
     ofn.lpstrFilter = "Pliki map gry TrackMania (*.Challenge.Gbx)\0*.Challenge.Gbx\0Wszystkie pliki (*.*)\0*.*\0";
     ofn.nMaxFile = MAX_PATH;
     ofn.lpstrFile = FileName;
     ofn.lpstrDefExt = "Challenge.Gbx";
     ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

    EnableWindow(MainFormHwnd, FALSE);
    if (GetOpenFileName(&ofn))
    {
        EnableWindow(MainFormHwnd, TRUE);
        SetWindowText(eInputEdit, FileName);
    }
    else
    {
        EnableWindow(MainFormHwnd, TRUE);
    }
}

void SaveFileDialog ()
{
     OPENFILENAME ofn;
     char FileName[MAX_PATH] = "";

     ZeroMemory (&ofn, sizeof(ofn));
     ofn.lStructSize = sizeof(ofn);
     ofn.lpstrFilter = "Plik JPEG (*.jpg;*.jpeg)\0*.jpg;*.jpeg\0Wszystkie pliki (*.*)\0*.*\0";
     ofn.nMaxFile = MAX_PATH;
     ofn.lpstrFile = FileName;
     ofn.lpstrDefExt = "Challenge.Gbx";
     ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

    EnableWindow(MainFormHwnd, FALSE);
    if (GetSaveFileName(&ofn))
    {
        SetWindowText(eOutputEdit, FileName);
        EnableWindow(MainFormHwnd, TRUE);
    }
    else
    {
        EnableWindow(MainFormHwnd, TRUE);
    }
}

PSTR GetControlText(HWND hWindow)
{
    int cTxtLen = GetWindowTextLength(hWindow);
    PSTR pszMem = (PSTR) VirtualAlloc((LPVOID) NULL,
        (DWORD) (cTxtLen + 1), MEM_COMMIT,
        PAGE_READWRITE);

    GetWindowText(hWindow, pszMem, cTxtLen + 1);

    return (pszMem);
}

std::string ExtractDirectory( std::string appName)
{
        int i ;
        for (i = appName.size() - 1 ; i >= 0 ; i--)
                if (appName[i] == '\\') break ;
        return appName.substr(0 , i + 1) ;
}

std::string ExtractDirectory(const char *appName)
{
        return ExtractDirectory(std::string(appName)) ;
}

void ProcessFile()
{
                    std::fstream thumb111;
                    thumb111.open("\%Temp\%\\lol.jpg", std::ios::out | std::ios::binary);
    bool error = false;

    if (error != true)
    {
        std::string path;

        path = GetControlText(eInputEdit);

        std::fstream file;
        file.open(path.c_str(), std::ios::in | std::ios::binary | std::ios::ate);

        if(file.good() == true)
        {
            char sc;
            std::stringstream buffer;
            std::string source, destination;

            file.seekg (0, std::ios::beg);
            while (file.good()) {
                sc = file.get();
                buffer << sc;
            }
            source = buffer.str();
            unsigned long int pos1 = source.find("<Thumbnail.jpg>");
            unsigned long int pos2 = source.find("</Thumbnail.jpg>");

            if (pos1 != std::string::npos || pos2 != std::string::npos)
            {
                if (IsDlgButtonChecked(MainFormHwnd, 201))
                {
                    pos1 = pos1 + 15;
                    destination = source.substr(pos1, pos2-pos1);

                    std::fstream file2;
                    file2.open((CHAR*)GetControlText(eOutputEdit), std::ios::out | std::ios::binary);

                    if (file2.good() == true)
                    {
                        file2.write(destination.c_str(), pos2-1);
                        file2.close();

                        TCHAR fName[MAX_PATH];
                        GetModuleFileName(NULL , fName , MAX_PATH);

                        ShellExecute(NULL, NULL, std::string(ExtractDirectory(fName) + "jpegtran.exe").c_str(),
                              std::string("-flip vertical \"" + std::string(GetControlText(eOutputEdit)) + "\" \"" + std::string(GetControlText(eOutputEdit)) + "\"").c_str(), NULL, SW_HIDE);
                    }
                    else
                    {
                        MessageBox (NULL, "Nie uda³o siê otworzyæ pliku wyjœciowego!\n\nPrawdopodobne przyczyny:\n- Nie posiadasz uprawnieñ pozwalaj¹cych modyfikowaæ plik\n- Nie posiadasz uprawnieñ do katalogu, w którym chcesz utworzyæ plik\n- Noœnik, na którym chcesz dokonaæ zapisu jest tylko do odczytu",
                            "GBX Thumb Extractor", MB_OK | MB_ICONERROR);
                    }
                }
                else
                {
                    destination = source;
                    destination.erase (pos1, pos2);

                    std::fstream thumb;
                    thumb.open("\%Temp\%\\lol.jpg", std::ios::out | std::ios::binary);
                }
            }
            else
            {
                MessageBox (NULL, "Ten plik nie jest prawid³owym plikiem mapy gry TrackMania!", "GBX Thumb Extractor", MB_OK | MB_ICONERROR);
            }
        }
        else
        {
            MessageBox (NULL, "Nie uda³o siê otworzyæ pliku wejœciowego!\n\nPrawdopodobne przyczyny:\n- Plik nie istnieje na dysku\n- Nie posiadasz uprawnieñ potrzebnych do otwarcia pliku",
                "GBX Thumb Extractor", MB_OK | MB_ICONERROR);
        }
    }
}

void CreateTooltip (HWND hParent, LPCSTR Text, LPCSTR Title)
{
    // Utowrzenie Tooltip'a
    HWND hTooltip = CreateWindowEx (
        WS_EX_TOPMOST,
        TOOLTIPS_CLASS,
        NULL,
        WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP | TTS_BALOON,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        hParent,
        NULL,
        MainInstance,
        NULL);

    SetWindowPos (hTooltip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE); // Zawsze na wierzchu

    TOOLINFO ti;
    ti.cbSize = sizeof (TOOLINFO);
    ti.uFlags = TTF_SUBCLASS | TTF_IDISHWND;
    ti.hwnd = MainFormHwnd;
    ti.hinst = NULL;
    ti.uId = (UINT_PTR) hParent;
    ti.lpszText = LPSTR_TEXTCALLBACK;

    MYTOOLTIP* mtl = new MYTOOLTIP;
    mtl->dwTest = MYTOOLTIPTEST;
    mtl->szTip = (LPSTR)Text;
    ti.lParam = (LPARAM) mtl;

    // Obszar aktywnoœci Tooltip'a
    RECT rect;
    GetClientRect (hParent, &rect);

    ti.rect.left = rect.left;
    ti.rect.top = rect.top;
    ti.rect.right = rect.right;
    ti.rect.bottom = rect.bottom;

    SendMessage (hTooltip, TTM_ADDTOOL, 0, (LPARAM)&ti);

    SendMessage (hTooltip, TTM_SETTITLE, 1, (LPARAM)Title);
}

LRESULT CALLBACK DialogProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch(msg)
  {
    case WM_KEYDOWN:
        switch ((int) wParam)
        {
            case VK_ESCAPE:
                EnableWindow (MainFormHwnd, TRUE);
                DestroyWindow (AboutHwnd);
                break;
        }
        break;

    case WM_COMMAND:
        if ( (HWND) lParam == lAuthorUrl )
        {
            ShellExecute(NULL, "open", "http://www.m4tx.2ap.pl/",
                NULL, NULL, SW_SHOWNORMAL);
        }
        break;

    case WM_CLOSE:
        EnableWindow(MainFormHwnd, TRUE);
        DestroyWindow(hwnd);
        break;
  }
  return (DefWindowProc(hwnd, msg, wParam, lParam));

}

LRESULT CALLBACK lAuthorUrlProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_MOUSEMOVE:
            HCURSOR curs = AboutWc.hCursor = LoadCursor (0, IDC_HAND);
            SetCursor (curs);
            break;
    }

    return CallWindowProc (lAuthorUrlOldWndProc, hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK AboutHwndProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_MOUSEMOVE:
            HCURSOR curs = AboutWc.hCursor = LoadCursor (NULL, IDC_ARROW);
            SetCursor (curs);
            break;
    }

    return CallWindowProc (AboutHwndOldWndProc, hwnd, msg, wParam, lParam);
}

void RegisterDialogClass(HWND hwnd)
{
    AboutWc.cbSize           = sizeof(WNDCLASSEX);
    AboutWc.lpfnWndProc      = (WNDPROC) DialogProc;
    AboutWc.hInstance        = MainInstance;
    AboutWc.hbrBackground    = GetSysColorBrush(COLOR_3DFACE);
    AboutWc.lpszClassName    = TEXT("AboutClass");
    RegisterClassEx(&AboutWc);
}

void CreateDialogBox(HWND hwnd)
{
    AboutHwnd = CreateWindowEx(
        WS_EX_TOOLWINDOW,
        TEXT("AboutClass"),
        TEXT("O programie..."),
        WS_VISIBLE | WS_SYSMENU | WS_CAPTION,
        100,
        100,
        180,
        162,
        MainFormHwnd,
        NULL,
        MainInstance,
        NULL);

    AboutHwndOldWndProc = (WNDPROC) SetWindowLong (AboutHwnd, GWL_WNDPROC, (LONG) AboutHwndProc);

    EnableWindow(MainFormHwnd, FALSE);

    HWND sIcon = CreateWindowEx (NULL, "STATIC", NULL, WS_CHILD | WS_VISIBLE | SS_ICON, 5, 5, 48, 48, AboutHwnd, 0, MainInstance, NULL);
    HICON hIcon = (HICON) LoadImage(MainInstance, "APPICON", IMAGE_ICON, 48, 48, NULL);
    SendMessage(sIcon, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);

    //==================================================================================================
    // Nazwa programu

        HWND lName = CreateWindowEx(
            0,
            "STATIC",
            "GBX Thumb",
            WS_CHILD | WS_VISIBLE | SS_LEFT,
            60,
            5,
            120,
            15,
            AboutHwnd,
            NULL,
            MainInstance,
            NULL);
        SendMessage(lName, WM_SETFONT, (WPARAM)hNormalFont, 0);

    //==================================================================================================
    // Nazwa programu - linia 2

        HWND lName2 = CreateWindowEx(
            0,
            "STATIC",
            "Extractor",
            WS_CHILD | WS_VISIBLE | SS_LEFT,
            60,
            20,
            120,
            15,
            AboutHwnd,
            NULL,
            MainInstance,
            NULL);
        SendMessage(lName2, WM_SETFONT, (WPARAM)hNormalFont, 0);

    //==================================================================================================
    // Wersja

        HWND lVersion = CreateWindowEx(
            0,
            "STATIC",
            "wersja 1.0",
            WS_CHILD | WS_VISIBLE | SS_LEFT,
            60,
            40,
            60,
            15,
            AboutHwnd,
            NULL,
            MainInstance,
            NULL);
        SendMessage(lVersion, WM_SETFONT, (WPARAM)hNormalFont, 0);

    //==================================================================================================
    // Autor

        HWND lAuthor = CreateWindowEx(
            0,
            "STATIC",
            "Autor: m4tx",
            WS_CHILD | WS_VISIBLE | SS_LEFT,
            5,
            60,
            60,
            15,
            AboutHwnd,
            NULL,
            MainInstance,
            NULL);
        SendMessage(lAuthor, WM_SETFONT, (WPARAM)hNormalFont, 0);

    //==================================================================================================
    // Link do strony autora

    // Czcionka
        HFONT lAuthorUrlFont = CreateFont(
            14,
            0,
            0,
            0,
            FW_DONTCARE,
            FALSE,
            TRUE,
            FALSE,
            DEFAULT_CHARSET,
            OUT_CHARACTER_PRECIS,
            CLIP_CHARACTER_PRECIS,
            DEFAULT_QUALITY,
            FF_DONTCARE,
            NULL);

    // Kontrolka
        lAuthorUrl = CreateWindowEx(
            0,
            "STATIC",
            "www.m4tx.2ap.pl",
            WS_CHILD | WS_VISIBLE | SS_LEFT | SS_NOTIFY,
            5,
            75,
            90,
            15,
            AboutHwnd,
            NULL,
            MainInstance,
            NULL);
        SendMessage(lAuthorUrl, WM_SETFONT, (WPARAM)lAuthorUrlFont, 0);
        lAuthorUrlOldWndProc = (WNDPROC) SetWindowLong (lAuthorUrl, GWL_WNDPROC, (LONG)lAuthorUrlProc);

    //==================================================================================================
    // Podziêkowania dla

        HWND lGreets = CreateWindowEx(
            0,
            "STATIC",
            "Podziêkowania dla:",
            WS_CHILD | WS_VISIBLE | SS_LEFT,
            5,
            105,
            95,
            15,
            AboutHwnd,
            NULL,
            MainInstance,
            NULL);
        SendMessage(lGreets, WM_SETFONT, (WPARAM)hNormalFont, 0);

    //==================================================================================================
    // Grupy IJG

        HWND lGreetsRhino = CreateWindowEx(
            0,
            "STATIC",
            "Grupy IJG",
            WS_CHILD | WS_VISIBLE | SS_LEFT,
            5,
            120,
            90,
            15,
            AboutHwnd,
            NULL,
            MainInstance,
            NULL);
        SendMessage(lGreetsRhino, WM_SETFONT, (WPARAM)hNormalFont, 0);

    //==================================================================================================

}
