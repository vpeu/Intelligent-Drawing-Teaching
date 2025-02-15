#include "IdtFreezeFrame.h"

int FreezeRecall;

void FreezeFrameWindow()
{
	while (!already) this_thread::sleep_for(chrono::milliseconds(50));

	if (displays_number != 1) return;
	while (magnificationWindowReady != -1) this_thread::sleep_for(chrono::milliseconds(50));

	thread_status[L"FreezeFrameWindow"] = true;

	DisableResizing(freeze_window, true);//禁止窗口拉伸
	SetWindowLong(freeze_window, GWL_STYLE, GetWindowLong(freeze_window, GWL_STYLE) & ~WS_CAPTION);//隐藏标题栏
	SetWindowPos(freeze_window, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_DRAWFRAME);
	SetWindowLong(freeze_window, GWL_EXSTYLE, WS_EX_TOOLWINDOW);//隐藏任务栏

	IMAGE freeze_background(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
	SetImageColor(freeze_background, RGBA(0, 0, 0, 0), true);

	// 设置BLENDFUNCTION结构体
	BLENDFUNCTION blend;
	blend.BlendOp = AC_SRC_OVER;
	blend.BlendFlags = 0;
	blend.SourceConstantAlpha = 255; // 设置透明度，0为全透明，255为不透明
	blend.AlphaFormat = AC_SRC_ALPHA; // 使用源图像的alpha通道
	HDC hdcScreen = GetDC(NULL);
	// 调用UpdateLayeredWindow函数更新窗口
	POINT ptSrc = { 0,0 };
	SIZE sizeWnd = { freeze_background.getwidth(),freeze_background.getheight() };
	POINT ptDst = { 0,0 }; // 设置窗口位置
	UPDATELAYEREDWINDOWINFO ulwi = { 0 };
	ulwi.cbSize = sizeof(ulwi);
	ulwi.hdcDst = hdcScreen;
	ulwi.pptDst = &ptDst;
	ulwi.psize = &sizeWnd;
	ulwi.pptSrc = &ptSrc;
	ulwi.crKey = RGB(255, 255, 255);
	ulwi.pblend = &blend;
	ulwi.dwFlags = ULW_ALPHA;

	do
	{
		Sleep(10);
		::SetWindowLong(freeze_window, GWL_EXSTYLE, ::GetWindowLong(freeze_window, GWL_EXSTYLE) | WS_EX_LAYERED);
	} while (!(::GetWindowLong(freeze_window, GWL_EXSTYLE) & WS_EX_LAYERED));
	do
	{
		Sleep(10);
		::SetWindowLong(freeze_window, GWL_EXSTYLE, ::GetWindowLong(freeze_window, GWL_EXSTYLE) | WS_EX_NOACTIVATE);
	} while (!(::GetWindowLong(freeze_window, GWL_EXSTYLE) & WS_EX_NOACTIVATE));

	ulwi.hdcSrc = GetImageHDC(&freeze_background);
	UpdateLayeredWindowIndirect(freeze_window, &ulwi);
	ShowWindow(freeze_window, SW_SHOW);

	FreezeFrame.update = true;
	int wait = 0;
	bool show_freeze_window = false;

	RECT fwords_rect;
	while (!off_signal)
	{
		Sleep(20);

		if (FreezeFrame.mode == 1)
		{
			IMAGE MagnificationTmp;
			if (!show_freeze_window)
			{
				RequestUpdateMagWindow = true;
				while (RequestUpdateMagWindow) Sleep(100);

				std::shared_lock<std::shared_mutex> lock1(MagnificationBackgroundSm);
				MagnificationTmp = MagnificationBackground;
				lock1.unlock();

				SetImageColor(freeze_background, RGBA(0, 0, 0, 0), true);
				hiex::TransparentImage(&freeze_background, 0, 0, &MagnificationTmp);

				ulwi.hdcSrc = GetImageHDC(&freeze_background);
				UpdateLayeredWindowIndirect(freeze_window, &ulwi);
				show_freeze_window = true;
			}

			if (SeewoCamera) wait = 480;
			while (!off_signal)
			{
				if (FreezeFrame.mode != 1 || ppt_show != NULL) break;

				if (FreezeFrame.update)
				{
					std::shared_lock<std::shared_mutex> lock1(MagnificationBackgroundSm);
					MagnificationTmp = MagnificationBackground;
					lock1.unlock();

					SetImageColor(freeze_background, RGBA(0, 0, 0, 0), true);
					hiex::TransparentImage(&freeze_background, 0, 0, &MagnificationTmp);
					ulwi.hdcSrc = GetImageHDC(&freeze_background);
					UpdateLayeredWindowIndirect(freeze_window, &ulwi);

					FreezeFrame.update = false;
				}
				else if (wait > 0 && SeewoCamera)
				{
					hiex::TransparentImage(&freeze_background, 0, 0, &MagnificationTmp);

					hiex::EasyX_Gdiplus_FillRoundRect((float)GetSystemMetrics(SM_CXSCREEN) / 2 - 160, (float)GetSystemMetrics(SM_CYSCREEN) - 200, 320, 50, 20, 20, RGBA(255, 255, 225, min(255, wait)), RGBA(0, 0, 0, min(150, wait)), 2, true, SmoothingModeHighQuality, &freeze_background);

					Graphics graphics(GetImageHDC(&freeze_background));
					Gdiplus::Font gp_font(&HarmonyOS_fontFamily, 24, FontStyleRegular, UnitPixel);
					SolidBrush WordBrush(hiex::ConvertToGdiplusColor(RGBA(255, 255, 255, min(255, wait)), true));
					graphics.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
					{
						dwords_rect.left = GetSystemMetrics(SM_CXSCREEN) / 2 - 160;
						dwords_rect.top = GetSystemMetrics(SM_CYSCREEN) - 200;
						dwords_rect.right = GetSystemMetrics(SM_CXSCREEN) / 2 + 160;
						dwords_rect.bottom = GetSystemMetrics(SM_CYSCREEN) - 200 + 52;
					}
					graphics.DrawString(L"智绘教已自动开启 画面定格", -1, &gp_font, hiex::RECTToRectF(dwords_rect), &stringFormat, &WordBrush);

					ulwi.hdcSrc = GetImageHDC(&freeze_background);
					UpdateLayeredWindowIndirect(freeze_window, &ulwi);

					wait -= 8;

					if (wait == 0)
					{
						hiex::TransparentImage(&freeze_background, 0, 0, &MagnificationTmp);

						ulwi.hdcSrc = GetImageHDC(&freeze_background);
						UpdateLayeredWindowIndirect(freeze_window, &ulwi);
					}
				}
				else if (FreezeRecall > 0)
				{
					hiex::TransparentImage(&freeze_background, 0, 0, &MagnificationTmp);
					hiex::EasyX_Gdiplus_FillRoundRect((float)GetSystemMetrics(SM_CXSCREEN) / 2 - 160, (float)GetSystemMetrics(SM_CYSCREEN) - 200, 320, 50, 20, 20, RGBA(255, 255, 225, min(255, FreezeRecall)), RGBA(0, 0, 0, min(150, FreezeRecall)), 2, true, SmoothingModeHighQuality, &freeze_background);

					wchar_t buffer[100];
					swprintf_s(buffer, L"超级恢复 %02d月%02d日 %02d:%02d:%02d", RecallImageTm.tm_mon + 1, RecallImageTm.tm_mday, RecallImageTm.tm_hour, RecallImageTm.tm_min, RecallImageTm.tm_sec);

					Graphics graphics(GetImageHDC(&freeze_background));
					Gdiplus::Font gp_font(&HarmonyOS_fontFamily, 22, FontStyleRegular, UnitPixel);
					SolidBrush WordBrush(hiex::ConvertToGdiplusColor(RGBA(255, 255, 255, min(255, FreezeRecall)), true));
					graphics.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
					{
						fwords_rect.left = GetSystemMetrics(SM_CXSCREEN) / 2 - 160;
						fwords_rect.top = GetSystemMetrics(SM_CYSCREEN) - 200;
						fwords_rect.right = GetSystemMetrics(SM_CXSCREEN) / 2 + 160;
						fwords_rect.bottom = GetSystemMetrics(SM_CYSCREEN) - 200 + 52;
					}
					graphics.DrawString(buffer, -1, &gp_font, hiex::RECTToRectF(fwords_rect), &stringFormat, &WordBrush);

					ulwi.hdcSrc = GetImageHDC(&freeze_background);
					UpdateLayeredWindowIndirect(freeze_window, &ulwi);

					FreezeRecall -= 10;

					if (FreezeRecall <= 0)
					{
						hiex::TransparentImage(&freeze_background, 0, 0, &MagnificationTmp);

						ulwi.hdcSrc = GetImageHDC(&freeze_background);
						UpdateLayeredWindowIndirect(freeze_window, &ulwi);

						if (FreezeRecall <= 0) FreezeRecall = 0;
						break;
					}
				}

				Sleep(20);
			}

			if (ppt_show != NULL) FreezeFrame.mode = 0;
			FreezeFrame.update = true;
		}
		else if (show_freeze_window)
		{
			SetImageColor(freeze_background, RGBA(0, 0, 0, 0), true);
			ulwi.hdcSrc = GetImageHDC(&freeze_background);
			UpdateLayeredWindowIndirect(freeze_window, &ulwi);
			show_freeze_window = false;
		}

		if (FreezeFrame.mode != 1 && FreezePPT)
		{
			if (!show_freeze_window)
			{
				SetImageColor(freeze_background, RGBA(0, 0, 0, 0), true);

				ulwi.hdcSrc = GetImageHDC(&freeze_background);
				UpdateLayeredWindowIndirect(freeze_window, &ulwi);
				show_freeze_window = true;
			}

			clock_t tRecord = clock();
			int for_i = -10;
			for (for_i = -10; for_i <= 60 && FreezePPT && !off_signal; for_i++)
			{
				SetImageColor(freeze_background, RGBA(0, 0, 0, 140), true);
				hiex::TransparentImage(&freeze_background, GetSystemMetrics(SM_CXSCREEN) / 2 - 500, GetSystemMetrics(SM_CYSCREEN) / 2 - 163, &SettingSign[3]);

				hiex::EasyX_Gdiplus_SolidRoundRect((float)GetSystemMetrics(SM_CXSCREEN) / 2 - 300, (float)GetSystemMetrics(SM_CYSCREEN) / 2 + 200, 600, 10, 10, 10, RGBA(255, 255, 255, 100), true, SmoothingModeHighQuality, &freeze_background);
				hiex::EasyX_Gdiplus_SolidRoundRect((float)GetSystemMetrics(SM_CXSCREEN) / 2 - 300, (float)GetSystemMetrics(SM_CYSCREEN) / 2 + 200, (float)max(0, min(50, for_i)) * 12, 10, 10, 10, RGBA(255, 255, 255, 255), false, SmoothingModeHighQuality, &freeze_background);

				{
					Graphics graphics(GetImageHDC(&freeze_background));
					Gdiplus::Font gp_font(&HarmonyOS_fontFamily, 24, FontStyleRegular, UnitPixel);
					SolidBrush WordBrush(hiex::ConvertToGdiplusColor(RGBA(255, 255, 255, 255), false));
					graphics.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
					{
						dwords_rect.left = GetSystemMetrics(SM_CXSCREEN) / 2 - 500;
						dwords_rect.top = GetSystemMetrics(SM_CYSCREEN) / 2 + 250;
						dwords_rect.right = GetSystemMetrics(SM_CXSCREEN) / 2 + 500;
						dwords_rect.bottom = GetSystemMetrics(SM_CYSCREEN) / 2 + 300;
					}
					graphics.DrawString(L"Tips：无需处于选择模式，点击下方按钮即可翻页", -1, &gp_font, hiex::RECTToRectF(dwords_rect), &stringFormat, &WordBrush);
				}

				ulwi.hdcSrc = GetImageHDC(&freeze_background);
				UpdateLayeredWindowIndirect(freeze_window, &ulwi);

				if (tRecord)
				{
					int delay = 1000 / 25 - (clock() - tRecord);
					if (delay > 0) std::this_thread::sleep_for(std::chrono::milliseconds(delay));
				}
				tRecord = clock();
			}

			if (for_i > 60) ppt_title_recond[ppt_title] = true;
			FreezePPT = false;
		}
		if (FreezeFrame.mode != 1 && FreezeRecall)
		{
			while (!off_signal)
			{
				SetImageColor(freeze_background, RGBA(0, 0, 0, 0), true);

				hiex::EasyX_Gdiplus_FillRoundRect((float)GetSystemMetrics(SM_CXSCREEN) / 2 - 160, (float)GetSystemMetrics(SM_CYSCREEN) - 200, 320, 50, 20, 20, RGBA(255, 255, 225, min(255, FreezeRecall)), RGBA(0, 0, 0, min(150, FreezeRecall)), 2, true, SmoothingModeHighQuality, &freeze_background);

				wchar_t buffer[100];
				swprintf_s(buffer, L"超级恢复 %02d月%02d日 %02d:%02d:%02d", RecallImageTm.tm_mon + 1, RecallImageTm.tm_mday, RecallImageTm.tm_hour, RecallImageTm.tm_min, RecallImageTm.tm_sec);

				Graphics graphics(GetImageHDC(&freeze_background));
				Gdiplus::Font gp_font(&HarmonyOS_fontFamily, 22, FontStyleRegular, UnitPixel);
				SolidBrush WordBrush(hiex::ConvertToGdiplusColor(RGBA(255, 255, 255, min(255, FreezeRecall)), true));
				graphics.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
				{
					fwords_rect.left = GetSystemMetrics(SM_CXSCREEN) / 2 - 160;
					fwords_rect.top = GetSystemMetrics(SM_CYSCREEN) - 200;
					fwords_rect.right = GetSystemMetrics(SM_CXSCREEN) / 2 + 160;
					fwords_rect.bottom = GetSystemMetrics(SM_CYSCREEN) - 200 + 52;
				}
				graphics.DrawString(buffer, -1, &gp_font, hiex::RECTToRectF(fwords_rect), &stringFormat, &WordBrush);

				ulwi.hdcSrc = GetImageHDC(&freeze_background);
				UpdateLayeredWindowIndirect(freeze_window, &ulwi);

				FreezeRecall -= 10;
				Sleep(20);

				if (FreezeRecall <= 0)
				{
					SetImageColor(freeze_background, RGBA(0, 0, 0, 0), true);

					ulwi.hdcSrc = GetImageHDC(&freeze_background);
					UpdateLayeredWindowIndirect(freeze_window, &ulwi);

					if (FreezeRecall <= 0) FreezeRecall = 0;
					break;
				}
			}
		}
	}
	thread_status[L"FreezeFrameWindow"] = false;
}