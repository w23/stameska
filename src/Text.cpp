#error Text support still broken

#ifdef GEN_TEXT
#ifdef _WIN32
#define DEFAULT_FONT L"Tahoma"

struct Label {
	const char *uniform;
	const char *ansi;
	const wchar_t *unicode;
	const wchar_t *font;
	int size;
	int x, y, w, h;
};

//const wchar_t rojikoma[] = { 0x30ed, 0x30b8, 0x30b3, 0x30de, 0x0020, 0x0425, 0x0423, 0x0419, 0 };

Label labels[] = {
	{"lab_greets",
#include "greetings.h"
, NULL, L"Arial", 24, 0, 0, 0, 0},
};

static void initText(Texture &text) {
	const unsigned int TEXT_WIDTH = 256, TEXT_HEIGHT = 2048;
	const HDC text_dc = CreateCompatibleDC(NULL);
	BITMAPINFO bitmap_info = { 0 };
	bitmap_info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitmap_info.bmiHeader.biWidth = TEXT_WIDTH;
	bitmap_info.bmiHeader.biHeight = TEXT_HEIGHT;
	bitmap_info.bmiHeader.biBitCount = 32;
	bitmap_info.bmiHeader.biCompression = BI_RGB;
	bitmap_info.bmiHeader.biPlanes = 1;
	void *bitmap_ptr = NULL;
	const HBITMAP dib = CreateDIBSection(text_dc, &bitmap_info, DIB_RGB_COLORS, &bitmap_ptr, NULL, 0);
	const HGDIOBJ obj = SelectObject(text_dc, dib);
	RECT rect = { 0, 0, TEXT_WIDTH, TEXT_HEIGHT };
	SetTextColor(text_dc, RGB(255, 255, 255));
	SetBkMode(text_dc, TRANSPARENT);

	int size = 0;
	HFONT font = 0;
	for (int i = 0; i < COUNTOF(labels); ++i) {
		Label &l = labels[i];

		font = CreateFont(l.size, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*CLEARTYPE_QUALITY*/ NONANTIALIASED_QUALITY, 0, l.font);
		SelectObject(text_dc, font);

		// needs rect; keeps newlines
		if (l.ansi) {
			DrawTextA(text_dc, l.ansi, -1, &rect, DT_CALCRECT);
			DrawTextA(text_dc, l.ansi, -1, &rect, 0);
			//DrawTextA(text_dc, l.ansi, -1, &rect, DT_CENTER | DT_VCENTER | DT_CALCRECT);
			//DrawTextA(text_dc, l.ansi, -1, &rect, DT_CENTER | DT_VCENTER);
			// DT_SINGLELINE | DT_NOCLIP);
		} else {
			DrawTextW(text_dc, l.unicode, -1, &rect, DT_CALCRECT);
			DrawTextW(text_dc, l.unicode, -1, &rect, 0);
		}

		l.x = rect.left;
		l.y = rect.top; l.w = rect.right - rect.left;
		l.h = rect.bottom - rect.top;

#ifdef TOOL
		MSG("%s %d %d %d %d", l.uniform, l.x, l.y, l.w, l.h);
#endif

		rect.left = 0;
		rect.right = TEXT_WIDTH;
		rect.top = rect.bottom;
		rect.bottom = TEXT_HEIGHT;

		/*
		// ignores newlines
		TextOutW(text_dc, 0, 400, ro, 8);
		TextOutA(text_dc, 0, 600, shader_program, strlen(shader_program));
		*/
		DeleteObject(font);
	}

	text.upload(TEXT_WIDTH, TEXT_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, bitmap_ptr);

	DeleteObject(dib);
	DeleteDC(text_dc);
}
#endif // WIN32
#endif // GEN_TEXT
