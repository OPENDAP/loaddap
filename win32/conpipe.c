/*  Win32 items to replace popen() and pclose() on unix.  This   */
/*  is for receiving the stdout of a console process.            */
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <process.h>
#include <conpipe.h>

HANDLE subprocess;

HANDLE hPipeOutputRead;
HANDLE hPipeOutputWrite;
HANDLE hPipeInputRead;
HANDLE hPipeInputWrite;

FILE *openpipe(char *command, unsigned short redirect_stderr)
{
	SECURITY_ATTRIBUTES		sa		= {0};
	STARTUPINFO				si		= {0};
	PROCESS_INFORMATION		pi		= {0};
	BOOL					bTest= 0;
	DWORD					dwNumberOfBytesRead = 0;

	FILE					*stream;
	int						CrtFileHandle;

	hPipeOutputRead = hPipeOutputWrite = hPipeInputRead = hPipeInputWrite = NULL;

	sa.nLength = sizeof(sa);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;

	if(!CreatePipe(&hPipeOutputRead,&hPipeOutputWrite,&sa,0))
		{
		mexPrintf("Can't open stdout pipe\n");
		return(NULL);
		}

	if(!CreatePipe(&hPipeInputRead,&hPipeInputWrite,&sa,0))
		{
		mexPrintf("Can't open stdin pipe\n");
		return(NULL);
		}

	si.cb = sizeof(si);
	si.dwFlags		= STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.wShowWindow	= SW_HIDE;
	si.hStdInput	= hPipeInputRead;
	si.hStdOutput	= hPipeOutputWrite;
	if(redirect_stderr)
		si.hStdError	= hPipeOutputWrite;

	if(CreateProcess(NULL, command, NULL, NULL, TRUE, HIGH_PRIORITY_CLASS, NULL, NULL, &si, &pi) == 0)
		{
		mexPrintf("Could not execute DODS subprocess\n");
		return(NULL);
		}

	CloseHandle(hPipeOutputWrite);
	CloseHandle(hPipeInputRead);

	/*  Convert OS file handle to CRT file pointer  */
	if((CrtFileHandle = _open_osfhandle((long)hPipeOutputRead, _O_RDONLY|_O_BINARY)) == -1)
		{
		mexPrintf("Could not translate windows handle to crt handle\n");
		return(NULL);
		}

	/*  Change handle access to stream access.  */
	if((stream = _fdopen(CrtFileHandle, "r+b")) == NULL)
		{
		mexPrintf("Could not open FILE * for pipe\n");
		return(NULL);
		}

	subprocess = pi.hProcess;

	return(stream);

}

/*  Shutdown all handles to console application and  */
/*  kill it if it didn't terminate on its' own.      */
int pclose(FILE *stream)
{
	HANDLE phandle = (HANDLE)(_get_osfhandle(stream));

	/*  To make sure we don't zip through this function prematurely     */
	/*  (before process startup) we might have to wait for a bit.       */
	WaitForSingleObject(subprocess,INFINITE);

	TerminateProcess(subprocess, 1);

	CloseHandle(subprocess);
	CloseHandle(hPipeOutputRead);
	CloseHandle(hPipeInputWrite);

	fclose(stream);
	CloseHandle(phandle);

	//  Success, always
	return(0);
}
