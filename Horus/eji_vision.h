#pragma once

#include <opencv.hpp>
#include <iostream>
#include <Windows.h>

class eji_vision
{
public:
	/// <summary>
	/// HWND转Mat
	/// </summary>
	/// <param name="hwnd"></param>
	/// <returns></returns>
	static cv::Mat HWND2Mat(HWND hwnd)
	{
		HDC hdc_window, hdc_window_compatible;
		HBITMAP hb_window;
		cv::Mat im_src;
		BITMAPINFOHEADER bmp_info;

		hdc_window = GetDC(hwnd);								// 获取窗口句柄的设备上下文
		hdc_window_compatible = CreateCompatibleDC(hdc_window);		// 获取设备上下文的内存句柄
		SetStretchBltMode(hdc_window_compatible, COLORONCOLOR);		// 设置位图的拉伸模式，以便当图像调整为较小尺寸时，消除的像素在不保留信息的情况下被删除

		// 窗口的尺寸
		RECT window_size;
		GetClientRect(hwnd, &window_size);

		auto width = window_size.right;
		auto height = window_size.bottom;

		// 创建彩色矩阵
		im_src.create(height, width, CV_8UC4);

		// 创建位图
		hb_window = CreateCompatibleBitmap(hdc_window, width, height);
		bmp_info.biSize = sizeof(BITMAPINFOHEADER);
		bmp_info.biWidth = width;
		bmp_info.biHeight = -height;  // 这是使它上下颠倒的线
		bmp_info.biPlanes = 1;
		bmp_info.biBitCount = 32;
		bmp_info.biCompression = BI_RGB;
		bmp_info.biSizeImage = 0;
		bmp_info.biXPelsPerMeter = 0;
		bmp_info.biYPelsPerMeter = 0;
		bmp_info.biClrUsed = 0;
		bmp_info.biClrImportant = 0;

		// 使用先前创建的设备上下文用于位图
		SelectObject(hdc_window_compatible, hb_window);
		// 从窗口设备上下文复制到位图设备上下文
		StretchBlt(hdc_window_compatible, 0, 0, width, height, hdc_window, 0, 0, width, height, SRCCOPY);
		GetDIBits(hdc_window_compatible, hb_window, 0, height, im_src.data, (BITMAPINFO*)&bmp_info, DIB_RGB_COLORS);

		// 释放资源
		DeleteObject(hb_window);
		DeleteDC(hdc_window_compatible);
		ReleaseDC(hwnd, hdc_window);

		return im_src;
	}

	/// <summary>
	/// 返回模板匹配结果
	/// </summary>
	/// <param name="im_src"></param>
	/// <param name="template_path"></param>
	/// <returns></returns>
	static cv::Point MatchTemplate(cv::Mat im_src, cv::Mat im_template)
	{
		cv::Mat im_gray;
		cv::Mat im_thresh;
		cv::cvtColor(im_src, im_gray, cv::COLOR_BGRA2GRAY);
		// 进行二值化处理
		cv::threshold(im_gray, im_thresh, 200, 255, cv::THRESH_BINARY);

		// 进行模板匹配，定位角色状态栏
		cv::Mat im_match_result;
		cv::matchTemplate(im_thresh, im_template, im_match_result, cv::TM_CCORR_NORMED);
		// 归一化结果
		cv::normalize(im_match_result, im_match_result, 0, 1, cv::NORM_MINMAX);
		// 定位最佳匹配位置
		double min_value, max_value;
		cv::Point min_loc, max_loc;
		cv::Point match_loc;
		cv::minMaxLoc(im_match_result, &min_value, &max_value, &min_loc, &max_loc, cv::Mat());
		// 对于方法SQDIFF和SQDIFF_NORMED两种方法来讲，越小的值就有着更高的匹配结果，而其余的方法则是数值越大匹配效果越好
		match_loc = max_loc;

		return match_loc;
	}
};
