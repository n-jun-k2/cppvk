#pragma once

#include <Windows.h>
#include <vector>

///�A�v���P�[�V�����ƃE�B���h�E�̑���
class WindowPtr
{
public:
	~WindowPtr() {}
	virtual HWND getWin32() = 0;
	virtual std::vector<const char*> GetRequired() = 0;
};