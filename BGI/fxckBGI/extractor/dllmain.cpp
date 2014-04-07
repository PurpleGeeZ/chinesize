#include <Windows.h>
#include <Shellapi.h>
#include <shlobj.h>
#include <objbase.h>      // For COM headers
#include <shobjidl.h>     // for IFileDialogEvents and IFileDialogControlEvents
#include "..\shell\fxckBGI.h"
#include "extractor.h"
#include "resource.h"
#include "misc.h"
#include <ilhook.h>
#include <vector>
#include <string>

//#pragma comment(linker,"/entry:MyDllMain")

using namespace std;
int InjectToProcess(HANDLE process,HANDLE thread,wchar_t* dllPath,DecoprFunc func);

DecoprFunc g_DecompressFile;
HANDLE g_hProcessHeap;
HWND g_hwndMain;

HANDLE g_workingThread;
HANDLE g_paused;
HANDLE g_needStop;
wstring g_extPath;

BYTE g_stolenBytesSWT[8];
BYTE g_stolenBytesMB[8];
DWORD g_setWindowTextAddr;
DWORD g_messageBoxAddr;
BOOL g_ready=FALSE;

Options g_options;

DWORD WINAPI waitReady(LPVOID param)
{
	while(TRUE)
	{
		if(g_ready)
		{
            if(g_DecompressFile)
			    SetWindowText(g_hwndMain,L"fxckBGI - ready");
            else
            {
                MessageBox(g_hwndMain,L"Can't find the function address! Please restart fxckBGI and specify it manually.",
                    L"fxckBGI",MB_ICONASTERISK);
                SetWindowText(g_hwndMain,L"fxckBGI - failed");
                //EndDialog(g_hwndMain,0);
            }
			break;
		}
		Sleep(100);
	}
	return 0;
}

BOOL CheckReady(HWND hwndDlg)
{
    if(!g_ready)
    {
        if(g_DecompressFile==0)
        {
            MessageBox(hwndDlg,L"Please wait for the engine getting ready!",L"fxckBGI",
                MB_ICONASTERISK);
            return FALSE;
        }
        int rslt=MessageBox(hwndDlg,L"It seems not ready for the engine, force to start?",L"fxckBGI",
            MB_YESNO|MB_ICONASTERISK);
        if(rslt==IDNO)
            return FALSE;
    }
    else if(!g_DecompressFile)
    {
        return FALSE;
    }
    return TRUE;
}

BOOL CALLBACK MainWndProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	vector<wstring>* arcList;
	wchar_t* path;
	IShellItemArray* items;
	IShellItem* item;
	DWORD itemCount;
	switch(message)
	{
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_PAUSE:
			if(WaitForSingleObject(g_paused,0)==0)
			{
				ResetEvent(g_paused);
				SetWindowText((HWND)lParam,L"Continue");
			}
			else
			{
				SetEvent(g_paused);
				SetWindowText((HWND)lParam,L"Pause");
			}
			break;
		case IDC_STOP:
			SetEvent(g_paused);
			SetEvent(g_needStop);
			Log(L"操作被终止");
			break;
		case IDC_SELECTARC:
            if(!CheckReady(hwndDlg))
                break;
			if(BasicFileOpen(&items,FALSE,TRUE)==E_NOTIMPL)
			{
				MessageBox(hwndDlg,L"Please drag arc file(s) to this window!",L"Not supported",MB_ICONINFORMATION);
				break;
			}
			if(items)
			{
				items->GetCount(&itemCount);
				arcList=new vector<wstring>();
				for(DWORD i=0;i<itemCount;i++)
				{
					HRESULT hr=items->GetItemAt(i,&item);
					if(SUCCEEDED(hr))
					{
						hr=item->GetDisplayName(SIGDN_FILESYSPATH,&path);
						if(SUCCEEDED(hr))
						{
							arcList->push_back(path);
							CoTaskMemFree(path);
						}
						item->Release();
					}
				}
				items->Release();

				g_workingThread=CreateThread(0,0,ExtractThread,arcList,0,0);
				if(g_workingThread==0)
				{
					delete arcList;
					MessageBox(hwndDlg,L"Can't start working thread",0,0);
				}
			}
			break;
		case IDC_BROWSE:

			if(BasicFileOpen(&path,TRUE)==E_NOTIMPL)
			{
				MessageBox(hwndDlg,L"Please type or paste the path by yourself!",L"Not supported",MB_ICONINFORMATION);
				break;
			}
			if(path!=NULL)
			{
				SetDlgItemText(g_hwndMain,IDC_PATHTOSAVE,path);
				CoTaskMemFree(path);
			}
			break;
		case IDC_ENCODE:
			g_options.isEncode=IsDlgButtonChecked(hwndDlg,IDC_ENCODE);
			EnableWindow(GetDlgItem(hwndDlg,IDC_PICFORMAT),
				g_options.isEncode?TRUE:FALSE);
			EnableWindow(GetDlgItem(hwndDlg,IDC_ALPHABLEND),
				g_options.isEncode?FALSE:TRUE);
			break;
		case IDC_EXPORTLOG:
			BasicFileSave(&path);
			if(path!=NULL)
			{
				HANDLE logFile=CreateFile(path,GENERIC_WRITE,FILE_SHARE_READ,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);
				CoTaskMemFree(path);
				if(logFile==INVALID_HANDLE_VALUE)
				{
					MessageBox(hwndDlg,L"无法创建文件！",0,0);
					break;
				}

				DWORD temp;

				WriteFile(logFile,"\xff\xfe",2,&temp,0);

				int curLen=100;
				wchar_t* str=new wchar_t[curLen];
				HWND listBox=GetDlgItem(hwndDlg,IDC_LOGLIST);
				int itemCount=SendMessage(listBox,LB_GETCOUNT,0,0);
				
				for(int i=0;i<itemCount;i++)
				{
					int len=SendMessage(listBox,LB_GETTEXTLEN,i,0);
					if(len!=LB_ERR)
					{
						if(curLen<len+3)
						{
							curLen=len+3;
							delete[] str;
							str=new wchar_t[curLen];
						}
						len=SendMessage(listBox,LB_GETTEXT,i,(LPARAM)str);
						wcscat(str,L"\r\n");

						if(len!=LB_ERR)
						{
							WriteFile(logFile,str,(len+2)*2,&temp,0);
						}
					}
				}
				CloseHandle(logFile);
				delete[] str;
			}
			break;
		case IDCANCEL:
			EndDialog(hwndDlg,0);

		}
		break;
	case WM_DROPFILES:
		{
            if(!CheckReady(hwndDlg))
                break;
			HDROP drop=(HDROP)wParam;
			int fileCount=DragQueryFile(drop,-1,0,0);
			arcList=new vector<wstring>();
			for(int i=0;i<fileCount;i++)
			{
				wchar_t* fname;
				int namelen=DragQueryFile(drop,i,0,0);
				if(namelen!=0)
				{
					fname=new wchar_t[namelen+1];
					if(DragQueryFile(drop,i,fname,namelen+1))
					{
						arcList->push_back(fname);
						//Log(fname);
					}
					delete[] fname;
				}
			}
			if(arcList->size()==0)
			{
				delete arcList;
				MessageBox(hwndDlg,L"no files input",0,0);
			}
			else
			{
				g_workingThread=CreateThread(0,0,ExtractThread,arcList,0,0);
				if(g_workingThread==0)
				{
					delete arcList;
					MessageBox(hwndDlg,L"Can't start working thread",0,0);
				}
			}
		}
		break;
	case WM_INITDIALOG:
		g_hwndMain=hwndDlg;
		g_needStop=CreateEvent(0,FALSE,FALSE,0);
		g_paused=CreateEvent(0,TRUE,TRUE,0);
		//CreateThread(0,0,waitReady,0,0,0);

        SetWindowText(hwndDlg,L"fxckBGI v" PRODUCT_VERSION);
		SendDlgItemMessage(hwndDlg,IDC_PICFORMAT,CB_ADDSTRING,0,(LPARAM)L"PNG");
// 		SendDlgItemMessage(hwndDlg,IDC_PICFORMAT,CB_ADDSTRING,0,(LPARAM)L"JPEG");
		SendDlgItemMessage(hwndDlg,IDC_PICFORMAT,CB_SETCURSEL,0,0);
		CheckDlgButton(hwndDlg,IDC_ENCODE,BST_CHECKED);
		CheckDlgButton(hwndDlg,IDC_ALPHABLEND,BST_CHECKED);
		EnableWindow(GetDlgItem(hwndDlg,IDC_ALPHABLEND),FALSE);

		return TRUE;
	}
	return FALSE;
}
DWORD WINAPI MainWnd(LPVOID param)
{
	//wchar_t pipeName[30];
	//wsprintf(pipeName,PIPE_NAME,GetCurrentProcessId());
	CoInitialize(0);

#ifndef DBG
//	HANDLE pipe=CreateFile(pipeName,GENERIC_READ|GENERIC_WRITE,0,0,OPEN_EXISTING,0,0);
//	if(pipe==INVALID_HANDLE_VALUE)
//	{
//		MessageBox(0,pipeName,0,0);
//		return 0;
//	}
//	DWORD dwMode = PIPE_READMODE_BYTE; 
//
//	BOOL fSuccess = SetNamedPipeHandleState(pipe,&dwMode,NULL,NULL);
//	if ( ! fSuccess) 
//	{
//		CloseHandle(pipe);
//		MessageBox(0,L"Can't set pipe state",0,0);
//		return 0;
//	}
//
//	//MyDataStruct* mds;
//	DWORD readBytes;
//	ReadFile(pipe,(DWORD*)&g_DecompressFile,4,&readBytes,0);
//	if(readBytes!=4)
//	{
//		CloseHandle(pipe);
//		MessageBox(0,L"Receive error",0,0);
//		return 0;
//	}
//
//	//g_DecompressFile=mds->funcAddr;
//
//	DWORD message='OVER';
//	WriteFile(pipe,&message,4,&readBytes,0);
//	CloseHandle(pipe);
//#else
//    g_DecompressFile=(DecoprFunc)0x44fd20;
#endif

	HMODULE thisMod=GetModuleHandle(L"extractor.dll");
	DialogBoxParam(thisMod,(LPCWSTR)IDD_MAIN,0,MainWndProc,0);
	CoUninitialize();
	FreeLibraryAndExitThread(thisMod,0);
	return 0;
}

DWORD WINAPI StartWindow()
{
    g_DecompressFile=(DecoprFunc)SearchDecompressFunc();
    if(!g_DecompressFile)
        MessageBox(0,L"This may not be a BGI exe.",0,0);
    else
    {
        CreateThread(0,0,(LPTHREAD_START_ROUTINE)MainWnd,0,0,0);
    }
    return 0;
}

__declspec(naked) int SetWindowOnce()
{
	__asm
	{
        cmp g_ready,0
        jnz _lbl
        pushad
//        call SearchDecompressFunc
//        mov g_DecompressFile,eax
        call StartWindow
        popad

        mov g_ready,1
_lbl:
		lea ecx,g_stolenBytesSWT
		mov eax,[ecx]
		mov edx,g_setWindowTextAddr
		mov [edx],eax
		mov al,[ecx+4]
		mov [edx+4],al
		jmp g_setWindowTextAddr
	}
}

__declspec(naked) int MessageBoxOnce()
{
    __asm
    {
        cmp g_ready,0
        jnz _lbl
        pushad
        call StartWindow
        popad

        mov g_ready,1
_lbl:
        lea ecx,g_stolenBytesMB
        mov eax,[ecx]
        mov edx,g_messageBoxAddr
        mov [edx],eax
        mov al,[ecx+4]
        mov [edx+4],al
        jmp g_messageBoxAddr
    }
}
typedef BOOL (WINAPI *CreateProcessInternalWRoutine)(
    HANDLE token,
    _In_opt_     LPCTSTR lpApplicationName,
    _Inout_opt_  LPTSTR lpCommandLine,
    _In_opt_     LPSECURITY_ATTRIBUTES lpProcessAttributes,
    _In_opt_     LPSECURITY_ATTRIBUTES lpThreadAttributes,
    _In_         BOOL bInheritHandles,
    _In_         DWORD dwCreationFlags,
    _In_opt_     LPVOID lpEnvironment,
    _In_opt_     LPCTSTR lpCurrentDirectory,
    _In_         LPSTARTUPINFO lpStartupInfo,
    _Out_        LPPROCESS_INFORMATION lpProcessInformation,
    PHANDLE pnewtoken
    );

BOOL WINAPI MyCreateProcessInternalW(
    CreateProcessInternalWRoutine func,
    HANDLE token,
    _In_opt_     LPCTSTR lpApplicationName,
    _Inout_opt_  LPTSTR lpCommandLine,
    _In_opt_     LPSECURITY_ATTRIBUTES lpProcessAttributes,
    _In_opt_     LPSECURITY_ATTRIBUTES lpThreadAttributes,
    _In_         BOOL bInheritHandles,
    _In_         DWORD dwCreationFlags,
    _In_opt_     LPVOID lpEnvironment,
    _In_opt_     LPCTSTR lpCurrentDirectory,
    _In_         LPSTARTUPINFO lpStartupInfo,
    _Out_        LPPROCESS_INFORMATION lpProcessInformation,
    PHANDLE pnewtoken
   )
{
    int hasSus= (dwCreationFlags & CREATE_SUSPENDED);
    dwCreationFlags |= CREATE_SUSPENDED;
    BOOL ret=func(0,lpApplicationName,lpCommandLine,lpProcessAttributes,lpThreadAttributes,bInheritHandles,
        dwCreationFlags,lpEnvironment,lpCurrentDirectory,lpStartupInfo,lpProcessInformation,0);
    if(ret)
    {
        HMODULE hm=GetModuleHandle(L"extractor.dll");
        if(hm)
        {
            int pathLen=256;
            wchar_t* dllPath=new wchar_t[pathLen];
            int retlen=GetModuleFileName(hm,dllPath,pathLen);
            while(GetLastError()==ERROR_INSUFFICIENT_BUFFER)
            {
                delete[] dllPath;
                pathLen*=2;
                dllPath=new wchar_t[pathLen];
                retlen=GetModuleFileName(0,dllPath,pathLen);
            };
            InjectToProcess(lpProcessInformation->hProcess,lpProcessInformation->hThread,dllPath,0);
            delete[] dllPath;
        }
        if(!hasSus)
            ResumeThread(lpProcessInformation->hThread);
    }
    return ret;
}

char CheckFileName[100];
void WINAPI MyGetFileAttributesA(LPSTR name)
{
    int len0=strlen(CheckFileName);
    int len1=strlen(name);
    if(len1<len0 || len0==0)
        return;
    if(!_stricmp(name+(len1-len0),CheckFileName))
        name[len1-len0]+=1;
}

void GetZeroBytesFile()
{
    HMODULE hm=GetModuleHandle(0);
    int pathLen=256;
    wchar_t* dllPath=new wchar_t[pathLen];
    int retlen=GetModuleFileName(hm,dllPath,pathLen);
    while(GetLastError()==ERROR_INSUFFICIENT_BUFFER)
    {
        delete[] dllPath;
        pathLen*=2;
        dllPath=new wchar_t[pathLen];
        retlen=GetModuleFileName(0,dllPath,pathLen);
    };
    wchar_t* p=dllPath+retlen;
    for(;p>dllPath;p--)
        if(*p==L'\\')
            break;
    *(p+1)=L'\0';
    wstring path0(dllPath);
    *(p+1)=L'*';
    *(p+2)=L'\0';

    WIN32_FIND_DATA wfd;
    auto hf=FindFirstFile(dllPath,&wfd);
    if(hf!=INVALID_HANDLE_VALUE)
    {
        do 
        {
            auto hfile=CreateFile((path0+wfd.cFileName).c_str(),GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
            if(hfile!=INVALID_HANDLE_VALUE)
            {
                DWORD fileSize=GetFileSize(hfile,0);
                CloseHandle(hfile);
                if(fileSize<=16)
                {
                    WideCharToMultiByte(CP_ACP,0,wfd.cFileName,-1,CheckFileName,100,0,0);
                    break;
                }
            }
            
        } while (FindNextFile(hf,&wfd));
        FindClose(hf);
    }
}

int WINAPI DllMain(_In_  HINSTANCE hinstDLL,
  _In_  DWORD fdwReason,
  _In_  LPVOID lpvReserved
  )
{
	if(fdwReason==DLL_PROCESS_ATTACH)
	{
		g_hProcessHeap=GetProcessHeap();
		//g_extPath=new wstring(L"");

		HMODULE hm=LoadLibrary(L"user32.dll");
		BYTE* stAddr=(BYTE*)GetProcAddress(hm,"SetWindowTextA");
		g_setWindowTextAddr=(DWORD)stAddr;
		DWORD oldProtect;
		if(!VirtualProtect(stAddr,5,PAGE_EXECUTE_READWRITE,&oldProtect))
		{
			MessageBox(0,L"hook failed",0,0);
			return FALSE;
		}
		*(DWORD*)g_stolenBytesSWT=*(DWORD*)stAddr;
		g_stolenBytesSWT[4]=stAddr[4];
		*stAddr=0xe9;
		*(DWORD*)(stAddr+1)=(DWORD)SetWindowOnce-(DWORD)stAddr-5;

        stAddr=(BYTE*)GetProcAddress(hm,"MessageBoxA");
        g_messageBoxAddr=(DWORD)stAddr;
        if(!VirtualProtect(stAddr,5,PAGE_EXECUTE_READWRITE,&oldProtect))
        {
            MessageBox(0,L"hook2 failed",0,0);
            return FALSE;
        }
        *(DWORD*)g_stolenBytesMB=*(DWORD*)stAddr;
        g_stolenBytesMB[4]=stAddr[4];
        *stAddr=0xe9;
        *(DWORD*)(stAddr+1)=(DWORD)MessageBoxOnce-(DWORD)stAddr-5;

        BYTE* buff=(BYTE*)VirtualAlloc(0,1000,MEM_COMMIT,PAGE_EXECUTE_READWRITE);
        HookSrcObject src;
        HookStubObject stub;
        hm=LoadLibrary(L"kernel32.dll");
        auto func=GetProcAddress(hm,"CreateProcessInternalW");
        if(!InitializeHookSrcObject(&src,func) ||
            !InitializeStubObject(&stub,buff,100,48,STUB_DIRECTLYRETURN|STUB_OVERRIDEEAX) ||
            !Hook32(&src,0,&stub,MyCreateProcessInternalW,"f123456789ABC"))
        {
            MessageBox(0,L"Can't hook CreateProcessInternalW",0,0);
            return FALSE;
        }

        func=GetProcAddress(hm,"GetFileAttributesA");
        if(!InitializeHookSrcObject(&src,func) ||
            !InitializeStubObject(&stub,buff+100,100) ||
            !Hook32(&src,0,&stub,MyGetFileAttributesA,"1"))
        {
            MessageBox(0,L"Can't hook GetFileAttributesA",0,0);
            return FALSE;
        }

        GetZeroBytesFile();

//		CreateThread(0,0,MainWnd,0,0,0);
	}
	return TRUE;
}