// ---------------------------------------------------------------------------------------------------------------------
// Shell Image Viewer v1.0.0 [2018/07/21]
// (C) 2018 Evgeny Vrublevsky <me@veg.by>
// ---------------------------------------------------------------------------------------------------------------------

#define UNICODE
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <commdlg.h>

// https://support.microsoft.com/en-us/help/164787/info-windows-rundll-and-rundll32-interface
// extern "C" __declspec(dllimport) VOID WINAPI ImageView_FullscreenW(HWND hwnd, HINSTANCE hInst, LPCWSTR path, int nShow);
typedef void (WINAPI *SHIMGVW_PROC)(HWND, HINSTANCE, LPCWSTR, int);

int exec(wchar_t* path)
{
	HMODULE dll;
	SHIMGVW_PROC ImageView_FullscreenW;
	STARTUPINFOW si;
	int result = 0;

	if (LOBYTE(LOWORD(GetVersion())) >= 6)
	{
		AddAtomW(L"FailObsoleteShellAPIs"); // shunimpl.dll rejects to load without it (on Windows 7 x64)
	}
	dll = LoadLibrary(L"shimgvw.dll");
	if (!dll)
	{
		result = GetLastError();
	}
	else
	{
		ImageView_FullscreenW = (SHIMGVW_PROC)GetProcAddress(dll, "ImageView_FullscreenW");
		if (!ImageView_FullscreenW)
		{
			result = GetLastError();
		}
		else
		{
			GetStartupInfoW(&si);
			ImageView_FullscreenW(0, dll, path, (si.dwFlags & STARTF_USESHOWWINDOW) ? si.wShowWindow : SW_SHOWDEFAULT);
		}
		FreeLibrary(dll);
	}

	return result;
}

int wmain(int argc, wchar_t* argv[])
{
	wchar_t path[MAX_PATH];
	path[0] = 0;
	OPENFILENAMEW ofn;

	if (argc == 2)
	{
		GetFullPathName(argv[1], MAX_PATH, path, NULL);
	}
	else
	{
		memset(&ofn, 0, sizeof(OPENFILENAMEW));
		ofn.lStructSize = sizeof(OPENFILENAMEW);
		ofn.hwndOwner = 0;
		ofn.lpstrFile = path;
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrFilter = L"All\0*.*\0Images\0*.BMP;*.GIF;*.PNG;*.JPG;*.JPEG;*.TIF;*.TIFF;*.ICO;*.WMF;*.EMF\0";
		ofn.nFilterIndex = 2;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
		if (!GetOpenFileName(&ofn))
		{
			return NO_ERROR;
		}
	}

	if (!PathFileExistsW(path))
	{
		return ERROR_PATH_NOT_FOUND;
	}

	return exec(path);
}

int __stdcall WinMain(
  HINSTANCE hInstance,
  HINSTANCE hPrevInstance,
  LPSTR     lpCmdLine,
  int       nShowCmd
)
{
	int result;
	LPWSTR* argv;
	LPWSTR error_msg;
	int argc = 0;
	argv = CommandLineToArgvW(GetCommandLineW(), &argc);

	result = wmain(argc, argv);

	if (result != 0)
	{
		error_msg = NULL;
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, result, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&error_msg, 0, NULL);
		if (error_msg)
		{
			MessageBox(0, error_msg, L"Shell Image Viewer", MB_OK | MB_ICONERROR);
			LocalFree(error_msg);
		}
	}

	LocalFree(argv);
	ExitProcess(result);
	return result;
}
