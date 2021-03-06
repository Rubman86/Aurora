// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "CgSDK.x64_2015.h"
#include <stdio.h>
#include <string>
#include <iomanip>
#include <fstream>
#include <sstream>
BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
#define PIPE_NAME L"\\\\.\\pipe\\Aurora\\server" 

static bool isInitialized = false;
HANDLE hPipe;
static std::string program_name;

bool WriteToPipe(const std::string command_cargo)
{
	if (!isInitialized)
		return false;

	//Create JSON
	std::string contents = "";

	contents += '{';
	contents += "\"provider\": {\"name\": \"" + program_name + "\", \"appid\": 0},";
	contents += command_cargo;
	contents += '}';
	contents += "\r\n";

	if (INVALID_HANDLE_VALUE == hPipe)
	{
		//Try to restore handle
		//Connect to the server pipe using CreateFile()
		hPipe = CreateFile(
			PIPE_NAME,   // pipe name 
			GENERIC_WRITE,  // write access 
			0,              // no sharing 
			NULL,           // default security attributes
			OPEN_EXISTING,  // opens existing pipe 
			0,              // default attributes 
			NULL);          // no template file 

		if (INVALID_HANDLE_VALUE == hPipe)
		{
			return false;
		}
	}

	DWORD cbBytes;

	const char* c_contents = contents.c_str();
	int c_cotents_len = strlen(c_contents);

	BOOL bResult = WriteFile(
		hPipe,                // handle to pipe 
		c_contents,             // buffer to write from 
		c_cotents_len,   // number of bytes to write, include the NULL
		&cbBytes,             // number of bytes written 
		NULL);                // not overlapped I/O 

	if ((!bResult) || c_cotents_len != cbBytes)
	{
		CloseHandle(hPipe);
		return false;
	}
	else
	{
		return true;
	}

	return false;
}

void write_text_to_log_file(const std::string &text)
{

	//std::ofstream out("output.txt", std::ios_base::app);
	//out << text;
	//out.close();

}

CORSAIR_GAME_SDK bool CgSdkRequestControl(CorsairAccessMode accessMode)
{
	write_text_to_log_file("\nrelease");
	return true;
}

// checks file and protocol version of CUE to understand which of SDK functions can be used with this version of CUE
CORSAIR_GAME_SDK CorsairProtocolDetails CgSdkPerformProtocolHandshake()
{
	if (!isInitialized)
	{
		//Get Application name
		CHAR pBuf[MAX_PATH];
		int bytes = GetModuleFileNameA(NULL, pBuf, MAX_PATH);
		std::string filepath = pBuf;

		int fn_beginning = 0;
		for (int chr_pos = strlen(pBuf) - 1; chr_pos > -1; chr_pos--)
		{
			if (pBuf[chr_pos] == '\\')
			{
				fn_beginning = chr_pos + 1;
				break;
			}
		}

		program_name = filepath.substr(fn_beginning);

		//Connect to the server pipe using CreateFile()
		hPipe = CreateFile(
			PIPE_NAME,   // pipe name 
			GENERIC_WRITE,  // write access 
			0,              // no sharing 
			NULL,           // default security attributes
			OPEN_EXISTING,  // opens existing pipe 
			0,              // default attributes 
			NULL);          // no template file 

		if (hPipe == NULL || hPipe == INVALID_HANDLE_VALUE)
		{
			DWORD last_error = GetLastError();

			switch (last_error)
			{
			case ERROR_PIPE_BUSY:
				write_text_to_log_file("Pipe error, ERROR_PIPE_BUSY");
				break;
			case ERROR_PIPE_CONNECTED:
				write_text_to_log_file("Pipe error, ERROR_PIPE_CONNECTED");
				break;
			case ERROR_PIPE_LISTENING:
				write_text_to_log_file("Pipe error, ERROR_PIPE_LISTENING");
				break;
			case ERROR_PIPE_LOCAL:
				write_text_to_log_file("Pipe error, ERROR_PIPE_LOCAL");
				break;
			case ERROR_PIPE_NOT_CONNECTED:
				write_text_to_log_file("Pipe error, ERROR_PIPE_NOT_CONNECTED");
				break;
			default:
				write_text_to_log_file("Non-pipe related error");
				break;
			}

			isInitialized = false;

		}
	}
	isInitialized = true;
	CorsairProtocolDetails details;
	details.sdkVersion = "3.0.165";
	details.sdkProtocolVersion = 1;
	details.serverProtocolVersion = 1;
	details.serverVersion = "3.7.99";
	details.breakingChanges = false;
	write_text_to_log_file("\nhanshake");
	return details;
}

// returns last error that occured while using any of Corsair* functions
CORSAIR_GAME_SDK CorsairError CgSdkGetLastError()
{
	isInitialized = false;
	write_text_to_log_file("error");
	return CE_InvalidArguments;
}

//releases previously requested control for specified access mode
CORSAIR_GAME_SDK bool CgSdkReleaseControl(CorsairAccessMode accessMode)
{
	if (isInitialized)
	{
		std::string contents = "";
		std::stringstream ss;
		ss << "\"command\": " << "\"" << "CorsRelease" << "\"" << ',';
		ss << "\"command_data\": {";
		ss << '}';
		contents = ss.str();
		WriteToPipe(contents);
	}
	write_text_to_log_file("Call, UnInit()");
	if (isInitialized && (hPipe != NULL && hPipe != INVALID_HANDLE_VALUE))
		CloseHandle(hPipe);

	isInitialized = false;

	return true;
}


CORSAIR_GAME_SDK bool CgSdkSetGame(char *gamename)
{
	if (isInitialized)
	{
		std::string contents = "";
		std::stringstream ss;
		ss << "\"command\": " << "\"" << "CorsGame" << "\"" << ',';
		ss << "\"command_data\": {";
		ss << "\"effect_config\": " << "\"" << gamename << "\"";
		ss << '}';
		contents = ss.str();
		WriteToPipe(contents);
	}
	write_text_to_log_file("\nSet Game: ");
	write_text_to_log_file(gamename);
	return true;
}

CORSAIR_GAME_SDK bool CgSdkSetState(char *gamename)
{
	if (isInitialized)
	{
		std::string contents = "";
		std::stringstream ss;
		ss << "\"command\": " << "\"" << "CorsState" << "\"" << ',';
		ss << "\"command_data\": {";
		ss << "\"effect_config\": " << "\"" << gamename << "\"";
		ss << '}';
		contents = ss.str();
		WriteToPipe(contents);
	}
	write_text_to_log_file("\nSet State: ");
	write_text_to_log_file(gamename);
	return true;
}

CORSAIR_GAME_SDK bool CgSdkSetEvent(char *gamename)
{

	if (isInitialized)
	{
		std::string contents = "";
		std::stringstream ss;
		ss << "\"command\": " << "\"" << "CorsEvent" << "\"" << ',';
		ss << "\"command_data\": {";
		ss << "\"effect_config\": " << "\"" << gamename << "\"";
		ss << '}';
		contents = ss.str();
		WriteToPipe(contents);
	}
	write_text_to_log_file("\nsetevent"); write_text_to_log_file(gamename);
	return true;
}

CORSAIR_GAME_SDK bool CgSdkSetStateWithKey(char *gamename)
{
	if (isInitialized)
	{
		std::string contents = "";
		std::stringstream ss;
		ss << "\"command\": " << "\"" << "CorsStateKey" << gamename << "\"" << ',';
		ss << "\"command_data\": {";
		ss << '}';
		contents = ss.str();
		WriteToPipe(contents);
	}
	write_text_to_log_file("\nstatekey");
	return true;
}



CORSAIR_GAME_SDK bool CgSdkSetEventWithKey(char *gamename)
{
	if (isInitialized)
	{
		std::string contents = "";
		std::stringstream ss;
		ss << "\"command\": " << "\"" << "CorsEventKey" << gamename << "\"" << ',';
		ss << "\"command_data\": {";
		ss << '}';
		contents = ss.str();
		WriteToPipe(contents);
	}
	write_text_to_log_file("\neventkey");
	return true;
}

CORSAIR_GAME_SDK bool CgSdkSetProgressBarValue()
{
	if (isInitialized)
	{
		std::string contents = "";
		std::stringstream ss;
		ss << "\"command\": " << "\"" << "CorsSetProgress" << "\"" << ',';
		ss << "\"command_data\": {";
		ss << '}';
		contents = ss.str();
		WriteToPipe(contents);
	}
	write_text_to_log_file("\nsetProgress");
	return true;
}

CORSAIR_GAME_SDK bool CgSdkShowProgressBar()
{
	if (isInitialized)
	{
		std::string contents = "";
		std::stringstream ss;
		ss << "\"command\": " << "\"" << "CorsShowProgress" << "\"" << ',';
		ss << "\"command_data\": {";
		ss << '}';
		contents = ss.str();
		WriteToPipe(contents);
	}
	write_text_to_log_file("\nshowProgress");
	return true;
}

CORSAIR_GAME_SDK bool CgSdkHideProgressBar()
{
	if (isInitialized)
	{
		std::string contents = "";
		std::stringstream ss;
		ss << "\"command\": " << "\"" << "CorsHideProgress" << "\"" << ',';
		ss << "\"command_data\": {";
		ss << '}';
		contents = ss.str();
		WriteToPipe(contents);
	}
	write_text_to_log_file("\ntest");
	return true;
}

CORSAIR_GAME_SDK bool CgSdkClearState(char *gamename)
{
	if (isInitialized)
	{
		std::string contents = "";
		std::stringstream ss;
		ss << "\"command\": " << "\"" << "CorsClearState" << "\"" << ',';
		ss << "\"command_data\": {";
		ss << "\"effect_config\": " << "\"" << gamename << "\"";
		ss << '}';
		contents = ss.str();
		WriteToPipe(contents);
	}
	write_text_to_log_file("\nClear State ");
	return true;
}

CORSAIR_GAME_SDK bool CgSdkClearStateWithKey(char *gamename)
{
	if (isInitialized)
	{
		std::string contents = "";
		std::stringstream ss;
		ss << "\"command\": " << "\"" << "CorsClearKey" << gamename << "\"" << ',';
		ss << "\"command_data\": {";
		ss << '}';
		contents = ss.str();
		WriteToPipe(contents);
	}
	write_text_to_log_file("\nClear State Key ");
	write_text_to_log_file(gamename);
	return true;
}

CORSAIR_GAME_SDK bool CgSdkClearAllStates()
{
	if (isInitialized)
	{
		std::string contents = "";
		std::stringstream ss;
		ss << "\"command\": " << "\"" << "CorsClearAllStates" << "\"" << ',';
		ss << "\"command_data\": {";
		ss << '}';
		contents = ss.str();
		WriteToPipe(contents);
	}
	write_text_to_log_file("\nclearall");
	return true;
}

CORSAIR_GAME_SDK bool CgSdkClearAllEvents()
{
	if (isInitialized)
	{
		std::string contents = "";
		std::stringstream ss;
		ss << "\"command\": " << "\"" << "CorsClearAllEvents" << "\"" << ',';
		ss << "\"command_data\": {";
		ss << '}';
		contents = ss.str();
		WriteToPipe(contents);
	}
	write_text_to_log_file("\nclearallevents");
	return true;
}

